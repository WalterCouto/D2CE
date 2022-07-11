/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
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

#pragma once

#include "CharacterConstants.h"
#include "ItemConstants.h"
#include "DataTypes.h"
#include <json/json.h>

namespace d2ce
{
    template <class _Ct, class _Pr>
    void removeItem_if(_Ct& _Cted, _Pr _Pred)
    {
        auto iter = std::remove_if(_Cted.begin(), _Cted.end(), _Pred);
        if (iter != _Cted.end())
        {
            _Cted.erase(iter, _Cted.end());
        }
    }

    namespace ItemHelpers
    {
        void setTxtReader(const ITxtReader& txtReader);
        const ITxtReader& getTxtReader();
        bool isTxtReaderInitialized();
        const std::string& getLanguage();
        const std::string& setLanguage(const std::string& lang);

        const d2ce::ItemStat& getItemStat(EnumItemVersion itemVersion, size_t idx);
        const d2ce::ItemStat& getItemStat(EnumItemVersion itemVersion, const std::string& name); // by stat name
        const d2ce::ItemStat& getItemStat(const MagicalAttribute& attrib);
        const bool hasItemStat(EnumItemVersion itemVersion, size_t idx);
        const bool hasItemStat(EnumItemVersion itemVersion, const std::string& name); // by stat name
        const bool hasItemStat(const MagicalAttribute& attrib);

        const ItemType& getItemTypeHelper(const std::array<std::uint8_t, 4>& strcode);
        const ItemType& getItemTypeHelper(const std::string& code);
        const ItemType& getUniqueItemTypeHelper(std::uint16_t id);
        const ItemType& getSetItemTypeHelper(std::uint16_t id);
        const ItemType& getSetItemTypeHelper(std::uint16_t id, const std::array<std::uint8_t, 4>& strcode);
        
        void getValidGPSCodes(std::vector <std::string>& gpsCodes, bool isExpansion = true);
        std::uint16_t getGPSSortIndex(const std::array<std::uint8_t, 4>& strcode);

