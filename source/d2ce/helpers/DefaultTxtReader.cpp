/*
    Diablo II Character Editor
    Copyright (C) 2021-2022 Walter Couto

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
#include <utf8/utf8.h>

//---------------------------------------------------------------------------
namespace d2ce
{
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
            std::istringstream stream(f());
            LoadJson(stream);
        }
        
        void LoadItems()
        {
            LoadJsonStream(JsonReaderHelper::GetItemGems);
            LoadJsonStream(JsonReaderHelper::GetItemModifiers);
            LoadJsonStream(JsonReaderHelper::GetItemNameAffixes);
            LoadJsonStream(JsonReaderHelper::GetItemNames);
            LoadJsonStream(JsonReaderHelper::GetItemRunes);
        }

        void LoadAllJson()
        {
            LoadItems();
            LoadJsonStream(JsonReaderHelper::GetLevels);
            LoadJsonStream(JsonReaderHelper::GetMercenaries);
            LoadJsonStream(JsonReaderHelper::GetMonsters);
            LoadJsonStream(JsonReaderHelper::GetQuests);
            LoadJsonStream(JsonReaderHelper::GetItemGems);
            LoadJsonStream(JsonReaderHelper::GetSkills);
            LoadJsonStream(JsonReaderHelper::GetUI);
        }
    };

    class TxtReaderImpl : public ITxtReader
    {
        virtual std::unique_ptr<ITxtDocument> GetStringTxt() const override
        {
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
            std::istringstream stream(f());
            return std::make_unique<CsvTxtDocument>(stream, pLabelParams, pSeparatorParams, pConverterParams, pLineReaderParams, idx);
        }

        virtual std::unique_ptr<ITxtDocument> GetCharStatsTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetCharStats);
        }

        virtual std::unique_ptr<ITxtDocument> GetPlayerClassTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetPlayerClass);
        }

        virtual std::unique_ptr<ITxtDocument> GetExperienceTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetExperience);
        }

        virtual std::unique_ptr<ITxtDocument> GetHirelingTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetHireling);
        }

        virtual std::unique_ptr<ITxtDocument> GetItemStatCostTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetItemStatCost);
        }

        virtual std::unique_ptr<ITxtDocument> GetItemTypesTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetItemTypes);
        }

        virtual std::unique_ptr<ITxtDocument> GetPropertiesTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetProperties);
        }

        virtual std::unique_ptr<ITxtDocument> GetGemsTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetGems);
        }

        virtual std::unique_ptr<ITxtDocument> GetBeltsTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetBelts);
        }

        virtual std::unique_ptr<ITxtDocument> GetArmorTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetArmor);
        }

        virtual std::unique_ptr<ITxtDocument> GetWeaponsTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetWeapons);
        }

        virtual std::unique_ptr<ITxtDocument> GetMiscTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetMisc);
        }

        virtual std::unique_ptr<ITxtDocument> GetMagicPrefixTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetMagicPrefix);
        }

        virtual std::unique_ptr<ITxtDocument> GetMagicSuffixTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetMagicSuffix);
        }

        virtual std::unique_ptr<ITxtDocument> GetRarePrefixTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetRarePrefix);
        }

        virtual std::unique_ptr<ITxtDocument> GetRareSuffixTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetRareSuffix);
        }

        virtual std::unique_ptr<ITxtDocument> GetUniqueItemsTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetUniqueItems);
        }

        virtual std::unique_ptr<ITxtDocument> GetSetsTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetSets);
        }

        virtual std::unique_ptr<ITxtDocument> GetSetItemsTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetSetItems);
        }

        virtual std::unique_ptr<ITxtDocument> GetRunesTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetRunes);
        }

        virtual std::unique_ptr<ITxtDocument> GetSkillsTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetSkills);
        }

        virtual std::unique_ptr<ITxtDocument> GetSkillDescTxt() const override
        {
            return LoadTxtStream(TxtReaderHelper::GetSkillDesc);
        }
    };

    const ITxtReader& getDefaultTxtReader()
    {
        static TxtReaderImpl defaultTxtReader;
        return defaultTxtReader;
    }
}
//---------------------------------------------------------------------------
