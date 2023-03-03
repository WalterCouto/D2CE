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
#include "DefaultTxtReader.h"
#include <rapidcsv/rapidcsv.h>
#include <json/json.h>
#include <sstream>
#include <iterator>
#include <regex>
#include <map>
#include <memory>
#include <utf8/utf8.h>

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
        const std::string& GetQuests();
        const std::string& GetSkills();
        const std::string& GetUI();
    }

    namespace TxtReaderHelper
    {
        const std::string& GetArmor();
        const std::string& GetBelts();
        const std::string& GetCharStats();
        const std::string& GetExperience();
        const std::string& GetGems();
        const std::string& GetHireling();
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
        const std::string& GetSuperiorModsTxt();
        const std::string& GetCraftModsTxt();
    }

    class D2RModReaderHelper
    {
    public:
        D2RModReaderHelper(const std::filesystem::path& path) : modPath(path)
        {
            Init();
        }

        bool isValid() const
        {
            return !modPath.empty();
        }

        std::string GetModName() const
        {
            if (!isValid())
            {
                return std::string();
            }

            return modPath.filename().u8string();
        }

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
            if (!LoadExcelText(strArmor, "armor.txt").empty())
            {
                return strArmor;
            }

            return TxtReaderHelper::GetArmor();
        }

        const std::string& GetBelts()
        {
            if (!LoadExcelText(strBelts, "belts.txt").empty())
            {
                return strBelts;
            }

            return TxtReaderHelper::GetBelts();
        }

        const std::string& GetCharStats()
        {
            if (!LoadExcelText(strCharStats, "charstats.txt").empty())
            {
                return strCharStats;
            }

            return TxtReaderHelper::GetCharStats();
        }

        const std::string& GetExperience()
        {
            if (!LoadExcelText(strExperience, "experience.txt").empty())
            {
                return strExperience;
            }

            return TxtReaderHelper::GetExperience();
        }

        const std::string& GetGems()
        {
            if (!LoadExcelText(strGems, "gems.txt").empty())
            {
                return strGems;
            }

            return TxtReaderHelper::GetGems();
        }

        const std::string& GetHireling()
        {
            if (!LoadExcelText(strHireling, "hireling.txt").empty())
            {
                return strHireling;
            }

            return TxtReaderHelper::GetHireling();
        }

        const std::string& GetItemStatCost()
        {
            if (!LoadExcelText(strItemStatCost, "itemstatcost.txt").empty())
            {
                return strItemStatCost;
            }

            return TxtReaderHelper::GetItemStatCost();
        }

        const std::string& GetItemTypes()
        {
            if (!LoadExcelText(strItemTypes, "itemtypes.txt").empty())
            {
                return strItemTypes;
            }

            return TxtReaderHelper::GetItemTypes();
        }

        const std::string& GetMagicPrefix()
        {
            if (!LoadExcelText(strMagicPrefix, "magicprefix.txt").empty())
            {
                return strMagicPrefix;
            }

            return TxtReaderHelper::GetMagicPrefix();
        }

        const std::string& GetMagicSuffix()
        {
            if (!LoadExcelText(strMagicSuffix, "magicsuffix.txt").empty())
            {
                return strMagicSuffix;
            }

            return TxtReaderHelper::GetMagicSuffix();
        }

        const std::string& GetMisc()
        {
            if (!LoadExcelText(strMisc, "misc.txt").empty())
            {
                return strMisc;
            }

            return TxtReaderHelper::GetMisc();
        }

        const std::string& GetPlayerClass()
        {
            if (!LoadExcelText(strPlayerClass, "playerclass.txt").empty())
            {
                return strPlayerClass;
            }

            return TxtReaderHelper::GetPlayerClass();
        }

        const std::string& GetProperties()
        {
            if (!LoadExcelText(strProperties, "properties.txt").empty())
            {
                return strProperties;
            }

            return TxtReaderHelper::GetProperties();
        }

        const std::string& GetRarePrefix()
        {
            if (!LoadExcelText(strRarePrefix, "rareprefix.txt").empty())
            {
                return strRarePrefix;
            }

            return TxtReaderHelper::GetRarePrefix();
        }

        const std::string& GetRareSuffix()
        {
            if (!LoadExcelText(strRareSuffix, "raresuffix.txt").empty())
            {
                return strRareSuffix;
            }

            return TxtReaderHelper::GetRareSuffix();
        }

        const std::string& GetRunes()
        {
            if (!LoadExcelText(strRunes, "runes.txt").empty())
            {
                return strRunes;
            }

            return TxtReaderHelper::GetRunes();
        }

        const std::string& GetSetItems()
        {
            if (!LoadExcelText(strSetItems, "setitems.txt").empty())
            {
                return strSetItems;
            }

            return TxtReaderHelper::GetSetItems();
        }

        const std::string& GetSets()
        {
            if (!LoadExcelText(strSets, "sets.txt").empty())
            {
                return strSets;
            }

            return TxtReaderHelper::GetSets();
        }

        const std::string& GetSkillDesc()
        {
            if (!LoadExcelText(strSkillDesc, "skilldesc.txt").empty())
            {
                return strSkillDesc;
            }

            return TxtReaderHelper::GetSkillDesc();
        }

        const std::string& GetSkillsTxt()
        {
            if (!LoadExcelText(strSkillsTxt, "skills.txt").empty())
            {
                return strSkillsTxt;
            }

            return TxtReaderHelper::GetSkills();
        }

        const std::string& GetUniqueItems()
        {
            if (!LoadExcelText(strUniqueItems, "uniqueitems.txt").empty())
            {
                return strUniqueItems;
            }

            return TxtReaderHelper::GetUniqueItems();
        }

        const std::string& GetWeapons()
        {
            if (!LoadExcelText(strWeapons, "weapons.txt").empty())
            {
                return strWeapons;
            }

            return TxtReaderHelper::GetWeapons();
        }

        const std::string& GetMonStats()
        {
            if (!LoadExcelText(strMonStats, "monstats.txt").empty())
            {
                return strMonStats;
            }

            return TxtReaderHelper::GetMonStats();
        }

        const std::string& GetSuperiorModsTxt()
        {
            if (!LoadExcelText(strSuperiorModsTxt, "qualityitems.txt").empty())
            {
                return strSuperiorModsTxt;
            }

            return TxtReaderHelper::GetSuperiorModsTxt();
        }

        const std::string& GetCraftModsTxt()
        {
            if (!LoadExcelText(strCraftModsTxt, "craftitems.txt").empty())
            {
                return strCraftModsTxt;
            }

            return TxtReaderHelper::GetCraftModsTxt();
        }

    private:
        void Init()
        {
            if (modPath.empty() || !std::filesystem::exists(modPath))
            {
                modPath.clear();
                return;
            }

            // check for mpq file or directory
            modPath /= modPath.filename();
            modPath.replace_extension(".mpq");
            if (!std::filesystem::exists(modPath))
            {
                modPath.clear();
                return;
            }

            if (std::filesystem::is_directory(modPath))
            {
                // we are directly dealing with json and csv files
                isArchive = false;
                modPath /= "data";
                if (!std::filesystem::exists(modPath) || !std::filesystem::is_directory(modPath))
                {
                    modPath.clear();
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

            // TODO: extract from mpq file
            isArchive = true;
        }

        std::string& LoadJsonText(std::string& jsonStr, const std::string& jsonFileName)
        {
            if (!jsonStr.empty())
            {
                return jsonStr;
            }

            if (modPath.empty() || modStringsPath.empty())
            {
                return jsonStr;
            }

            std::filesystem::path path = modStringsPath / jsonFileName;
            jsonStr = LoadTextFromFile(path);
            return jsonStr;
        }

        std::string& LoadExcelText(std::string& excelStr, const std::string& excelFileName)
        {
            if (!excelStr.empty())
            {
                return excelStr;
            }

            if (modPath.empty() || modExcelPath.empty())
            {
                return excelStr;
            }

            std::filesystem::path path = modExcelPath / excelFileName;
            excelStr = LoadTextFromFile(path);
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

    private:
        std::filesystem::path modPath;
        std::filesystem::path modStringsPath;
        std::filesystem::path modExcelPath;
        bool isArchive = false;

        std::string strItemGems;
        std::string strItemModifiers;
        std::string strItemNameAffixes;
        std::string strItemNames;
        std::string strItemRunes;
        std::string strLevels;
        std::string strMercenaries;
        std::string strMonsters;
        std::string strQuests;
        std::string strSkills;
        std::string strUI;

        std::string strArmor;
        std::string strBelts;
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
        std::string strSuperiorModsTxt;
        std::string strCraftModsTxt;
    };

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
                return "";
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
            if(iter != stringCols.end())
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
                return std::make_unique<JsonStringsDocument>(*modReader.get());
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