        std::int64_t getMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx, const ItemStat& stat);
        void applyNonMaxMagicalAttributes(CharStats& cs, std::vector<MagicalAttribute>& attribs);
        void applyMaxMagicalAttributes(CharStats& cs, std::vector<MagicalAttribute>& attribs);
        bool formatMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel);

        std::vector<std::reference_wrapper<const ItemType>> getPossibleItems(std::uint16_t gameVersion, EnumCharClass charClass, const CharStats& cs);
    }
    //---------------------------------------------------------------------------

    namespace LocalizationHelpers
    {
        class ArgBase
        {
        public:
            ArgBase() {}
            virtual ~ArgBase() {}
            virtual void Format(std::ostringstream& ss, const std::string& fmt) = 0;
        };

        template <class T>
        class Arg : public ArgBase
        {
        public:
            Arg(T arg) : m_arg(arg) {}
            virtual ~Arg() {}
            virtual void Format(std::ostringstream& ss, const std::string& /*fmt*/)
            {
                ss << m_arg;
            }
        private:
            T m_arg;
        };

        class ArgArray : public std::vector<ArgBase*>
        {
        public:
            ArgArray() {}
            ~ArgArray()
            {
                std::for_each(begin(), end(), [](ArgBase* p) { delete p; });
            }
        };

        static void FormatItem(std::ostringstream& ss, const std::string& item, const ArgArray& args)
        {
            int index = 0;
            int alignment = 0;
            std::string fmt;

            char* endptr = nullptr;
            index = strtol(&item[0], &endptr, 10);
            if (index < 0 || index >= args.size())
            {
                return;
            }

            if (*endptr == ',')
            {
                alignment = strtol(endptr + 1, &endptr, 10);
                if (alignment > 0)
                {
                    ss << std::right << std::setw(alignment);
                }
                else if (alignment < 0)
                {
                    ss << std::left << std::setw(-alignment);
                }
            }

            if (*endptr == ':')
            {
                fmt = endptr + 1;
            }

            args[index]->Format(ss, fmt);

            return;
        }

        template <class T>
        static void Transfer(ArgArray& argArray, T t)
        {
            argArray.push_back(new Arg<T>(t));
        }

        template <class T, typename... Args>
        static void Transfer(ArgArray& argArray, T t, Args&&... args)
        {
            Transfer(argArray, t);
            Transfer(argArray, args...);
        }

        template<typename ... Args>
        std::string string_format(const std::string& format, Args ... args)
        {
            int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
            if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
            auto size = static_cast<size_t>(size_s);
            std::unique_ptr<char[]> buf(new char[size]);
            std::snprintf(buf.get(), size, format.c_str(), args ...);
            return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
        }

        template<typename ... Args>
        std::string string_formatDiablo(const std::string& format, Args ... args)
        {
            std::string result = format;
            if (sizeof...(args) == 0)
            {
                return format;
            }

            ArgArray argArray;
            Transfer(argArray, args...);
            size_t start = 0;
            size_t pos = 0;
            std::ostringstream ss;
            size_t formatSize = format.size();
            while (start < formatSize)
            {
                pos = format.find('%', start);
                if (pos == std::string::npos)
                {
                    ss << format.substr(start);
                    break;
                }

                ss << format.substr(start, pos - start);
                if (format[pos + 1] == '%')
                {
                    ss << '%';
                    start = pos + 2;
                    continue;
                }

                pos = pos + 1;
                start = pos; // supports 0 - 9 only
                FormatItem(ss, format.substr(start, pos - start + 1), argArray);
                start = pos + 1;
            }

            return ss.str();
        }

        template <typename... Args>
        std::string string_posformat(const std::string& format, Args&&... args)
        {
            if (sizeof...(args) == 0)
            {
                return format;
            }

            ArgArray argArray;
            Transfer(argArray, args...);
            size_t start = 0;
            size_t pos = 0;
            std::ostringstream ss;
            size_t formatSize = format.size();
            while (start < formatSize)
            {
                pos = format.find('{', start);
                if (pos == std::string::npos)
                {
                    ss << format.substr(start);
                    break;
                }

                ss << format.substr(start, pos - start);
                if (format[pos + 1] == '{')
                {
                    ss << '{';
                    start = pos + 2;
                    continue;
                }

                start = pos + 1;
                pos = format.find('}', start);
                if (pos == std::string::npos)
                {
                    ss << format.substr(start - 1);
                    break;
                }

                FormatItem(ss, format.substr(start, pos - start), argArray);
                start = pos + 1;
            }

            return ss.str();
        }

        bool GetStringTxtValue(const std::string& str, std::string& outStr, std::string& gender, const char* defValue = nullptr);
        bool GetStringTxtValue(const std::string& str, std::string& outStr, const char* defValue = nullptr);
        bool GetStringTxtValue(size_t id, std::string& outStr, std::string& gender, const char* defValue = nullptr);
        bool GetStringTxtValue(size_t id, std::string& outStr, const char* defValue = nullptr);
        const std::string& GetIndestructibleStringTxtValue(std::string& outStr, std::string& gender);
        const std::string& GetIndestructibleStringTxtValue(std::string& outStr);
        const std::string& GetEtherealStringTxtValue(std::string& outStr, std::string& gender);
        const std::string& GetEtherealStringTxtValue(std::string& outStr);
        const std::string& GetSocketedStringTxtValue(std::string& outStr, std::string& gender);
        const std::string& GetSocketedStringTxtValue(std::string& outStr);
        const std::string& GetDifficultyStringTxtValue(EnumDifficulty diff, std::string& outStr);

        const std::string& CheckCharName(std::string& curName, bool bASCII = false);
        const std::vector<std::string> GetCharacterTitles(bool isFemale = false, bool isHardcore = false, bool isExpansion = true);
        const std::vector<std::string> GetCharacterTypes(bool isExpansion = true);
    }
    //---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------

