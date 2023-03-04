/*
    Diablo II Character Editor
    Copyright (C) 2021-2023 Walter Couto

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------

#include "pch.h"
#include "ItemHelpers.h"
#include "DefaultTxtReader.h"
#include <rapidcsv/rapidcsv.h>
#include <json/json.h>
#include <sstream>
#include <iterator>
#include <regex>
#include <map>
#include <memory>
#include <variant>
#include <utf8/utf8.h>
#include <StormLib.h>

//---------------------------------------------------------------------------
namespace d2ce
{
#define MAX_VALUE_NAME 4096
    std::filesystem::path GetD2RModsPath()
    {
        // Search the Classic register position for the path to use
        HKEY regKey = 0;
        if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Diablo II Resurrected", 0, KEY_QUERY_VALUE, &regKey) != ERROR_SUCCESS)
        {
            return std::filesystem::path();
        }

        std::vector<wchar_t> valueName(MAX_VALUE_NAME + 1, 0);
        DWORD valueNameLength = MAX_VALUE_NAME;
        DWORD valueType = 0;
        if ((RegQueryValueEx(regKey, L"InstallLocation", 0, &valueType, (LPBYTE)&valueName[0], &valueNameLength) == ERROR_SUCCESS) && (valueType == REG_SZ))
        {
            std::filesystem::path path(&valueName[0]);
            path /= "mods";
            return path;
        }

        return std::filesystem::path();
    }

    namespace JsonReaderHelper
    {
        const std::string& GetItemGems();
        const std::string& GetItemModifiers();
        const std::string& GetItemNameAffixes();
        const std::string& GetItemNames();
        const std::string& GetItemRunes();
        const std::string& GetLevels();
        const std::string& GetMercenaries();
        const std::string& GetMonsters();
        const std::string& GetNPCs();
        const std::string& GetQuests();
        const std::string& GetSkills();
        const std::string& GetUI();
    }

    namespace TxtReaderHelper
    {
        const std::string& GetArmor();
        const std::string& GetBelts();
        const std::string& GetBodyLocs();
        const std::string& GetCharStats();
        const std::string& GetExperience();
        const std::string& GetGems();
        const std::string& GetHireling();
        const std::string& GetItemsCode();
        const std::string& GetItemStatCost();
        const std::string& GetItemTypes();
        const std::string& GetMagicPrefix();
        const std::string& GetMagicSuffix();
        const std::string& GetMisc();
        const std::string& GetPlayerClass();
        const std::string& GetProperties();
        const std::string& GetRarePrefix();
        const std::string& GetRareSuffix();
        const std::string& GetRunes();
        const std::string& GetSetItems();
        const std::string& GetSets();
        const std::string& GetSkillDesc();
        const std::string& GetSkills();
        const std::string& GetUniqueItems();
        const std::string& GetWeapons();
        const std::string& GetMonStats();
        const std::string& GetMonType();
        const std::string& GetSuperiorModsTxt();
        const std::string& GetCraftModsTxt();
    }

    class D2RModReaderHelper;
    class D2RFormatDescriptor
    {
    public:
        enum class CommandType
        {
            s8,
            u8,
            s16,
            u16,
            s32,
            u32,
            chararray,
            ignore,
        };

        struct D2FormatItem
        {
            CommandType outType = CommandType::ignore;
            size_t cmdSize = 0;
            bool blankIfZero = false;
            std::function<std::string(std::int32_t, const D2RModReaderHelper&, size_t)>* pNumericFunc = nullptr;
            std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>* pUnsignedFunc = nullptr;
            std::function<std::string(std::string, const D2RModReaderHelper&, size_t)>* pStringFunc = nullptr;
        };

        struct D2RExcelFormat
        {
            std::string header;
            size_t idx = 0;
            bool blankIfZero = false;
            bool fake = false;
            std::function<std::string(std::int32_t, const D2RModReaderHelper&, size_t)>* pNumericFunc = nullptr;
            std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>* pUnsignedFunc = nullptr;
            std::function<std::string(std::string, const D2RModReaderHelper&, size_t)>* pStringFunc = nullptr;
            std::function<std::string(size_t)>* pRowIDFunc = nullptr;
        };

        D2RFormatDescriptor(const char* format, const std::vector<D2RExcelFormat>& excelFormat, size_t versionTestCol = MAXSIZE_T) : m_excelFormat(excelFormat), m_versionTestCol(versionTestCol)
        {
            if (format != nullptr)
            {
                std::string val(format);
                ParseFormat(val);
            }
        }

        D2RFormatDescriptor(const std::string& format, const std::vector<D2RExcelFormat>& excelFormat, size_t versionTestCol = MAXSIZE_T) : m_excelFormat(excelFormat), m_versionTestCol(versionTestCol)
        {
            ParseFormat(format);
        }

        const D2RFormatDescriptor::D2FormatItem* getNextFormatItem() const
        {
            if (m_iter == m_format.cend())
            {
                reset();
                static D2FormatItem dummy;
                return &dummy;
            }

            auto* formatItem = &(*m_iter);
            ++m_iter;
            return formatItem;
        }

        size_t getLineByteSize() const
        {
            return m_lineByteSize;
        }

        size_t size() const
        {
            return m_format.size();
        }

        size_t numColumns() const
        {
            return m_indices.size();
        }

        D2FormatItem& columnFormat(size_t idx)
        {
            return m_format.at(m_indices.at(idx));
        }

        void reset() const
        {
            if (!m_format.empty())
            {
                m_iter = m_format.cbegin();
            }
        }

        size_t numExcelColumns() const
        {
            return m_excelFormat.size();
        }

        const std::vector<D2RExcelFormat>& getExcelFormat() const
        {
            return m_excelFormat;
        }

        size_t getVersionTestIdx() const
        {
            return m_versionTestCol;
        }

    private:
        void ParseFormat(const std::string& format)
        {
            m_lineByteSize = 0;
            D2FormatItem curItem;
            auto iter = format.begin();
            auto iter_end = format.end();
            while (iter != iter_end)
            {
                curItem.outType = CommandType::ignore;
                curItem.cmdSize = 0;
                curItem.blankIfZero = false;
                switch (*iter)
                {
                case 'c':
                    curItem.outType = CommandType::s8;
                    curItem.cmdSize = sizeof(std::int8_t);
                    break;

                case 'b':
                    curItem.outType = CommandType::u8;
                    curItem.cmdSize = sizeof(std::uint8_t);
                    break;

                case 's':
                    curItem.outType = CommandType::s16;
                    curItem.cmdSize = sizeof(std::int16_t);
                    break;

                case 'w':
                    curItem.outType = CommandType::u16;
                    curItem.cmdSize = sizeof(std::uint16_t);
                    break;

                case 'i':
                    curItem.outType = CommandType::s32;
                    curItem.cmdSize = sizeof(std::int32_t);
                    break;

                case 'u':
                    curItem.outType = CommandType::u32;
                    curItem.cmdSize = sizeof(std::uint32_t);
                    break;

                case 'a':
                    curItem.outType = CommandType::chararray;
                    curItem.cmdSize = getArgSize(iter, iter_end);
                    break;

                case 'x':
                    curItem.outType = CommandType::ignore;
                    curItem.cmdSize = getArgSize(iter, iter_end);
                    break;
                }

                m_lineByteSize += curItem.cmdSize;
                if (curItem.outType != CommandType::ignore)
                {
                    m_indices.push_back(m_format.size());
                }

                m_format.push_back(curItem);
                if (iter != iter_end)
                {
                    ++iter;
                }
            }

            if (!m_format.empty())
            {
                m_iter = m_format.cbegin();
            }

            // Sync options from excel format with bin format
            for (auto& column : m_excelFormat)
            {
                if (column.fake || column.pRowIDFunc != nullptr)
                {
                    continue;
                }

                if (column.idx < numColumns())
                {
                    auto& item = columnFormat(column.idx);
                    item.blankIfZero = column.blankIfZero;
                    item.pNumericFunc = column.pNumericFunc;
                    item.pUnsignedFunc = column.pUnsignedFunc;
                    item.pStringFunc = column.pStringFunc;
                }
            }
        }

        size_t getArgSize(std::string::const_iterator& iter, std::string::const_iterator iter_end) const
        {
            if (iter == iter_end)
            {
                return 0;
            }

            std::string num;
            while ((iter + 1) != iter_end && isdigit(*(iter + 1)))
            {
                ++iter;
                num += *iter;
            }

            try
            {
                return size_t(std::stoul(num));
            }
            catch (...)
            {
                return 0;
            }
        }

    protected:
        mutable std::vector<D2FormatItem> m_format;
        mutable std::vector<D2RExcelFormat> m_excelFormat;
        size_t m_versionTestCol = MAXSIZE_T;

    private:
        mutable std::vector<D2FormatItem>::const_iterator m_iter = m_format.cend();
        size_t m_lineByteSize = 0;
        std::vector<size_t> m_indices;
    };

    class D2RBinReader
    {
    public:
        static std::string readArmorBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readBeltsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readBodyLocsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readCharStatsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readExperienceBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readGemsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readHirelingBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readItemsCodeBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readItemStatCostBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readItemTypesBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readMagicAffixBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readMiscBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readMonStatsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readMonTypeBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readPlayerClassBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readPropertiesBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readQualityItemsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readRareAffixBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readRunesBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readSetItemsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readSetsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readSkillDescBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readSkillsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readUniqueItemsBin(const std::string& bin, const D2RModReaderHelper& parent);
        static std::string readWeaponsBin(const std::string& bin, const D2RModReaderHelper& parent);

    private:
        static bool initializeItemTypesBin(const std::string& bin, const D2RModReaderHelper& parent);
        static bool initializeSkillsBin(const std::string& bin, const D2RModReaderHelper& parent);

        static std::string processExpressionStack(std::stack<std::variant<std::uint8_t, std::uint16_t, std::uint32_t, std::string>>& stack)
        {
            auto value = stack.top();
            stack.pop();
            if (std::uint8_t const* pval = std::get_if<std::uint8_t>(&value))
            {
                return std::to_string(std::int8_t(*pval));
            }

            if (std::uint16_t const* pval = std::get_if<std::uint16_t>(&value))
            {
                return std::to_string(std::int16_t(*pval));
            }

            if (std::uint32_t const* pval = std::get_if<std::uint32_t>(&value))
            {
                return std::to_string(std::int32_t(*pval));
            }

            if (std::string const* pval = std::get_if<std::string>(&value))
            {
                return *pval;
            }

            return std::string();
        }

        static bool isExpansionVersion(const std::string& strVal, size_t /*lineNum*/)
        {
            if (strVal.empty())
            {
                return false;

            }

            try
            {
                if (std::stoul(strVal) >= 100)
                {
                    return true;
                }
            }
            catch (...)
            {
            }

            return false;
        }

        static bool isExpansionGem(const std::string& strVal, size_t /*lineNum*/)
        {
            if (strVal.empty())
            {
                return false;

            }

            return true;
        }

        static bool isExpansionPlayerClass(const std::string& strVal, size_t /*lineNum*/)
        {
            if (strVal.empty())
            {
                return false;
            }

            if (strVal == "Druid" || strVal == "Assassin")
            {
                return true;
            }

            return false;
        }

        static bool isExpansionPropertyRow(const std::string& /*strVal*/, size_t lineNum)
        {
            return lineNum >= 121 ? true : false;
        }

        static std::string processBin(const char* data, size_t& length, const D2RFormatDescriptor& desc, const D2RModReaderHelper& parent, std::function<bool(const std::string&, size_t)>* pIsExpansion = nullptr)
        {
            if (data == nullptr || length < 4)
            {
                return std::string();
            }

            std::uint32_t numLines = 0;
            size_t dataSize = sizeof(std::uint32_t);
            memcpy(&numLines, data, dataSize);
            data += dataSize;
            length -= dataSize;
            if (numLines == 0 || (numLines * desc.getLineByteSize() > length))
            {
                // size mismatch
                return std::string();
            }

            // output header
            std::stringstream ss;
            bool bFirst = true;
            const auto& format = desc.getExcelFormat();
            for (const auto& column : format)
            {
                if (!bFirst)
                {
                    ss << "\t";
                }
                ss << column.header;
                bFirst = false;
            }
            ss << "\n";

            std::stringstream ssExpansion;
            size_t verstionTestIdx = desc.getVersionTestIdx();
            bool bIsExpansion = false;
            bool bHasExpansionRows = false;
            std::vector<std::string> output;
            std::string strVal;
            size_t lineNum = 0;
            while (readLine(data, length, desc, output, parent, lineNum))
            {
                bIsExpansion = false;
                if (pIsExpansion != nullptr && verstionTestIdx != MAXSIZE_T)
                {
                    strVal.clear();
                    if (verstionTestIdx != MAXSIZE_T)
                    {
                        strVal = output.at(verstionTestIdx);
                    }
                    bIsExpansion = (*pIsExpansion)(strVal, lineNum);
                    if (bIsExpansion)
                    {
                        bHasExpansionRows = true;
                    }
                }

                auto& ssRef = bIsExpansion ? ssExpansion : ss;

                bFirst = true;
                for (const auto& column : format)
                {
                    strVal.clear();
                    if (!bFirst)
                    {
                        ssRef << "\t";
                    }

                    if (column.fake || column.pRowIDFunc != nullptr)
                    {
                        if (column.pRowIDFunc != nullptr)
                        {
                            strVal = (*column.pRowIDFunc)(lineNum);
                        }
                        else if (column.pStringFunc != nullptr)
                        {
                            strVal = (*column.pStringFunc)(output.at(column.idx), parent, lineNum);
                        }
                        else if (column.pUnsignedFunc != nullptr)
                        {
                            try
                            {
                                strVal = (*column.pUnsignedFunc)(std::uint32_t(std::stoul(output.at(column.idx))), parent, lineNum);
                            }
                            catch (...)
                            {
                                strVal = output.at(column.idx);
                            }
                        }
                        else if (column.pNumericFunc != nullptr)
                        {
                            try
                            {
                                strVal = (*column.pNumericFunc)(std::stoi(output.at(column.idx)), parent, lineNum);
                            }
                            catch (...)
                            {
                                strVal = output.at(column.idx);
                            }
                        }
                        else
                        {
                            strVal = output.at(column.idx);
                        }
                    }
                    else
                    {
                        strVal = output.at(column.idx);
                    }

                    // trim trailing spaces
                    strVal.erase(std::find_if(strVal.rbegin(), strVal.rend(), [](int ch) { return !isspace(ch); }).base(), strVal.end());
                    if (!strVal.empty())
                    {
                        ssRef << strVal;
                    }
                    bFirst = false;
                }
                ssRef << "\n";
                ++lineNum;
            }

            if (bHasExpansionRows)
            {
                ss << "Expansion";
                auto numCols = desc.numColumns();
                if (numCols > 1)
                {
                    ss << std::string(numCols - 1, '\t');
                }
                ss << "\n";
                ss << ssExpansion.str();
            }

            return ss.str();
        }

        static bool readLine(const char*& data, size_t& length, const D2RFormatDescriptor& desc, std::vector<std::string>& output, const D2RModReaderHelper& parent, size_t lineNum)
        {
            if (data == nullptr || length == 0)
            {
                return false;
            }

            desc.reset();
            output.clear();
            if (length < desc.getLineByteSize())
            {
                // to small
                return false;
            }

            std::string valStr;
            for (auto formatItem = desc.getNextFormatItem(); formatItem != nullptr && formatItem->cmdSize != 0 && length >= formatItem->cmdSize; formatItem = desc.getNextFormatItem())
            {
                valStr = processField(data, length, *formatItem, parent, lineNum);
                if (formatItem->outType != D2RFormatDescriptor::CommandType::ignore)
                {
                    output.push_back(valStr);
                }
            }

            return true;
        }

        static std::string processUnsignedType(const char*& data, size_t& length, const D2RFormatDescriptor::D2FormatItem& formatItem, const D2RModReaderHelper& parent, size_t lineNum)
        {
            std::uint16_t valu16 = 0;
            std::uint32_t valu32 = 0;
            switch (formatItem.outType)
            {
            case D2RFormatDescriptor::CommandType::u8:
                valu32 = std::uint32_t(std::uint8_t(*data));
                break;

            case D2RFormatDescriptor::CommandType::u16:
                memcpy(&valu16, data, formatItem.cmdSize);
                valu32 = valu16;
                break;

            case D2RFormatDescriptor::CommandType::u32:
                memcpy(&valu32, data, formatItem.cmdSize);
                break;
            }

            data += formatItem.cmdSize;
            length -= formatItem.cmdSize;
            if (formatItem.blankIfZero && valu32 == 0)
            {
                return std::string();
            }
            else if (formatItem.pUnsignedFunc != nullptr)
            {
                return (*formatItem.pUnsignedFunc)(valu32, parent, lineNum);
            }
            else if (formatItem.pNumericFunc != nullptr)
            {
                return (*formatItem.pNumericFunc)(std::int32_t(valu32), parent, lineNum);
            }
            else
            {
                return std::to_string(valu32);
            }
        }

        static std::string processNumericType(const char*& data, size_t& length, const D2RFormatDescriptor::D2FormatItem& formatItem, const D2RModReaderHelper& parent, size_t lineNum)
        {
            std::int16_t val16 = 0;
            std::int32_t val32 = 0;
            switch (formatItem.outType)
            {
            case D2RFormatDescriptor::CommandType::s8:
                val32 = std::int16_t(*data);
                break;

            case D2RFormatDescriptor::CommandType::s16:
                memcpy(&val16, data, formatItem.cmdSize);
                val32 = val16;
                break;

            case D2RFormatDescriptor::CommandType::s32:
                memcpy(&val32, data, formatItem.cmdSize);
                break;
            }

            data += formatItem.cmdSize;
            length -= formatItem.cmdSize;
            if (formatItem.blankIfZero && val32 == 0)
            {
                return std::string();
            }
            else if (formatItem.pNumericFunc != nullptr)
            {
                return (*formatItem.pNumericFunc)(val32, parent, lineNum);
            }
            else if (formatItem.pUnsignedFunc != nullptr)
            {
                return (*formatItem.pUnsignedFunc)(std::uint32_t(val32), parent, lineNum);
            }
            else
            {
                return std::to_string(val32);
            }
        }

        static std::string processStringType(const char*& data, size_t& length, const D2RFormatDescriptor::D2FormatItem& formatItem, const D2RModReaderHelper& parent, size_t lineNum)
        {
            size_t strLen = 0;
            switch (formatItem.outType)
            {
            case D2RFormatDescriptor::CommandType::chararray:
                strLen = strlen(data);
                break;
            }

            std::string valStr;
            if (strLen != 0)
            {
                if (strLen > formatItem.cmdSize)
                {
                    valStr = std::string(data, formatItem.cmdSize);
                }
                else
                {
                    valStr = data;
                }

                if (formatItem.pStringFunc != nullptr)
                {
                    return (*formatItem.pStringFunc)(valStr, parent, lineNum);
                }
            }

            data += formatItem.cmdSize;
            length -= formatItem.cmdSize;
            return valStr;
        }

        static std::string processField(const char*& data, size_t& length, const D2RFormatDescriptor::D2FormatItem& formatItem, const D2RModReaderHelper& parent, size_t lineNum)
        {
            switch (formatItem.outType)
            {
            case D2RFormatDescriptor::CommandType::s8:
            case D2RFormatDescriptor::CommandType::s16:
            case D2RFormatDescriptor::CommandType::s32:
                return processNumericType(data, length, formatItem, parent, lineNum);

            case D2RFormatDescriptor::CommandType::u8:
            case D2RFormatDescriptor::CommandType::u16:
            case D2RFormatDescriptor::CommandType::u32:
                return processUnsignedType(data, length, formatItem, parent, lineNum);

            case D2RFormatDescriptor::CommandType::chararray:
                return processStringType(data, length, formatItem, parent, lineNum);

            case D2RFormatDescriptor::CommandType::ignore:
            default:
                data += formatItem.cmdSize;
                length -= formatItem.cmdSize;
                return std::string();
            }
        }

        static std::string generateStatName(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processStatName(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processItemType(std::uint32_t type, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processItemCode(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string generatePropertyCode(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processPropertyEnabled(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processPropertyCode(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processBodyLocs(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processStringId(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processStringIdFiltered(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processCalcExpression(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processExperienceLevel(std::uint32_t type, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string generateSetIndex(std::uint32_t type, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processSetIndex(std::uint32_t type, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processSkillName(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string generateSkillDesc(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processSkillDesc(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processPlayerClassCode(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processHirelingType(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string generateMonType(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processMonType(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processMonStatsEnabledFlag(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processMonStatsKillableFlag(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string generateID(size_t lineNum);
        static std::string processColor(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum);
        
        static std::string processItemName(const std::string& code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processItemTypeName(const std::string& code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processBodyLocation(const std::string& code, const D2RModReaderHelper& parent, size_t lineNum);
        static std::string processRuneName(const std::string& code, const D2RModReaderHelper& parent, size_t lineNum);
    };

    class D2RModReaderHelper
    {
        friend class D2RBinReader;

    public:
        D2RModReaderHelper(const std::filesystem::path& path) : modPath(path)
        {
            LoadAll();
        }

        bool isValid() const
        {
            return !modName.empty();
        }

        std::string GetModName() const
        {
            return modName;
        }

        std::unique_ptr<ITxtDocument> GetStringTxt() const;

        // JsonReaderHelper
        const std::string& GetItemGems()
        {
            if (!LoadJsonText(strItemGems, "item-gems.json").empty())
            {
                return strItemGems;
            }

            return JsonReaderHelper::GetItemGems();
        }

        const std::string& GetItemModifiers()
        {
            if (!LoadJsonText(strItemModifiers, "item-modifiers.json").empty())
            {
                return strItemModifiers;
            }

            return JsonReaderHelper::GetItemModifiers();
        }

        const std::string& GetItemNameAffixes()
        {
            if (!LoadJsonText(strItemNameAffixes, "item-nameaffixes.json").empty())
            {
                return strItemNameAffixes;
            }

            return JsonReaderHelper::GetItemNameAffixes();
        }

        const std::string& GetItemNames()
        {
            if (!LoadJsonText(strItemNames, "item-names.json").empty())
            {
                return strItemNames;
            }

            return JsonReaderHelper::GetItemNames();
        }

        const std::string& GetItemRunes()
        {
            if (!LoadJsonText(strItemRunes, "item-runes.json").empty())
            {
                return strItemRunes;
            }

            return JsonReaderHelper::GetItemRunes();
        }

        const std::string& GetLevels()
        {
            if (!LoadJsonText(strLevels, "levels.json").empty())
            {
                return strLevels;
            }

            return JsonReaderHelper::GetLevels();
        }

        const std::string& GetMercenaries()
        {
            if (!LoadJsonText(strMercenaries, "monsters.json").empty())
            {
                return strMercenaries;
            }

            return JsonReaderHelper::GetMercenaries();
        }

        const std::string& GetMonsters()
        {
            if (!LoadJsonText(strMonsters, "monsters.json").empty())
            {
                return strMonsters;
            }

            return JsonReaderHelper::GetMonsters();
        }

        const std::string& GetNPCs()
        {
            if (!LoadJsonText(strNPCs, "npcs.json").empty())
            {
                return strNPCs;
            }

            return JsonReaderHelper::GetNPCs();
        }

        const std::string& GetQuests()
        {
            if (!LoadJsonText(strQuests, "quests.json").empty())
            {
                return strQuests;
            }

            return JsonReaderHelper::GetQuests();
        }

        const std::string& GetSkills()
        {
            if (!LoadJsonText(strSkills, "skills.json").empty())
            {
                return strSkills;
            }

            return JsonReaderHelper::GetSkills();
        }

        const std::string& GetUI()
        {
            if (!LoadJsonText(strUI, "ui.json").empty())
            {
                return strUI;
            }

            return JsonReaderHelper::GetUI();
        }

        // TxtReaderHelper
        const std::string& GetArmor()
        {
            auto& strValue = strArmor;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "armor.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "armor.bin", D2RBinReader::readArmorBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetArmor();
        }

        const std::string& GetBelts()
        {
            auto& strValue = strBelts;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "belts.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "belts.bin", D2RBinReader::readBeltsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetBelts();
        }

        const std::string& GetBodyLocs()
        {
            auto& strValue = strBodyLocs;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "bodylocs.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "bodylocs.bin", D2RBinReader::readBodyLocsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetBodyLocs();
        }

        const std::string& GetCharStats()
        {
            auto& strValue = strCharStats;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "charstats.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "charstats.bin", D2RBinReader::readCharStatsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetCharStats();
        }

        const std::string& GetExperience()
        {
            auto& strValue = strExperience;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "experience.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "experience.bin", D2RBinReader::readExperienceBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetExperience();
        }

        const std::string& GetGems()
        {
            auto& strValue = strGems;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "gems.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "gems.bin", D2RBinReader::readGemsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetGems();
        }

        const std::string& GetHireling()
        {
            auto& strValue = strHireling;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "hireling.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "hireling.bin", D2RBinReader::readHirelingBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetHireling();
        }

        const std::string& GetItemStatCost()
        {
            auto& strValue = strItemStatCost;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "itemstatcost.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "itemstatcost.bin", D2RBinReader::readItemStatCostBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetItemStatCost();
        }

        const std::string& GetItemsCode()
        {
            auto& strValue = strItemsCode;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelItemsCodeBinText(strValue).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetItemsCode();
        }

        const std::string& GetItemTypes()
        {
            auto& strValue = strItemTypes;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "itemtypes.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "itemtypes.bin", D2RBinReader::readItemTypesBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetItemTypes();
        }

        const std::string& GetMagicPrefix()
        {
            auto& strValue = strMagicPrefix;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "magicprefix.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "magicprefix.bin", D2RBinReader::readMagicAffixBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetMagicPrefix();
        }

        const std::string& GetMagicSuffix()
        {
            auto& strValue = strMagicSuffix;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "magicsuffix.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "magicsuffix.bin", D2RBinReader::readMagicAffixBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetMagicSuffix();
        }

        const std::string& GetMisc()
        {
            auto& strValue = strMisc;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "misc.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "misc.bin", D2RBinReader::readMiscBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetMisc();
        }

        const std::string& GetPlayerClass()
        {
            auto& strValue = strPlayerClass;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "playerclass.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "playerclass.bin", D2RBinReader::readPlayerClassBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetPlayerClass();
        }

        const std::string& GetProperties()
        {
            auto& strValue = strProperties;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "properties.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "properties.bin", D2RBinReader::readPropertiesBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetProperties();
        }

        const std::string& GetRarePrefix()
        {
            auto& strValue = strRarePrefix;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "rareprefix.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "rareprefix.bin", D2RBinReader::readRareAffixBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetRarePrefix();
        }

        const std::string& GetRareSuffix()
        {
            auto& strValue = strRareSuffix;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "raresuffix.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "raresuffix.bin", D2RBinReader::readRareAffixBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetRareSuffix();
        }

        const std::string& GetRunes()
        {
            auto& strValue = strRunes;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "runes.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "runes.bin", D2RBinReader::readRunesBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetRunes();
        }

        const std::string& GetSetItems()
        {
            auto& strValue = strSetItems;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "setitems.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "setitems.bin", D2RBinReader::readSetItemsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetSetItems();
        }

        const std::string& GetSets()
        {
            auto& strValue = strSets;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "sets.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "sets.bin", D2RBinReader::readSetsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetSets();
        }

        const std::string& GetSkillDesc()
        {
            auto& strValue = strSkillDesc;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "skilldesc.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "skilldesc.bin", D2RBinReader::readSkillDescBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetSkillDesc();
        }

        const std::string& GetSkillsTxt()
        {
            auto& strValue = strSkillsTxt;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "skills.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "skills.bin", D2RBinReader::readSkillsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetSkills();
        }

        const std::string& GetUniqueItems()
        {
            auto& strValue = strUniqueItems;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "uniqueitems.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "uniqueitems.bin", D2RBinReader::readUniqueItemsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetUniqueItems();
        }

        const std::string& GetWeapons()
        {
            auto& strValue = strWeapons;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "weapons.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "weapons.bin", D2RBinReader::readWeaponsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetWeapons();
        }

        const std::string& GetMonStats()
        {
            auto& strValue = strMonStats;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "monstats.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "monstats.bin", D2RBinReader::readMonStatsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetMonStats();
        }

        const std::string& GetMonType()
        {
            auto& strValue = strMonType;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "montype.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "montype.bin", D2RBinReader::readMonTypeBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetMonType();
        }

        const std::string& GetSuperiorModsTxt()
        {
            auto& strValue = strSuperiorModsTxt;
            if (!strValue.empty())
            {
                return strValue;
            }

            if (!LoadExcelText(strValue, "qualityitems.txt").empty())
            {
                return strValue;
            }

            if (!LoadExcelBinText(strValue, "qualityitems.bin", D2RBinReader::readQualityItemsBin).empty())
            {
                return strValue;
            }

            return TxtReaderHelper::GetSuperiorModsTxt();
        }

        const std::string& GetCraftModsTxt()
        {
            return TxtReaderHelper::GetCraftModsTxt();
        }

    private:
        void LoadAll()
        {
            Init();
            if (modPath.empty())
            {
                return;
            }

            // load up all JsonReaderHelper strings
            GetItemGems();
            GetItemModifiers();
            GetItemNameAffixes();
            GetItemNames();
            GetItemRunes();
            GetLevels();
            GetMercenaries();
            GetMonsters();
            GetNPCs();
            GetQuests();
            GetSkills();
            GetUI();

            // load up all TxtReaderHelper strings
            GetArmor();
            GetBelts();
            GetCharStats();
            GetExperience();
            GetGems();
            GetHireling();
            GetItemsCode();
            GetItemStatCost();
            GetItemTypes();
            GetMagicPrefix();
            GetMagicSuffix();
            GetMisc();
            GetPlayerClass();
            GetProperties();
            GetRarePrefix();
            GetRareSuffix();
            GetRunes();
            GetSetItems();
            GetSets();
            GetSkillDesc();
            GetSkills();
            GetUniqueItems();
            GetWeapons();
            GetMonStats();
            GetMonType();
            GetSuperiorModsTxt();
            GetCraftModsTxt();

            // done reading MPQ archive, so prevent any future read attemps
            if (hMpq != NULL)
            {
                SFileCloseArchive(hMpq);
                hMpq = NULL;
            }

            modPath.clear();
            modStringsPath.clear();
            modExcelPath.clear();

            // clear maps
            stringTxtInfo.clear();
            stringEnTxtByKey.clear();
            itemTypeMap.clear();
            itemCodeMap.clear();
            propertyCodeDefaultMap.clear();
            propertyCodeMap.clear();
            bodyLocsMap.clear();
            itemTypeNameMap.clear();
            itemDefaultStatMap.clear();
            itemStatMap.clear();
            setsMap.clear();
            skillsMap.clear();
            skillDesDefaultMap.clear();
            skillDescMap.clear();
            playerClassCodeMap.clear();
            itemDefaultMonTypeMap.clear();
            itemMonTypeMap.clear();
            calcExpressionMap.clear();
        }

        template <typename F>
        std::unique_ptr<ITxtDocument> LoadTxtStream(F f,
            const rapidcsv::LabelParams& pLabelParams = rapidcsv::LabelParams(),
            const rapidcsv::SeparatorParams& pSeparatorParams = rapidcsv::SeparatorParams('\t'),
            const rapidcsv::ConverterParams& pConverterParams = rapidcsv::ConverterParams(),
            const rapidcsv::LineReaderParams& pLineReaderParams = rapidcsv::LineReaderParams(),
            size_t idx = 0) const
        {
            std::istringstream stream(f);
            return std::make_unique<CsvTxtDocument>(stream, pLabelParams, pSeparatorParams, pConverterParams, pLineReaderParams, idx);
        }

        void processBodyLocs()
        {
            // we need to map id to excel code column string
            if (bodyLocsMap.empty())
            {
                auto pDoc = LoadTxtStream(GetBodyLocs());
                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T codeColumnIdx = pDoc->GetColumnIdx("Code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    bodyLocsMap[i] = pDoc->GetCellString(codeColumnIdx, i);
                }
            }
        }

        void processCalcExpression()
        {
            if (calcExpressionMap.empty())
            {
                auto pDoc = LoadTxtStream(GetItemsCode());
                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T codeColumnIdx = pDoc->GetColumnIdx("code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    calcExpressionMap[i] = pDoc->GetCellString(codeColumnIdx, i);
                }
            }
        }

        void processStrings()
        {
            // we need to map id to excel code column string
            if (stringTxtInfo.empty())
            {
                auto pDoc = GetStringTxt();
                if (pDoc == nullptr)
                {
                    return;
                }

                if (pDoc->GetColumnCount() == 0)
                {
                    // corrupt
                    return;
                }

                size_t numRows = pDoc->GetRowCount();
                std::string strName;
                std::map<std::string, std::string> stringCols;
                size_t idx = 0;
                for (size_t i = 0; i < numRows; ++i)
                {
                    idx = pDoc->GetRowValues(i, strName, stringCols);
                    if (strName.empty() || stringCols.empty() || (idx == MAXSIZE_T))
                    {
                        continue;
                    }

                    stringTxtInfo[std::uint32_t(idx)] = strName;
                    if (strName.compare("x") != 0 && strName.compare("X") != 0)
                    {
                        auto iter = stringCols.find("enUS");
                        if (iter == stringCols.end())
                        {
                            iter = stringCols.begin();
                        }

                        if (iter != stringCols.end())
                        {
                            stringEnTxtByKey[strName] = ColorHelpers::RemoveColorFromText(iter->second.c_str());
                        }
                    }
                }
            }
        }

        void processStatDefaultName()
        {
            // we need to map id to excel code column string
            if (itemDefaultStatMap.empty())
            {
                auto pDoc = LoadTxtStream(TxtReaderHelper::GetItemStatCost());
                if (pDoc == nullptr)
                {
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T nameColumnIdx = pDoc->GetColumnIdx("Stat");
                if (nameColumnIdx < 0)
                {
                    return;
                }

                SSIZE_T idColumnIdx = pDoc->GetColumnIdx("*ID");
                if (idColumnIdx < 0)
                {
                    idColumnIdx = pDoc->GetColumnIdx("ID");  // support the alternate version of this file format
                    if (idColumnIdx < 0)
                    {
                        return;
                    }
                }

                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(nameColumnIdx, i);
                    strValue.erase(std::find_if(strValue.rbegin(), strValue.rend(), [](int ch) { return !isspace(ch); }).base(), strValue.end());
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    itemDefaultStatMap[std::uint32_t(pDoc->GetCellUInt16(idColumnIdx, i))] = strValue;
                }
            }
        }

        void processStatName()
        {
            // we need to map id to excel code column string
            if (itemStatMap.empty())
            {
                auto pDoc = LoadTxtStream(GetItemStatCost());
                if (pDoc == nullptr)
                {
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T nameColumnIdx = pDoc->GetColumnIdx("Stat");
                if (nameColumnIdx < 0)
                {
                    return;
                }

                SSIZE_T idColumnIdx = pDoc->GetColumnIdx("*ID");
                if (idColumnIdx < 0)
                {
                    idColumnIdx = pDoc->GetColumnIdx("ID");  // support the alternate version of this file format
                    if (idColumnIdx < 0)
                    {
                        return;
                    }
                }

                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(nameColumnIdx, i);
                    strValue.erase(std::find_if(strValue.rbegin(), strValue.rend(), [](int ch) { return !isspace(ch); }).base(), strValue.end());
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    itemStatMap[std::uint32_t(pDoc->GetCellUInt16(idColumnIdx, i))] = strValue;
                }
            }
        }

        void processItemTypeName()
        {
            // we need to map id to excel code column string
            if (itemTypeNameMap.empty())
            {
                auto pDoc = LoadTxtStream(TxtReaderHelper::GetItemTypes());
                if (pDoc == nullptr)
                {
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T nameColumnIdx = pDoc->GetColumnIdx("ItemType");
                if (nameColumnIdx < 0)
                {
                    return;
                }

                const SSIZE_T codeColumnIdx = pDoc->GetColumnIdx("Code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(codeColumnIdx, i);
                    strValue.erase(std::find_if(strValue.rbegin(), strValue.rend(), [](int ch) { return !isspace(ch); }).base(), strValue.end());
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    itemTypeNameMap[strValue] = pDoc->GetCellString(nameColumnIdx, i);
                }
            }
        }

        void processItemType()
        {
            // we need to map id to excel code column string
            if (itemTypeMap.empty())
            {
                // we need to map id to excel type column string
                auto pDoc = LoadTxtStream(GetItemTypes());
                if (pDoc == nullptr)
                {
                    return;
                }

                if (!itemTypeMap.empty())
                {
                    // was initialized above
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T codeColumnIdx = pDoc->GetColumnIdx("Code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    itemTypeMap[i] = pDoc->GetCellString(codeColumnIdx, i);
                }
            }
        }

        void processItemCode()
        {
            // we need to map id to excel code column string
            if (itemCodeMap.empty())
            {
                std::uint32_t id = 0;

                // we need to map id to excel type column string
                auto pDoc = LoadTxtStream(GetWeapons());
                if (pDoc == nullptr)
                {
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                SSIZE_T nameColumnIdx = pDoc->GetColumnIdx("name");
                if (nameColumnIdx < 0)
                {
                    return;
                }

                SSIZE_T codeColumnIdx = pDoc->GetColumnIdx("code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                std::string name;
                std::string code;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    code = pDoc->GetCellString(codeColumnIdx, i);
                    if (code.empty())
                    {
                        // skip
                        continue;
                    }

                    name = pDoc->GetCellString(nameColumnIdx, i);
                    if (name.empty() || name == "Unused")
                    {
                        // skip
                        continue;
                    }

                    itemCodeMap[id] = code;
                    ++id;
                }

                pDoc = LoadTxtStream(GetArmor());
                if (pDoc == nullptr)
                {
                    return;
                }

                numRows = std::uint32_t(pDoc->GetRowCount());
                nameColumnIdx = pDoc->GetColumnIdx("name");
                if (nameColumnIdx < 0)
                {
                    return;
                }

                codeColumnIdx = pDoc->GetColumnIdx("code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    code = pDoc->GetCellString(codeColumnIdx, i);
                    if (code.empty())
                    {
                        // skip
                        continue;
                    }

                    name = pDoc->GetCellString(nameColumnIdx, i);
                    if (name.empty() || name == "Unused")
                    {
                        // skip
                        continue;
                    }

                    itemCodeMap[id] = code;
                    ++id;
                }

                pDoc = LoadTxtStream(GetMisc());
                if (pDoc == nullptr)
                {
                    return;
                }

                numRows = std::uint32_t(pDoc->GetRowCount());
                nameColumnIdx = pDoc->GetColumnIdx("name");
                if (nameColumnIdx < 0)
                {
                    return;
                }

                codeColumnIdx = pDoc->GetColumnIdx("code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    code = pDoc->GetCellString(codeColumnIdx, i);
                    if (code.empty())
                    {
                        // skip
                        continue;
                    }

                    name = pDoc->GetCellString(nameColumnIdx, i);
                    if (name.empty() || name == "Unused")
                    {
                        // skip
                        continue;
                    }

                    itemCodeMap[id] = code;
                    ++id;
                }
            }
        }

        void processPropertyCodeName()
        {
            // we need to map id to excel code column string
            if (propertyCodeDefaultMap.empty())
            {
                // we need to map id to excel code column string
                auto pDoc = LoadTxtStream(TxtReaderHelper::GetProperties());
                if (pDoc == nullptr)
                {
                    return;
                }

                if (!propertyCodeDefaultMap.empty())
                {
                    // was initialized above
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T codeColumnIdx = pDoc->GetColumnIdx("code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                std::uint32_t id = 0;
                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(codeColumnIdx, i);;
                    if (strValue == "Expansion")
                    {
                        // skip
                        continue;
                    }

                    propertyCodeDefaultMap[id] = strValue;
                    ++id;
                }
            }
        }

        void processPropertyCode()
        {
            // we need to map id to excel code column string
            if (propertyCodeMap.empty())
            {
                // we need to map id to excel code column string
                auto pDoc = LoadTxtStream(GetProperties());
                if (pDoc == nullptr)
                {
                    return;
                }

                if (!propertyCodeMap.empty())
                {
                    // was initialized above
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T codeColumnIdx = pDoc->GetColumnIdx("code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                std::uint32_t id = 0;
                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(codeColumnIdx, i);;
                    if (strValue == "Expansion")
                    {
                        // skip
                        continue;
                    }

                    propertyCodeMap[id] = strValue;
                    ++id;
                }
            }
        }

        void processSetIndex()
        {
            // we need to map id to excel code column string
            if (setsMap.empty())
            {
                // we need to map id to excel code column string
                auto pDoc = LoadTxtStream(GetSets());
                if (pDoc == nullptr)
                {
                    return;
                }

                if (!setsMap.empty())
                {
                    // was initialized above
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T indexColumnIdx = pDoc->GetColumnIdx("index");
                if (indexColumnIdx < 0)
                {
                    return;
                }

                const SSIZE_T versionColumnIdx = pDoc->GetColumnIdx("version");
                if (versionColumnIdx < 0)
                {
                    return;
                }

                std::string strValue;
                std::uint16_t id = 0;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(indexColumnIdx, i);
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    if (pDoc->GetCellString(versionColumnIdx, i).empty())
                    {
                        // skip
                        continue;
                    }

                    setsMap[id] = strValue;
                    ++id;
                }
            }
        }

        void processSkills()
        {
            // we need to map id to excel code column string
            if (skillsMap.empty())
            {
                // we need to map id to excel code column string
                auto pDoc = LoadTxtStream(GetSkillsTxt());
                if (pDoc == nullptr)
                {
                    return;
                }

                if (!skillsMap.empty())
                {
                    // was initialized above
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T skillColumnIdx = pDoc->GetColumnIdx("skill");
                if (skillColumnIdx < 0)
                {
                    return;
                }

                SSIZE_T idColumnIdx = pDoc->GetColumnIdx("*Id");
                if (idColumnIdx < 0)
                {
                    idColumnIdx = pDoc->GetColumnIdx("Id"); // support the alternate version of this file format
                    if (idColumnIdx < 0)
                    {
                        return;
                    }
                }

                std::string strValue;
                std::uint16_t id = MAXUINT16;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(idColumnIdx, i);
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    id = pDoc->GetCellUInt16(idColumnIdx, i);
                    strValue = pDoc->GetCellString(skillColumnIdx, i);
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }
                    skillsMap[id] = strValue;
                }
            }
        }

        void processSkillDescDefaultName()
        {
            // we need to map id to excel code column string
            if (skillDesDefaultMap.empty())
            {
                auto pDoc = LoadTxtStream(TxtReaderHelper::GetSkillDesc());
                if (pDoc == nullptr)
                {
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T skilldescColumnIdx = pDoc->GetColumnIdx("skilldesc");
                if (skilldescColumnIdx < 0)
                {
                    return;
                }

                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(skilldescColumnIdx, i);
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    skillDesDefaultMap[i] = strValue;
                }
            }
        }

        void processSkillDesc()
        {
            // we need to map id to excel code column string
            if (skillDescMap.empty())
            {
                // we need to map id to excel code column string
                auto pDoc = LoadTxtStream(GetSkillDesc());
                if (pDoc == nullptr)
                {
                    return;
                }

                if (!skillDescMap.empty())
                {
                    // was initialized above
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T skilldescColumnIdx = pDoc->GetColumnIdx("skilldesc");
                if (skilldescColumnIdx < 0)
                {
                    return;
                }

                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(skilldescColumnIdx, i);
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    skillDescMap[i] = strValue;
                }
            }
        }

        void processPlayerClass()
        {
            // we need to map id to excel code column string
            if (playerClassCodeMap.empty())
            {
                // we need to map id to excel code column string
                auto pDoc = LoadTxtStream(GetPlayerClass());
                if (pDoc == nullptr)
                {
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T codeColumnIdx = pDoc->GetColumnIdx("Code");
                if (codeColumnIdx < 0)
                {
                    return;
                }

                std::uint32_t id = 0;
                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(codeColumnIdx, i);
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    playerClassCodeMap[id] = strValue;
                    ++id;
                }
            }
        }

        void processMonTypeDefaultName()
        {
            // we need to map id to excel type column string
            if (itemDefaultMonTypeMap.empty())
            {
                auto pDoc = LoadTxtStream(TxtReaderHelper::GetMonType());
                if (pDoc == nullptr)
                {
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T typeColumnIdx = pDoc->GetColumnIdx("type");
                if (typeColumnIdx < 0)
                {
                    return;
                }

                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(typeColumnIdx, i);
                    strValue.erase(std::find_if(strValue.rbegin(), strValue.rend(), [](int ch) { return !isspace(ch); }).base(), strValue.end());
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    itemDefaultMonTypeMap[i] = strValue;
                }
            }
        }

        void processMonType()
        {
            // we need to map id to excel type column string
            if (itemMonTypeMap.empty())
            {
                auto pDoc = LoadTxtStream(GetMonType());
                if (pDoc == nullptr)
                {
                    return;
                }

                std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
                const SSIZE_T typeColumnIdx = pDoc->GetColumnIdx("type");
                if (typeColumnIdx < 0)
                {
                    return;
                }

                std::string strValue;
                for (std::uint32_t i = 0; i < numRows; ++i)
                {
                    strValue = pDoc->GetCellString(typeColumnIdx, i);
                    strValue.erase(std::find_if(strValue.rbegin(), strValue.rend(), [](int ch) { return !isspace(ch); }).base(), strValue.end());
                    if (strValue.empty())
                    {
                        // skip
                        continue;
                    }

                    itemMonTypeMap[i] = strValue;
                }
            }
        }

        void Init()
        {
            if (modPath.empty() || !std::filesystem::exists(modPath))
            {
                modPath.clear();
                return;
            }

            // check for mpq file or directory
            modName = modPath.filename().u8string();
            modPath /= modPath.filename();
            modPath.replace_extension(".mpq");
            if (!std::filesystem::exists(modPath))
            {
                modPath.clear();
                modName.clear();
                return;
            }

            if (std::filesystem::is_directory(modPath))
            {
                // we are directly dealing with json and csv files
                hMpq = NULL;
                modPath /= "data";
                if (!std::filesystem::exists(modPath) || !std::filesystem::is_directory(modPath))
                {
                    modPath.clear();
                    modName.clear();
                    return;
                }

                // Looks for the strings path
                modStringsPath = modPath / "local" / "lng" / "strings";
                if (!std::filesystem::exists(modStringsPath) || !std::filesystem::is_directory(modStringsPath))
                {
                    modStringsPath.clear();
                }

                // Looks for the excels path
                modExcelPath = modPath / "global" / "excel";
                if (!std::filesystem::exists(modExcelPath) || !std::filesystem::is_directory(modExcelPath))
                {
                    modExcelPath.clear();
                }

                return;
            }

            {
                auto path = modPath.wstring();
                if (!SFileOpenArchive(path.c_str(), 0, MPQ_OPEN_READ_ONLY, &hMpq))
                {
                    hMpq = NULL;
                    modPath.clear();
                    modName.clear();
                    return;
                }
            }
        }

        std::string& LoadJsonText(std::string& jsonStr, const std::string& jsonFileName)
        {
            if (!jsonStr.empty() || modPath.empty())
            {
                return jsonStr;
            }

            if (hMpq != NULL)
            {
                std::stringstream ss;
                ss << "data\\local\\lng\\strings\\";
                ss << jsonFileName;
                jsonStr = LoadTextFromMpq(ss.str());
                return jsonStr;
            }

            if (modStringsPath.empty())
            {
                return jsonStr;
            }

            std::filesystem::path path = modStringsPath / jsonFileName;
            jsonStr = LoadTextFromFile(path);
            return jsonStr;
        }

        std::string& LoadExcelText(std::string& excelStr, const std::string& excelFileName)
        {
            if (!excelStr.empty() || modPath.empty())
            {
                return excelStr;
            }

            if (hMpq != NULL)
            {
                std::stringstream ss;
                ss << "data\\global\\excel\\";
                ss << excelFileName;
                excelStr = LoadTextFromMpq(ss.str());
                return excelStr;
            }

            if (modExcelPath.empty())
            {
                return excelStr;
            }

            std::filesystem::path path = modExcelPath / excelFileName;
            excelStr = LoadTextFromFile(path);
            return excelStr;
        }

        std::string& LoadExcelItemsCodeBinText(std::string& excelStr)
        {
            if (!excelStr.empty() || modPath.empty())
            {
                return excelStr;
            }

            std::string binText;
            if (hMpq != NULL)
            {
                std::stringstream ss;
                binText = LoadTextFromMpq("data\\global\\excel\\itemscode.bin");
                if (binText.empty())
                {
                    return excelStr;
                }

                excelStr = D2RBinReader::readItemsCodeBin(binText, *this);
                return excelStr;
            }

            if (modExcelPath.empty())
            {
                return excelStr;
            }

            std::filesystem::path path = modExcelPath / "itemscode.bin";
            binText = LoadTextFromFile(path);
            excelStr = D2RBinReader::readItemsCodeBin(binText, *this);
            return excelStr;
        }

        std::string& LoadExcelBinText(std::string& excelStr, const std::string& excelFileName, std::function<std::string(const std::string&, const D2RModReaderHelper&)> binReader)
        {
            if (!excelStr.empty() || modPath.empty())
            {
                return excelStr;
            }

            std::string binText;
            if (hMpq != NULL)
            {
                std::stringstream ss;
                ss << "data\\global\\excel\\";
                ss << excelFileName;
                binText = LoadTextFromMpq(ss.str());
                if (binText.empty())
                {
                    return excelStr;
                }

                excelStr = binReader(binText, *this);
                return excelStr;
            }

            if (modExcelPath.empty())
            {
                return excelStr;
            }

            std::filesystem::path path = modExcelPath / excelFileName;
            binText = LoadTextFromFile(path);
            excelStr = binReader(binText, *this);
            return excelStr;
        }

        static std::string LoadTextFromFile(const std::filesystem::path& path)
        {
            if (!std::filesystem::exists(path))
            {
                return std::string();
            }

            std::ifstream modFile(path, std::ios::binary);
            if (!modFile.is_open() || !modFile.good() || modFile.eof())
            {
                return std::string();
            }
            return std::string((std::istreambuf_iterator<char>(modFile)), std::istreambuf_iterator<char>());
        }

        std::string LoadTextFromMpq(const std::string& path)
        {
            if ((hMpq == NULL) || !SFileHasFile(hMpq, path.c_str()))
            {
                return std::string();
            }

            HANDLE hFile = NULL;
            if (!SFileOpenFileEx(hMpq, path.c_str(), SFILE_OPEN_FROM_MPQ, &hFile))
            {
                return std::string();
            }

            auto fileSize = SFileGetFileSize(hFile, NULL);
            if (fileSize == 0)
            {
                SFileCloseFile(hFile);
                hFile = NULL;
                return std::string();
            }

            std::string ret;
            ret.resize(size_t(fileSize) + 1);
            DWORD numRead = 0;
            if (!SFileReadFile(hFile, ret.data(), fileSize, &numRead, NULL))
            {
                SFileCloseFile(hFile);
                hFile = NULL;
                return std::string();
            }

            SFileCloseFile(hFile);
            hFile = NULL;
            return ret;
        }

    private:
        std::filesystem::path modPath;
        std::filesystem::path modStringsPath;
        std::filesystem::path modExcelPath;
        HANDLE hMpq = NULL;
        std::string modName;

        std::string strItemGems;
        std::string strItemModifiers;
        std::string strItemNameAffixes;
        std::string strItemNames;
        std::string strItemRunes;
        std::string strLevels;
        std::string strMercenaries;
        std::string strMonsters;
        std::string strNPCs;
        std::string strQuests;
        std::string strSkills;
        std::string strUI;

        std::string strArmor;
        std::string strBelts;
        std::string strBodyLocs;
        std::string strCharStats;
        std::string strExperience;
        std::string strGems;
        std::string strHireling;
        std::string strItemStatCost;
        std::string strItemTypes;
        std::string strMagicPrefix;
        std::string strMagicSuffix;
        std::string strMisc;
        std::string strPlayerClass;
        std::string strProperties;
        std::string strRarePrefix;
        std::string strRareSuffix;
        std::string strRunes;
        std::string strSetItems;
        std::string strSets;
        std::string strSkillDesc;
        std::string strSkillsTxt;
        std::string strUniqueItems;
        std::string strWeapons;
        std::string strMonStats;
        std::string strMonType;
        std::string strSuperiorModsTxt;
        std::string strCraftModsTxt;
        std::string strItemsCode;
        
        std::map<std::uint32_t, std::string> stringTxtInfo;
        std::map<std::string, std::string> stringEnTxtByKey;
        std::map<std::uint32_t, std::string> itemTypeMap;
        std::map<std::uint32_t, std::string> itemCodeMap;
        std::map<std::uint32_t, std::string> propertyCodeDefaultMap;
        std::map<std::uint32_t, std::string> propertyCodeMap;
        std::map<std::uint32_t, std::string> bodyLocsMap;
        std::map<std::string, std::string> itemTypeNameMap;
        std::map<std::uint32_t, std::string> itemDefaultStatMap;
        std::map<std::uint32_t, std::string> itemStatMap;
        std::map<std::uint32_t, std::string> setsMap;
        std::map<std::uint32_t, std::string> skillsMap;
        std::map<std::uint32_t, std::string> skillDesDefaultMap;
        std::map<std::uint32_t, std::string> skillDescMap;
        std::map<std::uint32_t, std::string> playerClassCodeMap;
        std::map<std::uint32_t, std::string> itemDefaultMonTypeMap;
        std::map<std::uint32_t, std::string> itemMonTypeMap;
        std::map<std::uint32_t, std::string> calcExpressionMap;
    };

    std::string D2RBinReader::readArmorBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStrings(); // needed for usage below
            pParent->processItemType();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> typeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemType);
        static std::function<std::string(std::string, const D2RModReaderHelper&, size_t)> itemNameFunc = std::function<std::string(std::string, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemName);
        static std::function<bool(const std::string&, size_t)> isExpansionRow = std::function<bool(const std::string&, size_t)>(D2RBinReader::isExpansionVersion);

        size_t versionTestCol = 13;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"name", 4, false, true, nullptr, nullptr, &itemNameFunc}, {"version", versionTestCol}, {"compactsave", 35}, {"minac", 9}, {"maxac", 10},
            {"reqstr", 18}, {"reqdex", 19}, {"durability", 22}, {"nodurability", 23}, {"level", 15}, {"levelreq", 32}, {"code", 4}, {"namestr", 4, false, true},
            {"magic lvl", 33, true}, {"auto prefix", 14, true}, {"alternategfx", 8}, {"normcode", 5}, {"ubercode", 6}, {"ultracode", 7}, {"invwidth", 20},
            {"invheight", 21}, {"gemsockets", 30}, {"gemapplytype", 31}, {"flippyfile", 0}, {"invfile", 1}, {"uniqueinvfile", 2}, {"setinvfile", 3}, {"stackable", 29, true},
            {"minstack", 11, true}, {"maxstack", 12, true}, {"type", 24, true, false, nullptr, &typeFunc}, {"type2", 25, true, false, nullptr, &typeFunc}, {"unique", 26},
            {"belt", 28},  {"quest", 27, true}, {"mindam", 16}, {"maxdam", 17}, {"InvTrans", 34},  {"SkipName", 36}, {"Nameable", 37}
        };
        static D2RFormatDescriptor desc("a32a32a32a32a4a4a4a4a4x64uux16uux10bx1wx3bbbx10wwbbx1bbx11bbx9bwx4wbx5bx5bbbx1bbbbx102", format, versionTestCol);
        return processBin(data, length, desc, parent, &isExpansionRow);
    }

    std::string D2RBinReader::readBeltsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"numboxes", 0} };
        static D2RFormatDescriptor desc("x4ux280", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readBodyLocsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processBodyLocs(); // needed for usage below
        }

        static std::function<std::string(std::string, const D2RModReaderHelper&, size_t)> bodyLocNameFunc = std::function<std::string(std::string, const D2RModReaderHelper&, size_t)>(D2RBinReader::processBodyLocation);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"Body Location", 0, false, true, nullptr, nullptr, &bodyLocNameFunc}, {"Code", 0 } };
        static D2RFormatDescriptor desc("a4", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readCharStatsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStrings(); // needed for usage below
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> stringResource = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processStringId);
        static std::function<bool(const std::string&, size_t)> isExpansionRow = std::function<bool(const std::string&, size_t)>(D2RBinReader::isExpansionPlayerClass);

        size_t versionTestCol = 0;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"class", versionTestCol}, {"str", 1}, {"dex", 2}, {"int", 3}, {"vit", 4}, {"stamina", 5}, {"hpadd", 6},
            {"LifePerLevel", 7}, {"StaminaPerLevel", 8}, {"ManaPerLevel", 9}, {"LifePerVitality", 10}, {"StaminaPerVitality", 11}, {"ManaPerMagic", 12}, {"StatPerLevel", 13},
            {"SkillsPerLevel", 14}, {"StrAllSkills", 15, true, false, nullptr, &stringResource}, {"StrSkillTab1", 16, true, false, nullptr, &stringResource},
            {"StrSkillTab2", 17, true, false, nullptr, &stringResource}, {"StrSkillTab3", 18, true, false, nullptr, &stringResource},
            {"StrClassOnly", 19, true, false, nullptr, &stringResource}
        };
        static D2RFormatDescriptor desc("x32a16bbbbbbx6bbbbx1bbx9bbx6wwwwwx114", format, versionTestCol);
        return processBin(data, length, desc, parent, &isExpansionRow);
    }

    std::string D2RBinReader::readExperienceBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> experienceLevelFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processExperienceLevel);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"Level", 0, false, true, nullptr, &experienceLevelFunc}, {"Amazon", 0}, {"Sorceress", 1}, {"Necromancer", 2}, {"Paladin", 3}, {"Barbarian", 4}, {"Druid", 5}, {"Assassin", 6}
        };
        static D2RFormatDescriptor desc("uuuuuuux4", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readGemsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processItemCode(); // needed for usage below
            pParent->processPropertyCode();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> codeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemCode);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> propertyFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPropertyCode);
        static std::function<bool(const std::string&, size_t)> isExpansionRow = std::function<bool(const std::string&, size_t)>(D2RBinReader::isExpansionGem);

        size_t versionTestCol = 1;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"name", 0}, {"letter", versionTestCol}, {"transform", 3}, {"code", 2, false, false, nullptr, &codeFunc},
            {"weaponMod1Code", 4, false, false, nullptr, &propertyFunc}, {"weaponMod1Param", 5, true}, {"weaponMod1Min", 6, true}, {"weaponMod1Max", 7, true},
            {"weaponMod2Code", 8, false, false, nullptr, &propertyFunc}, {"weaponMod2Param", 9, true}, {"weaponMod2Min", 10, true}, {"weaponMod2Max", 11, true},
            {"weaponMod3Code", 12, false, false, nullptr, &propertyFunc}, {"weaponMod3Param", 13, true}, {"weaponMod3Min", 14, true}, {"weaponMod3Max", 15, true},
            {"helmMod1Code", 16, false, false, nullptr, &propertyFunc}, {"helmMod1Param", 17, true}, {"helmMod1Min", 18, true}, {"helmMod1Max", 19, true},
            {"helmMod2Code", 20, false, false, nullptr, &propertyFunc}, {"helmMod2Param", 21, true}, {"helmMod2Min", 22, true}, {"helmMod2Max", 23, true},
            {"helmMod3Code", 24, false, false, nullptr, &propertyFunc}, {"helmMod3Param", 25, true}, {"helmMod3Min", 26, true}, {"helmMod3Max", 27, true},
            {"shieldMod1Code", 28, false, false, nullptr, &propertyFunc}, {"shieldMod1Param", 29, true}, {"shieldMod1Min", 30, true}, {"shieldMod1Max", 31, true},
            {"shieldMod2Code", 32, false, false, nullptr, &propertyFunc}, {"shieldMod2Param", 33, true}, {"shieldMod2Min", 34, true}, {"shieldMod2Max", 35, true},
            {"shieldMod3Code", 36, false, false, nullptr, &propertyFunc}, {"shieldMod3Param", 37, true}, {"shieldMod3Min", 38, true}, {"shieldMod3Max", 39, true},
        };
        static D2RFormatDescriptor desc("a32a8ux3buuiiuuiiuuiiuuiiuuiiuuiiuuiiuuiiuuii", format, versionTestCol);
        return processBin(data, length, desc, parent, &isExpansionRow);
    }
    
    std::string D2RBinReader::readHirelingBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStrings(); // needed for usage below
            pParent->processSkills();
            pParent->processPlayerClass();
        }


        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> hirelingType = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processHirelingType);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> stringResource = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processStringId);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> skillName = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processSkillName);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> classCode = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPlayerClassCode);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"Hireling", 2, false, true, nullptr, &hirelingType}, {"Version", 0}, {"Id", 1}, {"Class", 2}, {"Act", 3}, {"Difficulty", 4}, {"Level", 7}, {"Seller", 5}, {"NameFirst", 51},
            {"NameLast", 52}, {"Gold", 6}, {"Exp/Lvl", 8}, {"HP", 9}, {"HP/Lvl", 10}, {"Defense", 11}, {"Def/Lvl", 12}, {"Str", 13}, {"Str/Lvl", 14}, {"Dex", 15}, {"Dex/Lvl", 16},
            {"AR", 17}, {"AR/Lvl", 18}, {"Dmg-Min", 19}, {"Dmg-Max", 20}, {"Dmg/Lvl", 21}, {"ResistFire", 22}, {"ResistFire/Lvl", 23}, {"ResistCold", 24}, {"ResistCold/Lvl", 25},
            {"ResistLightning", 26}, {"ResistLightning/Lvl", 27}, {"ResistPoison", 28}, {"ResistPoison/Lvl", 29}, {"HireDesc", 50, false, false, nullptr, &stringResource}, {"DefaultChance", 30},
            {"Skill1", 32, false, false, nullptr, &skillName}, {"Level1", 38}, {"LvlPerLvl1", 44}, {"Skill2", 33, false, false, nullptr, &skillName}, {"Level2", 39}, {"LvlPerLvl2", 45},
            {"Skill3", 34, false, false, nullptr, &skillName}, {"Level3", 40}, {"LvlPerLvl3", 46}, {"Skill4", 35, false, false, nullptr, &skillName}, {"Level4", 41}, {"LvlPerLvl4", 47},
            {"Skill5", 36, false, false, nullptr, &skillName}, {"Level5", 42}, {"LvlPerLvl5", 48}, {"Skill6", 37, false, false, nullptr, &skillName}, {"Level6", 43}, {"LvlPerLvl6", 49},
            {"equivalentcharclass", 31, false, false, nullptr, &classCode}
            
        };
        static D2RFormatDescriptor desc("uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuux16uuuuuuux54bbbbbbbbbbbbwa48a48x4", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readItemsCodeBin(const std::string& bin, const D2RModReaderHelper& /*parent*/)
    {
        if (bin.empty())
        {
            return std::string();
        }

        std::stringstream ss;
        ss << "code\n";

        auto data = bin.c_str();
        auto length = bin.length();
        std::stack<std::variant<std::uint8_t, std::uint16_t, std::uint32_t, std::string>> stack;
        std::variant<std::uint8_t, std::uint16_t, std::uint32_t, std::string> value;
        std::uint8_t operatorCode = 0;
        while (length > 0)
        {
            operatorCode = data[0];
            ++data;
            --length;
            switch (operatorCode)
            {
            case 0x04: // byte
                if (length < 1)
                {
                    return std::string();
                }
                {
                    std::stringstream ss2;
                    ss2 << "skill_calc_" << *(std::uint8_t*)(&data[0]);
                    ++data;
                    --length;
                    stack.push(ss2.str());
                }
                break;

            case 0x07:  // byte
                if (length < 1)
                {
                    return std::string();
                }
                stack.push(*(std::uint8_t*)(&data[0]));
                ++data;
                --length;
                break;

            case 0x08: // short
                if (length < 2)
                {
                    return std::string();
                }
                stack.push(*(std::uint16_t*)(&data[0]));
                data += 2;
                length -= 2;
                break;

            case 0x09: // int
                if (length < 4)
                {
                    return std::string();
                }
                stack.push(*(std::uint32_t*)(&data[0]));
                data += 4;
                length -= 4;
                break;

            case 0x00:
                if (stack.size() != 1)
                {
                    return std::string();
                }
                ss << processExpressionStack(stack) << "\n";
                if (length == 1 && data[0] == 0)
                {
                    // we are at the end
                    length = 0;
                }
                break;

            case 0x01:
                if (length < 1)
                {
                    return std::string();
                }

                operatorCode = data[0];
                ++data;
                --length;

                if (stack.size() < 2)
                {
                    return std::string();
                }

                {
                    std::stringstream ss2;
                    switch (operatorCode)
                    {
                    case 0x00:
                        ss2 << "min(" << processExpressionStack(stack) << "," << processExpressionStack(stack) << ")";
                        stack.push(ss2.str());
                        break;

                    case 0x01:
                        ss2 << "max(" << processExpressionStack(stack) << "," << processExpressionStack(stack) << ")";
                        stack.push(ss2.str());
                        break;

                    case 0x02:
                        ss2 << "rand(" << processExpressionStack(stack) << "," << processExpressionStack(stack) << ")";
                        stack.push(ss2.str());
                        break;

                    case 0x03:
                        ss2 << "stat(" << processExpressionStack(stack) << "." << processExpressionStack(stack) << ")";
                        stack.push(ss2.str());
                        break;

                    default:
                        return std::string();
                    }
                }
                break;
            }
        }

        return ss.str();
    }

    std::string D2RBinReader::readItemStatCostBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStatDefaultName(); // needed for usage below
            pParent->processStrings();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> statNameFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::generateStatName);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> stringResourceFiltered = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processStringIdFiltered);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"Stat", 0, false, true, nullptr, &statNameFunc }, {"*ID", 0 }, {"CSvBits", 1, true }, {"Encode", 7, true },
            {"1.09-Save Bits", 3, true }, {"1.09-Save Add", 5, true }, {"Save Bits", 2, true }, {"Save Add", 4, true }, {"Save Param Bits", 6, true }, {"op", 20, true }, {"op param", 21, true },
            {"op base", 22, false, false, nullptr, &statNameFunc }, {"op stat1", 23, false, false, nullptr, &statNameFunc }, {"op stat2", 24, false, false, nullptr, &statNameFunc },
            {"op stat3", 25, false, false, nullptr, &statNameFunc }, {"descpriority", 8, true }, {"descfunc", 9, true }, {"descval", 10, true }, {"descstrpos", 11, true, false, nullptr, &stringResourceFiltered},
            {"descstrneg", 12, true, false, nullptr, &stringResourceFiltered}, {"descstr2", 13, true, false, nullptr, &stringResourceFiltered}, {"dgrp", 14, true }, {"dgrpfunc", 15, true }, {"dgrpval", 16, true },
            {"dgrpstrpos", 17, true, false, nullptr, &stringResourceFiltered}, {"dgrpstrneg", 18, true, false, nullptr, &stringResourceFiltered}, {"dgrpstr2", 19, true, false, nullptr, &stringResourceFiltered},
        };
        static D2RFormatDescriptor desc("ux6bx10bbx1iiux8bx3wbbwwwwbbwwwx12bbwwwwx234", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readItemTypesBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty() || !initializeItemTypesBin(bin, parent))
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processBodyLocs();
            pParent->processItemType(); // needed for usage below
            pParent->processItemTypeName();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> bodyLocsFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processBodyLocs);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> typeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemType);
        static std::function<std::string(std::string, const D2RModReaderHelper&, size_t)> typeNameFunc = std::function<std::string(std::string, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemTypeName);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"ItemType", 0, false, true, nullptr, nullptr, &typeNameFunc }, {"Code", 0},
            {"Equiv1", 1, true, false, nullptr, &typeFunc}, {"Equiv2", 2, true, false, nullptr, &typeFunc}, {"Body", 3},
            {"BodyLoc1", 4, true, false, nullptr, &bodyLocsFunc}, {"BodyLoc2", 5, true, false, nullptr, &bodyLocsFunc}, {"Shoots", 6, true, false, nullptr, &typeFunc},
            {"Beltable", 7}, {"MaxSockets1", 8}, {"MaxSocketsLevelThreshold1", 11}, {"MaxSockets2", 9}, {"MaxSocketsLevelThreshold2", 12}, {"MaxSockets3", 10}
        };
        static D2RFormatDescriptor desc("a4wwx1bbbwx9bbbbbbx199", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readMagicAffixBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processItemType(); // needed for usage below
            pParent->processPropertyCode();
            pParent->processPlayerClass();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> typeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemType);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> propertyFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPropertyCode);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> colorCodeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processColor);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> classCode = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPlayerClassCode);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"name", 0}, {"version", 1}, {"spawnable", 14, true}, {"rare", 19}, {"level", 16, true}, {"maxlevel", 18, true}, {"levelreq", 20, true}, 
            {"classspecific", 21, false, false, nullptr, &classCode}, {"class", 22, false, false, nullptr, &classCode}, {"classlevelreq", 23, true}, {"frequency", 36}, {"group", 17, true}, 
            {"mod1code", 2, false, false, nullptr, &propertyFunc}, {"mod1param", 3, true}, {"mod1min", 4, true}, {"mod1max", 5, true}, {"mod2code", 6, false, false, nullptr, &propertyFunc}, {"mod2param", 7, true},
            {"mod2min", 8, true}, {"mod2max", 9, true}, {"mod3code", 10, false, false, nullptr, &propertyFunc}, {"mod3param", 11, true}, {"mod4min", 12, true}, {"mod4max", 13, true},
            {"transformcolor", 15, false, false, nullptr, &colorCodeFunc}, {"itype1", 24, true, false, nullptr, &typeFunc}, {"itype2", 25, true, false, nullptr, &typeFunc},
            {"itype3", 26, true, false, nullptr, &typeFunc}, {"itype4", 27, true, false, nullptr, &typeFunc}, {"itype5", 28, true, false, nullptr, &typeFunc}, {"itype6", 29, true, false, nullptr, &typeFunc},
            {"itype7", 30, true, false, nullptr, &typeFunc}, {"etype1", 31, true, false, nullptr, &typeFunc}, {"etype2", 32, true, false, nullptr, &typeFunc}, {"etype3", 33, true, false, nullptr, &typeFunc}, 
            {"etype4", 34, true, false, nullptr, &typeFunc}, {"etype5", 35, true, false, nullptr, &typeFunc}, {"multiply", 37}, {"add", 38},
        };
        static D2RFormatDescriptor desc("a32x2wuiiiuiiiuiiiwwwx2uubbbbwwwwwwwwwwwwwwuu", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readMiscBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStrings(); // needed for usage below
            pParent->processItemType(); 
            pParent->processStatName();
            pParent->processCalcExpression();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> typeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemType);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> statNameFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processStatName);
        static std::function<std::string(std::string, const D2RModReaderHelper&, size_t)> itemNameFunc = std::function<std::string(std::string, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemName);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> stringResourceFiltered = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processStringIdFiltered);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> calcExpression = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processCalcExpression);
        static std::function<bool(const std::string&, size_t)> isExpansionRow = std::function<bool(const std::string&, size_t)>(D2RBinReader::isExpansionVersion);

        size_t versionTestCol = 12;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"name", 4, false, true, nullptr, nullptr, &itemNameFunc}, {"compactsave", 32}, {"version", versionTestCol}, {"level", 13}, {"levelreq", 30},
            {"reqstr", 16}, {"reqdex", 17}, {"durability", 20}, {"nodurability", 21},  {"code", 4}, {"alternategfx", 5}, {"namestr", 4, false, true},  {"invwidth", 18}, {"invheight", 19}, {"gemsockets", 28},
            {"gemapplytype", 29}, {"flippyfile", 0}, {"invfile", 1}, {"uniqueinvfile", 2}, {"setinvfile", 3}, {"type", 22, true, false, nullptr, &typeFunc}, {"type2", 23, true, false, nullptr, &typeFunc}, {"unique", 24},
            {"belt", 26}, {"stackable", 27}, {"minstack", 10}, {"maxstack", 11}, {"quest", 25, true}, {"stat1", 6, true, false, nullptr, &statNameFunc}, {"spelldesc", 7, true},
            {"spelldescstr", 8, true, false, nullptr, &stringResourceFiltered}, {"spelldesccalc", 9, true, false, nullptr, &calcExpression}, {"mindam", 14, true}, {"maxdam", 15, true}, {"InvTrans", 31},
            {"SkipName", 33, true}, {"Nameable", 34, true}
        };
        //                                                                 11   1  111   1111  22   22  22  2
        //                               0  1  2  3  4    5    6   78  9   01   2  345   6789  01   23  45  6
        static D2RFormatDescriptor desc("a32a32a32a32a4x12a4x10wx20wwx4ux44uux10bx6bbbx10wwbbx1bbx11bbx9bbx5bx1bx5bx5bbx2bbbbx102", format, versionTestCol);
        return processBin(data, length, desc, parent, &isExpansionRow);
    }

    std::string D2RBinReader::readMonStatsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStrings(); // needed for usage below
            pParent->processMonType();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> stringResource = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processStringId);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> monTypeNameFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processMonType);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> monTypeEnabledFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processMonStatsEnabledFlag);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> monTypeKillableFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processMonStatsKillableFlag);
        

        size_t versionTestCol = 0;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"Id", 1}, {"*hcIdx", 0}, {"NameStr", 2, false, false, nullptr, &stringResource},
            {"MonType", 5, false, false, nullptr, &monTypeNameFunc}, {"enabled", 4, false, true, nullptr, &monTypeEnabledFunc}, 
            {"killable", 3, false, true, nullptr, &monTypeKillableFunc}
        };
        static D2RFormatDescriptor desc("wa48x4wx5bx1bx12wx418", format, versionTestCol);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readMonTypeBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processMonTypeDefaultName(); // needed for usage below
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> monTypeNameFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::generateMonType);

        size_t versionTestCol = 0;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"type", 0, false, true, nullptr, &monTypeNameFunc}
        };
        static D2RFormatDescriptor desc("x2wwwww", format, versionTestCol);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readPlayerClassBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        static std::function<bool(const std::string&, size_t)> isExpansionRow = std::function<bool(const std::string&, size_t)>(D2RBinReader::isExpansionPlayerClass);

        size_t versionTestCol = 0;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"Player Class", versionTestCol}, {"Code", 1}
        };
        static D2RFormatDescriptor desc("a48a4", format, versionTestCol);
        return processBin(data, length, desc, parent, &isExpansionRow);
    }

    std::string D2RBinReader::readPropertiesBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processPropertyCodeName(); // needed for usage below
            pParent->processStatName();
        }

        static std::function<bool(const std::string&, size_t)> isExpansionRow = std::function<bool(const std::string&, size_t)>(D2RBinReader::isExpansionPropertyRow); static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> typeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemType);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> statNameFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processStatName);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> propertyFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::generatePropertyCode);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> propertyEnabledFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPropertyEnabled);

        size_t versionTestCol = 0;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"code", versionTestCol, false, false, nullptr, &propertyFunc}, {"*Enabled", 15, true, true, nullptr, &propertyEnabledFunc},
            {"func1", 15, true}, {"stat1", 22, true, false, nullptr, &statNameFunc}, {"set1", 1, true}, {"val1", 8, true},
            {"func2", 16, true}, {"stat2", 23, true, false, nullptr, &statNameFunc}, {"set2", 2, true}, {"val2", 9, true},
            {"func3", 17, true}, {"stat3", 24, true, false, nullptr, &statNameFunc}, {"set3", 3, true}, {"val3", 10, true},
            {"func4", 18, true}, {"stat4", 25, true, false, nullptr, &statNameFunc}, {"set4", 4, true}, {"val4", 11, true},
            {"func5", 19, true}, {"stat5", 26, true, false, nullptr, &statNameFunc}, {"set5", 5, true}, {"val5", 12, true},
            {"func6", 20, true}, {"stat6", 27, true, false, nullptr, &statNameFunc}, {"set6", 6, true}, {"val6", 13, true},
            {"func7", 21, true}, {"stat7", 28, true, false, nullptr, &statNameFunc}, {"set7", 7, true}, {"val7", 14, true}
        };
        static D2RFormatDescriptor desc("wbbbbbbbx1wwwwwwwbbbbbbbx1wwwwwww", format, versionTestCol);
        return processBin(data, length, desc, parent, &isExpansionRow);
    }

    std::string D2RBinReader::readQualityItemsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processPropertyCode(); // needed for usage below
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> propertyFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPropertyCode);

        size_t versionTestCol = 0;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"mod1code", 10, false, false, nullptr, &propertyFunc}, {"mod1param", 11}, {"mod1min", 12}, {"mod1max", 13},
            {"mod2code", 14, false, false, nullptr, &propertyFunc}, {"mod2param", 15}, {"mod2min", 16, true}, {"mod2max", 17, true}, {"armor", 0}, {"armor", 0}, {"weapon", 1},
            {"shield", 2}, {"scepter", 3}, {"wand", 4}, {"staff", 5}, {"bow", 6}, {"boots", 7}, {"gloves", 8}, {"belt", 9},
        };
        static D2RFormatDescriptor desc("bbbbbbbbbbx2uuuuuuuu", format, versionTestCol);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readRareAffixBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processItemType(); // needed for usage below
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> typeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemType);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"name", 12}, {"version", 0}, {"itype1", 1, true, false, nullptr, &typeFunc}, {"itype2", 2, true, false, nullptr, &typeFunc},
            {"itype3", 3, true, false, nullptr, &typeFunc}, {"itype4", 4, true, false, nullptr, &typeFunc}, {"itype5", 5, true, false, nullptr, &typeFunc}, {"itype6", 6, true, false, nullptr, &typeFunc},
            {"itype7", 7, true, false, nullptr, &typeFunc}, {"etype1", 8, true, false, nullptr, &typeFunc}, {"etype2", 9, true, false, nullptr, &typeFunc}, {"etype3", 10, true, false, nullptr, &typeFunc},
            {"etype4", 11, true, false, nullptr, &typeFunc},

        };
        static D2RFormatDescriptor desc("x14wwwwwwwwwwwwa32x2", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readRunesBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStrings(); // needed for usage below
            pParent->processItemType();
            pParent->processPropertyCode();
            pParent->processItemCode();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> typeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemType);
        static std::function<std::string(std::string, const D2RModReaderHelper&, size_t)> runeNameFunc = std::function<std::string(std::string, const D2RModReaderHelper&, size_t)>(D2RBinReader::processRuneName);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> propertyFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPropertyCode);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> codeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemCode);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"name", 0}, {"*Rune Name", 0, false, true, nullptr, nullptr, &runeNameFunc}, {"complete", 3, true}, {"firstLadderSeason", 1, true},
            {"lastLadderSeason", 2, true}, {"itype1", 4, true, false, nullptr, &typeFunc}, {"itype2", 5, true, false, nullptr, &typeFunc},  {"itype3", 6, true, false, nullptr, &typeFunc},
            {"itype4", 7, true, false, nullptr, &typeFunc}, {"itype5", 8, true, false, nullptr, &typeFunc}, {"itype6", 9, true, false, nullptr, &typeFunc}, {"etype1", 10, true, false, nullptr, &typeFunc},
            {"etype2", 11, true, false, nullptr, &typeFunc}, {"etype3", 12, true, false, nullptr, &typeFunc}, {"Rune1", 13, false, false, nullptr, &codeFunc}, {"Rune2", 14, false, false, nullptr, &codeFunc},
            {"Rune3", 15, false, false, nullptr, &codeFunc}, {"Rune4", 16, false, false, nullptr, &codeFunc}, {"Rune5", 17, false, false, nullptr, &codeFunc}, {"Rune6", 18, false, false, nullptr, &codeFunc},
            {"T1Code1", 19, true, false, nullptr, &propertyFunc}, {"T1Param1", 20, true}, {"T1Min1", 21, true}, {"T1Max1", 22, true},
            {"T1Code2", 23, true, false, nullptr, &propertyFunc}, {"T1Param2", 24, true}, {"T1Min2", 25, true}, {"T1Max2", 26, true},
            {"T1Code3", 27, true, false, nullptr, &propertyFunc}, {"T1Param3", 28, true}, {"T1Min3", 29, true}, {"T1Max3", 30, true},
            {"T1Code4", 31, true, false, nullptr, &propertyFunc}, {"T1Param4", 32, true}, {"T1Min4", 33, true}, {"T1Max4", 34, true},
            {"T1Code5", 35, true, false, nullptr, &propertyFunc}, {"T1Param5", 36, true}, {"T1Min5", 37, true}, {"T1Max5", 38, true},
            {"T1Code6", 39, true, false, nullptr, &propertyFunc}, {"T1Param6", 40, true}, {"T1Min6", 41, true}, {"T1Max6", 42, true},
            {"T1Code7", 43, true, false, nullptr, &propertyFunc}, {"T1Param7", 44, true}, {"T1Min7", 45, true}, {"T1Max7", 46, true},

        };
        static D2RFormatDescriptor desc("a64wwbx5wwwwwwwwwuuuuuuuiiiuiiiuiiiuiiiuiiiuiiiuiii", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readSetItemsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processSetIndex(); // needed for usage below
            pParent->processPropertyCode();
        }

        static std::function<std::string(size_t)> generateID = std::function<std::string(size_t)>(D2RBinReader::generateID);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> setIndexFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processSetIndex);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> colorCodeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processColor);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> propertyFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPropertyCode);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"index", 0}, {"*ID", MAXSIZE_T, false, true, nullptr, nullptr, nullptr, &generateID },  {"set", 2, false, false, nullptr, &setIndexFunc},  {"item", 1}, {"lvl", 3}, {"lvl req", 4}, 
            {"invtransform", 5, false, false, nullptr, &colorCodeFunc}, {"invfile", 6}, {"add func", 7},
            {"prop1", 8, true, false, nullptr, &propertyFunc}, {"par1", 9, true}, {"min1", 10, true}, {"max1", 11, true},
            {"prop2", 12, true, false, nullptr, &propertyFunc}, {"par2", 13, true}, {"min2", 14, true}, {"max2", 15, true},
            {"prop3", 16, true, false, nullptr, &propertyFunc}, {"par3", 17, true}, {"min3", 18, true}, {"max3", 19, true},
            {"prop4", 20, true, false, nullptr, &propertyFunc}, {"par4", 21, true}, {"min4", 22, true}, {"max4", 23, true},
            {"prop5", 24, true, false, nullptr, &propertyFunc}, {"par5", 25, true}, {"min5", 26, true}, {"max5", 27, true},
            {"prop6", 28, true, false, nullptr, &propertyFunc}, {"par6", 29, true}, {"min6", 30, true}, {"max6", 31, true},
            {"prop7", 32, true, false, nullptr, &propertyFunc}, {"par7", 33, true}, {"min7", 34, true}, {"max7", 35, true},
            {"prop8", 36, true, false, nullptr, &propertyFunc}, {"par8", 37, true}, {"min8", 38, true}, {"max8", 39, true},
            {"prop9", 40, true, false, nullptr, &propertyFunc}, {"par9", 41, true}, {"min9", 42, true}, {"max9", 43, true},
            {"aprop1a", 44, true, false, nullptr, &propertyFunc}, {"apar1a", 45, true}, {"amin1a", 46, true}, {"amax1a", 47, true}, {"aprop1b", 48, true, false, nullptr, &propertyFunc}, {"apar1b", 49, true}, {"amin1b", 50, true}, {"amax1b", 51, true},
            {"aprop2a", 52, true, false, nullptr, &propertyFunc}, {"apar2a", 53, true}, {"amin2a", 54, true}, {"amax2a", 55, true}, {"aprop2b", 56, true, false, nullptr, &propertyFunc}, {"apar2b", 57, true}, {"amin2b", 58, true}, {"amax2b", 59, true},
            {"aprop3a", 60, true, false, nullptr, &propertyFunc}, {"apar3a", 61, true}, {"amin3a", 62, true}, {"amax3a", 63, true}, {"aprop3b", 64, true, false, nullptr, &propertyFunc}, {"apar3b", 65, true}, {"amin3b", 66, true}, {"amax3b", 67, true},
            {"aprop4a", 68, true, false, nullptr, &propertyFunc}, {"apar4a", 69, true}, {"amin4a", 70, true}, {"amax4a", 71, true}, {"aprop4b", 72, true, false, nullptr, &propertyFunc}, {"apar4b", 73, true}, {"amin4b", 74, true}, {"amax4b", 75, true},
            {"aprop5a", 76, true, false, nullptr, &propertyFunc}, {"apar5a", 77, true}, {"amin5a", 78, true}, {"amax5a", 79, true}, {"aprop5b", 80, true, false, nullptr, &propertyFunc}, {"apar5b", 81, true}, {"amin5b", 82, true}, {"amax5b", 83, true},
        };
        static D2RFormatDescriptor desc("x2a32x6a4uwwx13bx32a32x5buiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiix4", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readSetsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStrings(); // needed for usage below
            pParent->processPropertyCode();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> setIndexFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::generateSetIndex);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> propertyFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPropertyCode);
        static std::function<bool(const std::string&, size_t)> isExpansionRow = std::function<bool(const std::string&, size_t)>(D2RBinReader::isExpansionVersion);

        size_t versionTestCol = 1;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"index", 0, false, false, nullptr, &setIndexFunc},  {"name", 0, false, true}, {"version", versionTestCol},
            {"PCode2a", 2, true, false, nullptr, &propertyFunc}, {"PParam2a", 3, true}, {"PMin2a", 4, true}, {"PMax2a", 5, true}, {"PCode2b", 6, true, false, nullptr, &propertyFunc}, {"PParam2b", 7, true}, {"PMin2b", 8, true}, {"PMax2b", 9, true},
            {"PCode3a", 10, true, false, nullptr, &propertyFunc}, {"PParam3a", 11, true}, {"PMin3a", 12, true}, {"PMax3a", 13, true}, {"PCode3b", 14, true, false, nullptr, &propertyFunc}, {"PParam3b", 15, true}, {"PMin3b", 16, true}, {"PMax3b", 17, true},
            {"PCode4a", 18, true, false, nullptr, &propertyFunc}, {"PParam4a", 19, true}, {"PMin4a", 20, true}, {"PMax4a", 21, true}, {"PCode4b", 22, true, false, nullptr, &propertyFunc}, {"PParam4b", 23, true}, {"PMin4b", 24, true}, {"PMax4b", 25, true},
            {"PCode5a", 26, true, false, nullptr, &propertyFunc}, {"PParam5a", 27, true}, {"PMin5a", 28, true}, {"PMax5a", 29, true}, {"PCode5b", 30, true, false, nullptr, &propertyFunc}, {"PParam5b", 31, true}, {"PMin5b", 32, true}, {"PMax5b", 33, true},
            {"FCode1", 34, true, false, nullptr, &propertyFunc}, {"FParam1", 35}, {"FMin1", 36, true}, {"FMax1", 37, true},
            {"FCode2", 38, true, false, nullptr, &propertyFunc}, {"FParam2", 39}, {"FMin2", 40, true}, {"FMax2", 41, true},
            {"FCode3", 42, true, false, nullptr, &propertyFunc}, {"FParam3", 43}, {"FMin3", 44, true}, {"FMax3", 45, true},
            {"FCode4", 46, true, false, nullptr, &propertyFunc}, {"FParam4", 47}, {"FMin4", 48, true}, {"FMax4", 49, true},
            {"FCode5", 50, true, false, nullptr, &propertyFunc}, {"FParam5", 51}, {"FMin5", 52, true}, {"FMax5", 53, true},
            {"FCode6", 54, true, false, nullptr, &propertyFunc}, {"FParam6", 55}, {"FMin6", 56, true}, {"FMax6", 57, true},
            {"FCode7", 58, true, false, nullptr, &propertyFunc}, {"FParam7", 59}, {"FMin7", 60, true}, {"FMax7", 61, true},
            {"FCode8", 62, true, false, nullptr, &propertyFunc}, {"FParam8", 63}, {"FMin8", 64, true}, {"FMax8", 65, true},
        };
        static D2RFormatDescriptor desc("x2wux8uiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiix24", format, versionTestCol);
        return processBin(data, length, desc, parent, &isExpansionRow);
    }

    std::string D2RBinReader::readSkillDescBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStrings(); // needed for usage below
            pParent->processSkillDescDefaultName();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> skillDescName = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::generateSkillDesc);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> stringResourceFiltered = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processStringIdFiltered);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"skilldesc", 0, false, true, nullptr, &skillDescName}, {"SkillPage", 1}, {"SkillRow", 2}, {"SkillColumn", 3}, {"ListRow", 4}, {"IconCel", 5},
            {"str name", 6, false, false, nullptr, &stringResourceFiltered}, {"str short", 7, false, false, nullptr, &stringResourceFiltered}, {"str long", 8, false, false, nullptr, &stringResourceFiltered},
        };
        static D2RFormatDescriptor desc("wbbbcbx1wwwx286", format);
        return processBin(data, length, desc, parent);
    }

    bool D2RBinReader::initializeSkillsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty() || !parent.skillsMap.empty())
        {
            return true;
        }

        auto data = bin.c_str();
        auto length = bin.length(); static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"skill", 1}, {"*Id", 0}
        };
        static D2RFormatDescriptor desc("wa34x632", format);
        std::string result = processBin(data, length, desc, parent);
        if (result.empty())
        {
            return false;
        }

        // we need to map id to excel type column string
        auto pDoc = parent.LoadTxtStream(result);
        std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
        const SSIZE_T skillColumnIdx = pDoc->GetColumnIdx("skill");
        if (skillColumnIdx < 0)
        {
            return false;
        }

        SSIZE_T idColumnIdx = pDoc->GetColumnIdx("*Id");
        if (idColumnIdx < 0)
        {
            return false;
        }

        auto pParent = const_cast<D2RModReaderHelper*>(&parent);
        std::string strValue;
        std::uint16_t id = MAXUINT16;
        for (std::uint32_t i = 0; i < numRows; ++i)
        {
            strValue = pDoc->GetCellString(idColumnIdx, i);
            if (strValue.empty())
            {
                // skip
                continue;
            }

            id = pDoc->GetCellUInt16(idColumnIdx, i);
            strValue = pDoc->GetCellString(skillColumnIdx, i);
            if (strValue.empty())
            {
                // skip
                continue;
            }
            pParent->skillsMap[id] = strValue;
        }

        return true;
    }

    std::string D2RBinReader::readSkillsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty() || !initializeSkillsBin(bin, parent))
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processPlayerClass(); // needed for usage below
            pParent->processSkillDesc();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> classCode = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPlayerClassCode);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> skillName = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processSkillName);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> skillDescName = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processSkillDesc);

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"skill", 1}, {"*Id", 0}, {"charclass", 2, false, false, nullptr, &classCode}, {"skilldesc", 7, false, false, nullptr, &skillDescName}, {"reqlevel", 3},
            {"reqskill1", 4, false, false, nullptr, &skillName}, {"reqskill2", 5, false, false, nullptr, &skillName}, {"reqskill3", 6, false, false, nullptr, &skillName},
        };
        static D2RFormatDescriptor desc("wa34x8bx407wx8wwwx24wx174", format);
        return processBin(data, length, desc, parent);
    }

    std::string D2RBinReader::readUniqueItemsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processPropertyCode();
        }

        static std::function<std::string(size_t)> generateID = std::function<std::string(size_t)>(D2RBinReader::generateID);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> colorCodeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processColor);
        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> propertyFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processPropertyCode);
        static std::function<bool(const std::string&, size_t)> isExpansionRow = std::function<bool(const std::string&, size_t)>(D2RBinReader::isExpansionVersion);

        size_t versionTestCol = 1;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"index", 0}, {"*ID", MAXSIZE_T, false, true, nullptr, nullptr, nullptr, &generateID}, {"version", versionTestCol}, {"lvl", 3}, {"lvl req", 4}, {"code", 2}, {"invtransform", 5, false, false, nullptr, &colorCodeFunc}, {"invfile", 6},
            {"prop1", 7, true, false, nullptr, &propertyFunc}, {"par1", 8, true}, {"min1", 9, true}, {"max1", 10, true},
            {"prop2", 11, true, false, nullptr, &propertyFunc}, {"par2", 12, true}, {"min2", 13, true}, {"max2", 14, true},
            {"prop3", 15, true, false, nullptr, &propertyFunc}, {"par3", 16, true}, {"min3", 17, true}, {"max3", 18, true},
            {"prop4", 19, true, false, nullptr, &propertyFunc}, {"par4", 20, true}, {"min4", 21, true}, {"max4", 22, true},
            {"prop5", 23, true, false, nullptr, &propertyFunc}, {"par5", 24, true}, {"min5", 25, true}, {"max5", 26, true},
            {"prop6", 27, true, false, nullptr, &propertyFunc}, {"par6", 28, true}, {"min6", 29, true}, {"max6", 30, true},
            {"prop7", 31, true, false, nullptr, &propertyFunc}, {"par7", 32, true}, {"min7", 33, true}, {"max7", 34, true},
            {"prop8", 35, true, false, nullptr, &propertyFunc}, {"par8", 36, true}, {"min8", 37, true}, {"max8", 38, true},
            {"prop9", 39, true, false, nullptr, &propertyFunc}, {"par9", 40, true}, {"min9", 41, true}, {"max9", 42, true},
            {"prop10", 43, true, false, nullptr, &propertyFunc}, {"par10", 44, true}, {"min10", 45, true}, {"max10", 46, true},
            {"prop11", 47, true, false, nullptr, &propertyFunc}, {"par11", 48, true}, {"min11", 49, true}, {"max11", 50, true},
            {"prop12", 51, true, false, nullptr, &propertyFunc}, {"par12", 52, true}, {"min12", 53, true}, {"max12", 54, true}
        };
        static D2RFormatDescriptor desc("x2a32x2ua4x8wwx1bx32a32x22uiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiiuiiix4", format, versionTestCol);
        return processBin(data, length, desc, parent, &isExpansionRow);
    }

    std::string D2RBinReader::readWeaponsBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty())
        {
            return std::string();
        }

        {
            auto pParent = const_cast<D2RModReaderHelper*>(&parent);
            pParent->processStrings(); // needed for usage below
            pParent->processItemType();
        }

        static std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)> typeFunc = std::function<std::string(std::uint32_t, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemType);
        static std::function<std::string(std::string, const D2RModReaderHelper&, size_t)> itemNameFunc = std::function<std::string(std::string, const D2RModReaderHelper&, size_t)>(D2RBinReader::processItemName);
        static std::function<bool(const std::string&, size_t)> isExpansionRow = std::function<bool(const std::string&, size_t)>(D2RBinReader::isExpansionVersion);

        size_t versionTestCol = 11;
        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"name", 4, false, true, nullptr, nullptr, &itemNameFunc}, {"type", 27, true, false, nullptr, &typeFunc}, {"type2", 28, true, false, nullptr, &typeFunc},
            {"code", 4}, {"alternategfx", 8}, {"namestr", 4, false, true}, {"version", versionTestCol}, {"compactsave", 39}, {"mindam", 14, true}, {"maxdam", 15, true},  {"1or2handed", 34, true},
            {"2handed", 26, true}, {"2handmindam", 18, true}, {"2handmaxdam", 19, true}, {"minmisdam", 16, true}, {"maxmisdam", 17, true}, {"reqstr", 20, true}, {"reqdex", 21, true},
            {"durability", 24}, {"nodurability", 25, true}, {"level", 13}, {"levelreq", 36}, {"magic lvl", 37, true}, {"auto prefix", 12, true}, {"normcode", 5}, {"ubercode", 6}, {"ultracode", 7},
            {"invwidth", 22}, {"invheight", 23}, {"stackable", 32, true}, {"minstack", 9, true}, {"maxstack", 10, true}, {"flippyfile", 0}, {"invfile", 1}, {"uniqueinvfile", 2}, {"setinvfile", 3},
            {"gemsockets", 33, true}, {"gemapplytype", 35}, {"unique", 29}, {"belt", 31}, {"quest", 30, true}, {"InvTrans", 38},  {"SkipName", 40}, {"Nameable", 41}
        };
        static D2RFormatDescriptor desc("a32a32a32a32a4a4a4a4a4x88uux10bx1wx3bx1bbbbbbx5wwbbx1bbx8bx2bbx9bwx4wbx5bx4bbbbx1bbbbx102", format, versionTestCol);
        return processBin(data, length, desc, parent, &isExpansionRow);
    }

    bool D2RBinReader::initializeItemTypesBin(const std::string& bin, const D2RModReaderHelper& parent)
    {
        if (bin.empty() || !parent.itemTypeMap.empty())
        {
            return true;
        }

        auto data = bin.c_str();
        auto length = bin.length();
        static std::vector<D2RFormatDescriptor::D2RExcelFormat> format = { {"Code", 0 } };
        static D2RFormatDescriptor desc("a4x224", format);
        std::string result = processBin(data, length, desc, parent);
        if (result.empty())
        {
            return false;
        }

        // we need to map id to excel type column string
        auto pDoc = parent.LoadTxtStream(result);
        std::uint32_t numRows = std::uint32_t(pDoc->GetRowCount());
        const SSIZE_T codeColumnIdx = pDoc->GetColumnIdx("Code");
        if (codeColumnIdx < 0)
        {
            return false;
        }

        auto pParent = const_cast<D2RModReaderHelper*>(&parent);
        for (std::uint32_t i = 0; i < numRows; ++i)
        {
            pParent->itemTypeMap[i] = pDoc->GetCellString(codeColumnIdx, i);
        }

        return true;
    }

    std::string D2RBinReader::generateStatName(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        const auto& itemDefaultStatMap = parent.itemDefaultStatMap;
        auto iter = itemDefaultStatMap.find(code);
        if (iter == itemDefaultStatMap.end())
        {
            if (code == MAXUINT16)
            {
               return std::string();
            }

            std::stringstream ss;
            ss << "stat_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processStatName(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        const auto& itemStatMap = parent.itemStatMap;
        auto iter = itemStatMap.find(code);
        if (iter == itemStatMap.end())
        {
            if (code == MAXUINT16)
            {
               return std::string();
            }

            std::stringstream ss;
            ss << "stat_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processItemType(std::uint32_t type, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        const auto& itemTypeMap = parent.itemTypeMap;
        auto iter = itemTypeMap.find(type);
        if (iter == itemTypeMap.end())
        {
            std::stringstream ss;
            ss << "item_";
            ss << type;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processItemCode(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        if (code == MAXUINT32)
        {
            return std::string();
        }

        const auto& itemCodeMap = parent.itemCodeMap;
        auto iter = itemCodeMap.find(code);
        if (iter == itemCodeMap.end())
        {
            return std::to_string(code);
        }

        return iter->second;
    }

    std::string D2RBinReader::generatePropertyCode(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        if (code == MAXUINT32)
        {
            return std::string();
        }

        const auto& propertyCodeDefaultMap = parent.propertyCodeDefaultMap;
        auto iter = propertyCodeDefaultMap.find(code);
        if (iter == propertyCodeDefaultMap.end())
        {
            std::stringstream ss;
            ss << "prop_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processPropertyEnabled(std::uint32_t code, const D2RModReaderHelper& /*parent*/, size_t /*lineNum*/)
    {
        if (code >= MAXUINT16 || code == 0)
        {
            return std::string();
        }

        return "1";
    }

    std::string D2RBinReader::processPropertyCode(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        if (code == MAXUINT32)
        {
            return std::string();
        }

        const auto& propertyCodeMap = parent.propertyCodeMap;
        auto iter = propertyCodeMap.find(code);
        if (iter == propertyCodeMap.end())
        {
            std::stringstream ss;
            ss << "prop_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processBodyLocs(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        const auto& bodyLocsMap = parent.bodyLocsMap;
        auto iter = bodyLocsMap.find(code);
        if (iter == bodyLocsMap.end())
        {
            std::stringstream ss;
            ss << "body_loc_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processStringId(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        const auto& stringTxtInfo = parent.stringTxtInfo;
        auto iter = stringTxtInfo.find(code);
        if (iter == stringTxtInfo.end())
        {
            return std::string();
        }

        return iter->second;
    }

    std::string D2RBinReader::processStringIdFiltered(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum)
    {
        auto result = processStringId(code, parent, lineNum);
        if (result.empty())
        {
            return result;
        }

        if (result == "dummy" || result == "Dummy" || result == "x" || result == "X")
        {
            return std::string();
        }

        return result;
    }

    std::string D2RBinReader::processCalcExpression(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        if (code >= MAXUINT16)
        {
            return std::string();
        }

        const auto& calcExpressionMap = parent.calcExpressionMap;
        auto iter = calcExpressionMap.find(code);
        if (iter == calcExpressionMap.end())
        {
            return std::string();
        }

        return iter->second;
    }

    std::string D2RBinReader::processExperienceLevel(std::uint32_t /*code*/, const D2RModReaderHelper& /*parent*/, size_t lineNum)
    {
        if (lineNum == 0)
        {
            return "MaxLvl";
        }

        return std::to_string(lineNum - 1);
    }

    std::string D2RBinReader::generateSetIndex(std::uint32_t code, const D2RModReaderHelper& parent, size_t lineNum)
    {
        auto strKey = processStringId(code, parent, lineNum);
        strKey.erase(std::find_if(strKey.rbegin(), strKey.rend(), [](int ch) { return !isspace(ch); }).base(), strKey.end());
        if (strKey.empty())
        {
            return std::string();
        }

        const auto& stringEnTxtByKey = parent.stringEnTxtByKey;
        auto iter = stringEnTxtByKey.find(strKey);
        if (iter == stringEnTxtByKey.end())
        {
            return strKey;
        }

        return iter->second;
    }
    
    std::string D2RBinReader::processSetIndex(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        const auto& setsMap = parent.setsMap;
        auto iter = setsMap.find(code);
        if (iter == setsMap.end())
        {
            return std::string();
        }

        return iter->second;
    }

    std::string D2RBinReader::processSkillName(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        if (code >= MAXUINT16)
        {
            return std::string();
        }

        const auto& skillsMap = parent.skillsMap;
        auto iter = skillsMap.find(code);
        if (iter == skillsMap.end())
        {
            std::stringstream ss;
            ss << "skill_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::generateSkillDesc(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        const auto& skillDesDefaultMap = parent.skillDesDefaultMap;
        auto iter = skillDesDefaultMap.find(code);
        if (iter == skillDesDefaultMap.end())
        {
            if (code == MAXUINT16)
            {
                return std::string();
            }

            std::stringstream ss;
            ss << "skill_desc_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processSkillDesc(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        if (code >= MAXUINT16)
        {
            return std::string();
        }

        const auto& skillDescMap = parent.skillDescMap;
        auto iter = skillDescMap.find(code);
        if (iter == skillDescMap.end())
        {
            std::stringstream ss;
            ss << "skill_desc_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processPlayerClassCode(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        if (code == MAXUINT8)
        {
            return std::string();
        }

        const auto& playerClassCodeMap = parent.playerClassCodeMap;
        auto iter = playerClassCodeMap.find(code);
        if (iter == playerClassCodeMap.end())
        {
            return std::string();
        }

        return iter->second;
    }

    std::string D2RBinReader::processHirelingType(std::uint32_t code, const D2RModReaderHelper& /*parent*/, size_t /*lineNum*/)
    {
        const std::map<std::uint32_t, std::string> mercClassMap = { {271, "Rogue Scout"}, {338, "Desert Mercenary"}, {359, "Eastern Sorceror"}, {560, "Barbarian"}, {561, "Barbarian"} };
        if (code == MAXUINT8)
        {
            return std::string();
        }

        auto iter = mercClassMap.find(code);
        if (iter == mercClassMap.end())
        {
            std::stringstream ss;
            ss << "hireling_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::generateMonType(std::uint32_t /*code*/, const D2RModReaderHelper& parent, size_t lineNum)
    {
        const auto& itemDefaultMonTypeMap = parent.itemDefaultMonTypeMap;
        auto iter = itemDefaultMonTypeMap.find(std::uint32_t(lineNum));
        if (iter == itemDefaultMonTypeMap.end())
        {
            if (lineNum == 0)
            {
                return std::string();
            }

            std::stringstream ss;
            ss << "mon_type_";
            ss << lineNum;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processMonType(std::uint32_t code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        const auto& itemMonTypeMap = parent.itemMonTypeMap;
        auto iter = itemMonTypeMap.find(code);
        if (iter == itemMonTypeMap.end())
        {
            if (code == MAXUINT16 || code == 0)
            {
               return std::string();
            }

            std::stringstream ss;
            ss << "mon_type_";
            ss << code;
            return ss.str();
        }

        return iter->second;
    }

    std::string D2RBinReader::processMonStatsEnabledFlag(std::uint32_t code, const D2RModReaderHelper& /*parent*/, size_t /*lineNum*/)
    {
        if ((code & 0x0002) != 0)
        {
            return "1";
        }

        return std::string();
    }

    std::string D2RBinReader::processMonStatsKillableFlag(std::uint32_t code, const D2RModReaderHelper& /*parent*/, size_t /*lineNum*/)
    {
        if ((code & 0x080) != 0)
        {
            return "1";
        }

        return std::string();
    }

    std::string D2RBinReader::generateID(size_t lineNum)
    {
        return std::to_string(lineNum);
    }

    std::string D2RBinReader::processColor(std::uint32_t code, const D2RModReaderHelper& /*parent*/, size_t /*lineNum*/)
    {
        switch(code)
        {
        case 0:
            return "whit";
        case 1:
            return "lgry";
        case 2:
            return "dgry";
        case 3:
            return "blac";
        case 4:
            return "lblu";
        case 5:
            return "dblu";
        case 6:
            return "cblu";
        case 7:
            return "lred";
        case 8:
            return "dred";
        case 9:
            return "cred";
        case 10:
            return "lgrn";
        case 11:
            return "dgrn";
        case 12:
            return "cgrn";
        case 13:
            return "lyel";
        case 14:
            return "dyel";
        case 15:
            return "lgld";
        case 16:
            return "dgld";
        case 17:
            return "lpur";
        case 18:
            return "dpur";
        case 19:
            return "oran";
        case 20:
            return "bwht";
        }

        return std::string();
    }

    std::string D2RBinReader::processItemName(const std::string& code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        std::string strVal = code;
        strVal.erase(std::find_if(strVal.rbegin(), strVal.rend(), [](int ch) { return !isspace(ch); }).base(), strVal.end());
        const auto& itemTypeMap = parent.stringEnTxtByKey;
        auto iter = itemTypeMap.find(strVal);
        if (iter == itemTypeMap.end())
        {
            static std::set<std::string> builtInUnused = { "hpo", "mpo", "hpf", "mpf", "rps", "rpl", "bps", "bpl" };
            if (builtInUnused.find(strVal) != builtInUnused.end())
            {
                return strVal;
            }

            return "Unused";
        }

        return iter->second;
    }

    std::string D2RBinReader::processItemTypeName(const std::string& code, const D2RModReaderHelper& parent, size_t lineNum)
    {
        // trim trailing spaces
        std::string strVal = code;
        strVal.erase(std::find_if(strVal.rbegin(), strVal.rend(), [](int ch) { return !isspace(ch); }).base(), strVal.end());
        if (strVal.empty())
        {
            if (lineNum == 0)
            {
                return "Any";
            }

            return "Not Used";
        }

        const auto& itemTypeNameMap = parent.itemTypeNameMap;
        auto iter = itemTypeNameMap.find(strVal);
        if (iter == itemTypeNameMap.end())
        {
            if (strVal == "merc")
            {
                return "Merc Equip";
            }
            else
            {
                return strVal;
            }
        };

        return iter->second;
    }

    std::string D2RBinReader::processBodyLocation(const std::string& code, const D2RModReaderHelper& /*parent*/, size_t /*lineNum*/)
    {
        static std::map<std::string, std::string> valMap = { {"", "None"},
                                                             {"head", "Head"},
                                                             {"neck", "Neck"},
                                                             {"tors", "Torso"},
                                                             {"rarm", "Right Arm"},
                                                             {"larm", "Left Arm"},
                                                             {"rrin", "Right Ring"},
                                                             {"lrin", "Left Ring"},
                                                             {"belt", "Belt"},
                                                             {"feet", "Feet"},
                                                             {"glov", "Gloves"} };


        // trim trailing spaces
        std::string strVal = code;
        strVal.erase(std::find_if(strVal.rbegin(), strVal.rend(), [](int ch) { return !isspace(ch); }).base(), strVal.end());
        auto iter = valMap.find(strVal);
        if (iter == valMap.end())
        {
            return strVal;
        }

        return iter->second;
    }

    std::string D2RBinReader::processRuneName(const std::string& code, const D2RModReaderHelper& parent, size_t /*lineNum*/)
    {
        std::string strVal = code;
        strVal.erase(std::find_if(strVal.rbegin(), strVal.rend(), [](int ch) { return !isspace(ch); }).base(), strVal.end());
        const auto& itemTypeMap = parent.stringEnTxtByKey;
        auto iter = itemTypeMap.find(strVal);
        if (iter == itemTypeMap.end())
        {
            return strVal;
        }

        return iter->second;
    }

    class CsvTxtDocument : public ITxtDocument
    {
    public:
        CsvTxtDocument(std::istream& pStream,
            const rapidcsv::LabelParams& pLabelParams = rapidcsv::LabelParams(),
            const rapidcsv::SeparatorParams& pSeparatorParams = rapidcsv::SeparatorParams('\t'),
            const rapidcsv::ConverterParams& pConverterParams = rapidcsv::ConverterParams(),
            const rapidcsv::LineReaderParams& pLineReaderParams = rapidcsv::LineReaderParams(),
            size_t idx = 0)
            : doc(pStream, pLabelParams, pSeparatorParams, pConverterParams, pLineReaderParams), startIdx(idx)
        {
        }

        std::string GetRowName(SSIZE_T pRowIdx) const override
        {
            return doc.GetRowName(pRowIdx);
        }

        std::vector<std::string> GetColumnNames() const override
        {
            return doc.GetColumnNames();
        }

        std::string GetColumnName(SSIZE_T pColumnIdx) const override
        {
            return doc.GetColumnName(pColumnIdx);
        }

        size_t GetRowCount() const override
        {
            return doc.GetRowCount();
        }

        size_t GetColumnCount() const override
        {
            return doc.GetColumnCount();
        }

        SSIZE_T GetColumnIdx(const std::string& pColumnName) const override
        {
            return doc.GetColumnIdx(pColumnName);
        }

        std::string GetCellString(size_t pColumnIdx, size_t pRowIdx) const override
        {
            return doc.GetCell<std::string>(pColumnIdx, pRowIdx);
        }

        std::uint64_t GetCellUInt64(size_t pColumnIdx, size_t pRowIdx) const override
        {
            return doc.GetCell<std::uint64_t>(pColumnIdx, pRowIdx);
        }

        std::uint32_t GetCellUInt32(size_t pColumnIdx, size_t pRowIdx) const override
        {
            return doc.GetCell<std::uint32_t>(pColumnIdx, pRowIdx);
        }

        std::uint16_t GetCellUInt16(size_t pColumnIdx, size_t pRowIdx) const override
        {
            return doc.GetCell<std::uint16_t>(pColumnIdx, pRowIdx);
        }

        size_t GetRowValues(size_t rowIdx, std::string& index, std::map<std::string, std::string>& stringCols) const override
        {
            index.clear();
            stringCols.clear();

            index = doc.GetRowName(rowIdx);
            if (index.empty())
            {
                return MAXSIZE_T;
            }

            if (doc.GetColumnCount() != 1)
            {
                return MAXSIZE_T;
            }

            std::string strValue = doc.GetCell<std::string>(0, rowIdx);
            if (strValue.empty())
            {
                return MAXSIZE_T;
            }

            if (strValue.find("}") != strValue.npos)
            {
                const std::regex re{ "[^}]+" };
                const std::vector<std::string> tokens{
                    std::sregex_token_iterator(strValue.begin(), strValue.end(), re, 0),
                    std::sregex_token_iterator()
                };

                std::stringstream ss;
                auto iter = tokens.rbegin();
                for (; iter != tokens.rend(); ++iter)
                {
                    ss << *iter << "\n";
                }
                strValue = ss.str();
            }

            static std::string defaultColName("enUS");
            stringCols[defaultColName] = strValue;
            return rowIdx + startIdx;
        }

    protected:
        mutable rapidcsv::Document doc;
        size_t startIdx = 0;
    };

    class JsonStringsDocument : public ITxtDocument
    {
        std::string colId = "id";
        std::vector<std::string> columnNames;
        std::vector<std::string> rowNames;
        mutable std::map<std::string, std::map<std::string, std::string>> colValues;

    public:
        JsonStringsDocument()
        {
            LoadAllJson();
        }

        JsonStringsDocument(D2RModReaderHelper& jsonReader)
        {
            LoadAllJson(jsonReader);
        }

        std::string GetRowName(SSIZE_T pRowIdx) const override
        {
            return rowNames.at(pRowIdx);
        }

        std::vector<std::string> GetColumnNames() const override
        {
            return columnNames;
        }

        std::string GetColumnName(SSIZE_T pColumnIdx) const override
        {
            return columnNames.at(pColumnIdx);
        }

        size_t GetRowCount() const override
        {
            return rowNames.size();
        }

        size_t GetColumnCount() const override
        {
            return columnNames.size();
        }

        SSIZE_T GetColumnIdx(const std::string& pColumnName) const override
        {
            auto iter = std::find(columnNames.begin(), columnNames.end(), pColumnName);
            if (iter == columnNames.end())
            {
                return -1;
            }

            return std::distance(std::begin(columnNames), iter);
        }

        std::string GetCellString(size_t pColumnIdx, size_t pRowIdx) const override
        {
            const auto& row = colValues.at(rowNames.at(pRowIdx));
            auto iter = row.find(columnNames.at(pColumnIdx));
            if (iter == row.end())
            {
               return std::string();
            }

            return iter->second;
        }

        std::uint64_t GetCellUInt64(size_t pColumnIdx, size_t pRowIdx) const override
        {
            const auto& colName = columnNames.at(pColumnIdx);
            if (colName == colId)
            {
                const auto& row = colValues.at(rowNames.at(pRowIdx));
                auto iter = row.find(columnNames.at(pColumnIdx));
                if (iter == row.end())
                {
                    return 0;
                }

                return iter->second.empty() ? 0ui64 : static_cast<std::uint64_t>(std::stol(iter->second));
            }

            throw rapidcsv::no_converter();
        }

        std::uint32_t GetCellUInt32(size_t pColumnIdx, size_t pRowIdx) const override
        {
            return static_cast<std::uint32_t>(GetCellUInt64(pColumnIdx, pRowIdx));
        }

        std::uint16_t GetCellUInt16(size_t pColumnIdx, size_t pRowIdx) const override
        {
            return static_cast<std::uint16_t>(GetCellUInt64(pColumnIdx, pRowIdx));
        }

        size_t GetRowValues(size_t rowIdx, std::string& index, std::map<std::string, std::string>& stringCols) const override
        {
            index.clear();
            stringCols.clear();

            index = GetRowName(rowIdx);
            if (index.empty())
            {
                return MAXSIZE_T;
            }

            auto colIdx = GetColumnIdx(colId);
            if (colIdx < 0)
            {
                return MAXSIZE_T;
            }

            auto id = GetCellUInt32(colIdx, rowIdx);
            auto& row = colValues.at(index);
            if (row.empty())
            {
                return MAXSIZE_T;
            }

            stringCols.swap(row);
            auto iter = stringCols.find(colId);
            if (iter != stringCols.end())
            {
                stringCols.erase(iter);
            }

            return id;
        }

    protected:
        bool ParseChild(Json::Value& item)
        {
            auto members = item.getMemberNames();
            if (members.empty())
            {
                return false;
            }

            auto rowKey = item["Key"];
            if (rowKey.isNull())
            {
                return false;
            }

            std::string strValue;
            rowNames.push_back(rowKey.asString());
            auto& rowValues = colValues[rowNames.back()];
            for (auto& member : members)
            {
                if (member != "Key")
                {
                    auto iter = std::find(columnNames.begin(), columnNames.end(), member);
                    if (iter == columnNames.end())
                    {
                        columnNames.push_back(member);
                    }

                    auto value = item[member];
                    if (value.isNull())
                    {
                        continue;
                    }

                    strValue = value.asString();
                    if ((strValue.find("a0n1:") == 0) || (strValue.find("a1n0:") == 0))
                    {
                        // NOTE: Some langauges seem to begin with the string "a0n1:" or "a1n0" (not sure what this means, but needs to be stripped)
                        strValue.erase(0, 5);
                    }
                    rowValues[member] = strValue;
                }
            }
            return true;
        }

        bool LoadJson(std::istream& stream)
        {
            Json::Value root;
            Json::CharReaderBuilder builder;
            JSONCPP_STRING errs;
            if (!parseFromStream(builder, stream, &root, &errs))
            {
                return false;
            }

            if (root.isNull())
            {
                return false;
            }

            std::string key;
            auto iter_end = root.end();
            for (auto iter = root.begin(); iter != iter_end; ++iter)
            {
                if (iter->isNull())
                {
                    continue;
                }

                ParseChild(*iter);
            }

            return true;
        }

        template <typename F>
        void LoadJsonStream(F f)
        {
            std::istringstream stream(f);
            LoadJson(stream);
        }

        void LoadItems()
        {
            LoadJsonStream(JsonReaderHelper::GetItemGems());
            LoadJsonStream(JsonReaderHelper::GetItemModifiers());
            LoadJsonStream(JsonReaderHelper::GetItemNameAffixes());
            LoadJsonStream(JsonReaderHelper::GetItemNames());
            LoadJsonStream(JsonReaderHelper::GetItemRunes());
        }

        void LoadItems(D2RModReaderHelper& jsonReader)
        {
            if (!jsonReader.isValid())
            {
                return LoadItems();
            }

            LoadJsonStream(jsonReader.GetItemGems());
            LoadJsonStream(jsonReader.GetItemModifiers());
            LoadJsonStream(jsonReader.GetItemNameAffixes());
            LoadJsonStream(jsonReader.GetItemNames());
            LoadJsonStream(jsonReader.GetItemRunes());
        }

        void LoadAllJson()
        {
            LoadItems();
            LoadJsonStream(JsonReaderHelper::GetLevels());
            LoadJsonStream(JsonReaderHelper::GetMercenaries());
            LoadJsonStream(JsonReaderHelper::GetMonsters());
            LoadJsonStream(JsonReaderHelper::GetNPCs());
            LoadJsonStream(JsonReaderHelper::GetQuests());
            LoadJsonStream(JsonReaderHelper::GetItemGems());
            LoadJsonStream(JsonReaderHelper::GetSkills());
            LoadJsonStream(JsonReaderHelper::GetUI());
        }


        void LoadAllJson(D2RModReaderHelper& jsonReader)
        {
            if (!jsonReader.isValid())
            {
                return LoadAllJson();
            }

            LoadItems(jsonReader);
            LoadJsonStream(jsonReader.GetLevels());
            LoadJsonStream(jsonReader.GetMercenaries());
            LoadJsonStream(jsonReader.GetMonsters());
            LoadJsonStream(jsonReader.GetNPCs());
            LoadJsonStream(jsonReader.GetQuests());
            LoadJsonStream(jsonReader.GetItemGems());
            LoadJsonStream(jsonReader.GetSkills());
            LoadJsonStream(jsonReader.GetUI());
        }
    };

    class TxtReaderImpl : public ITxtReader
    {
    public:
        TxtReaderImpl()
        {
        }

        TxtReaderImpl(const std::filesystem::path& path) : modReader(std::make_unique<D2RModReaderHelper>(path))
        {
            if (!modReader->isValid())
            {
                modReader.reset();
            }
        }

        virtual std::unique_ptr<ITxtDocument> GetStringTxt() const override
        {
            if (modReader != nullptr)
            {
                return modReader->GetStringTxt();
            }

            return std::make_unique<JsonStringsDocument>();
        }

        virtual std::unique_ptr<ITxtDocument> GetExpansionStringTxt() const override
        {
            return nullptr;
        }

        virtual std::unique_ptr<ITxtDocument> GetPatchStringTxt() const override
        {
            return nullptr;
        }

        template <typename F>
        std::unique_ptr<ITxtDocument> LoadTxtStream(F f,
            const rapidcsv::LabelParams& pLabelParams = rapidcsv::LabelParams(),
            const rapidcsv::SeparatorParams& pSeparatorParams = rapidcsv::SeparatorParams('\t'),
            const rapidcsv::ConverterParams& pConverterParams = rapidcsv::ConverterParams(),
            const rapidcsv::LineReaderParams& pLineReaderParams = rapidcsv::LineReaderParams(),
            size_t idx = 0) const
        {
            std::istringstream stream(f);
            return std::make_unique<CsvTxtDocument>(stream, pLabelParams, pSeparatorParams, pConverterParams, pLineReaderParams, idx);
        }

        virtual std::unique_ptr<ITxtDocument> GetCharStatsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetCharStats());
            }

            return LoadTxtStream(TxtReaderHelper::GetCharStats());
        }

        virtual std::unique_ptr<ITxtDocument> GetPlayerClassTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetPlayerClass());
            }

            return LoadTxtStream(TxtReaderHelper::GetPlayerClass());
        }

        virtual std::unique_ptr<ITxtDocument> GetExperienceTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetExperience());
            }

            return LoadTxtStream(TxtReaderHelper::GetExperience());
        }

        virtual std::unique_ptr<ITxtDocument> GetHirelingTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetHireling());
            }

            return LoadTxtStream(TxtReaderHelper::GetHireling());
        }

        virtual std::unique_ptr<ITxtDocument> GetItemsCodeTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetItemsCode());
            }

            return LoadTxtStream(TxtReaderHelper::GetItemsCode());
        }

        virtual std::unique_ptr<ITxtDocument> GetItemStatCostTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetItemStatCost());
            }

            return LoadTxtStream(TxtReaderHelper::GetItemStatCost());
        }

        virtual std::unique_ptr<ITxtDocument> GetItemTypesTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetItemTypes());
            }

            return LoadTxtStream(TxtReaderHelper::GetItemTypes());
        }

        virtual std::unique_ptr<ITxtDocument> GetPropertiesTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetProperties());
            }

            return LoadTxtStream(TxtReaderHelper::GetProperties());
        }

        virtual std::unique_ptr<ITxtDocument> GetGemsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetGems());
            }

            return LoadTxtStream(TxtReaderHelper::GetGems());
        }

        virtual std::unique_ptr<ITxtDocument> GetBeltsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetBelts());
            }

            return LoadTxtStream(TxtReaderHelper::GetBelts());
        }

        virtual std::unique_ptr<ITxtDocument> GetArmorTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetArmor());
            }

            return LoadTxtStream(TxtReaderHelper::GetArmor());
        }

        virtual std::unique_ptr<ITxtDocument> GetWeaponsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetWeapons());
            }

            return LoadTxtStream(TxtReaderHelper::GetWeapons());
        }

        virtual std::unique_ptr<ITxtDocument> GetMiscTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetMisc());
            }

            return LoadTxtStream(TxtReaderHelper::GetMisc());
        }

        virtual std::unique_ptr<ITxtDocument> GetMagicPrefixTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetMagicPrefix());
            }

            return LoadTxtStream(TxtReaderHelper::GetMagicPrefix());
        }

        virtual std::unique_ptr<ITxtDocument> GetMagicSuffixTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetMagicSuffix());
            }

            return LoadTxtStream(TxtReaderHelper::GetMagicSuffix());
        }

        virtual std::unique_ptr<ITxtDocument> GetRarePrefixTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetRarePrefix());
            }

            return LoadTxtStream(TxtReaderHelper::GetRarePrefix());
        }

        virtual std::unique_ptr<ITxtDocument> GetRareSuffixTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetRareSuffix());
            }

            return LoadTxtStream(TxtReaderHelper::GetRareSuffix());
        }

        virtual std::unique_ptr<ITxtDocument> GetUniqueItemsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetUniqueItems());
            }

            return LoadTxtStream(TxtReaderHelper::GetUniqueItems());
        }

        virtual std::unique_ptr<ITxtDocument> GetSetsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetSets());
            }

            return LoadTxtStream(TxtReaderHelper::GetSets());
        }

        virtual std::unique_ptr<ITxtDocument> GetSetItemsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetSetItems());
            }

            return LoadTxtStream(TxtReaderHelper::GetSetItems());
        }

        virtual std::unique_ptr<ITxtDocument> GetRunesTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetRunes());
            }

            return LoadTxtStream(TxtReaderHelper::GetRunes());
        }

        virtual std::unique_ptr<ITxtDocument> GetSkillsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetSkillsTxt());
            }

            return LoadTxtStream(TxtReaderHelper::GetSkills());
        }

        virtual std::unique_ptr<ITxtDocument> GetSkillDescTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetSkillDesc());
            }

            return LoadTxtStream(TxtReaderHelper::GetSkillDesc());
        }

        virtual std::unique_ptr<ITxtDocument> GetMonStatsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetMonStats());
            }

            return LoadTxtStream(TxtReaderHelper::GetMonStats());
        }

        virtual std::unique_ptr<ITxtDocument> GetMonTypeTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetMonType());
            }

            return LoadTxtStream(TxtReaderHelper::GetMonType());
        }

        virtual std::unique_ptr<ITxtDocument> GetSuperiorModsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetSuperiorModsTxt());
            }

            return LoadTxtStream(TxtReaderHelper::GetSuperiorModsTxt());
        }

        virtual std::unique_ptr<ITxtDocument> GetCraftModsTxt() const override
        {
            if (modReader != nullptr)
            {
                return LoadTxtStream(modReader->GetCraftModsTxt());
            }

            return LoadTxtStream(TxtReaderHelper::GetCraftModsTxt());
        }

        virtual std::string GetModName() const override
        {
            if (modReader != nullptr)
            {
                return modReader->GetModName();
            }

            return std::string();
        }

    private:
        std::unique_ptr<D2RModReaderHelper> modReader;
    };

    std::unique_ptr<ITxtDocument> D2RModReaderHelper::GetStringTxt() const
    {
        return std::make_unique<JsonStringsDocument>(*const_cast<D2RModReaderHelper*>(this));
    }

    const ITxtReader& getDefaultTxtReader()
    {
        static TxtReaderImpl defaultTxtReader;
        return defaultTxtReader;
    }

    const ITxtReader& getDefaultTxtReader(const std::filesystem::path& modPath)
    {
        if (modPath.empty() || !std::filesystem::exists(modPath))
        {
            return getDefaultTxtReader();
        }

        static std::map<std::wstring, std::shared_ptr<TxtReaderImpl>> modTxtReaders;
        auto path = modPath.wstring();
        auto iter = modTxtReaders.find(path);
        if (iter != modTxtReaders.end())
        {
            return *iter->second.get();
        }

        auto newModTextReader = std::make_shared<TxtReaderImpl>(modPath);
        modTxtReaders[path] = newModTextReader;
        return *newModTextReader.get();
    }

    const ITxtReader& getDefaultTxtReader(std::string modName)
    {
        static auto d2RModsPath = GetD2RModsPath();
        if (d2RModsPath.empty() || !std::filesystem::exists(d2RModsPath))
        {
            return getDefaultTxtReader();
        }

        auto modPath = d2RModsPath / modName;
        return getDefaultTxtReader(modPath);
    }
}
//---------------------------------------------------------------------------
