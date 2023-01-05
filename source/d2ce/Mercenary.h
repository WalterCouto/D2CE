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

#pragma once

#include "Item.h"
#include "DataTypes.h"
#include "Constants.h"
#include "MercenaryConstants.h"
#include <json/json.h>

namespace d2ce
{
    class Character;

    //---------------------------------------------------------------------------
    class Mercenary
    {
        friend class Character;

    private:
        MercInfo Merc; // Dead:       pos 177 (1.09+ only)
                       // Id:         pos 179 (1.09+ only)
                       // NameId:     pos 183 (1.09+ only)
                       // Type:       pos 185 (1.09+ only)
                       // Experience: pos 187 (1.09+ only), hireling's experience

        Character& CharInfo;

    protected:
        Mercenary(Character& charInfo);

    public:
        ~Mercenary();

    protected:
        bool readInfo();
        bool readInfo(const Json::Value& root, bool bSerializedFormat);

        void setTxtReader();

    public:
        void clear();

        void fillMercInfo(MercInfo& merc) const;
        void updateMercInfo(MercInfo& merc);

        // Read-Only stats
        // Level, Experience, Strength, Dexterity and Life are set
        void fillMercStats(CharStats& cs) const;

        EnumCharVersion getVersion() const;
        std::uint32_t getLevel() const;
        std::uint32_t getMinLevel() const;
        std::uint32_t getMaxLevel() const;
        void setLevel(std::uint32_t level);
        std::uint32_t getExperience() const;
        std::uint32_t getMinExperience() const;
        std::uint32_t getMaxExperience() const;
        void setExperience(std::uint32_t experience);
        std::uint32_t getStrength() const;
        std::uint32_t getDexterity() const;
        EnumMercenaryClass getClass() const;
        const std::string& getClassName() const;
        void setClass(EnumMercenaryClass mercClass);
        EnumDifficulty getDifficulty() const;
        void setDifficulty(EnumDifficulty difficulty);
        std::uint8_t getAttributeId() const;
        void setAttributeId(std::uint8_t attributeId);
        std::string getAttributeName() const;
        std::uint16_t getNameId() const;
        void setNameId(std::uint16_t nameId);
        std::string getName() const;
        void getDamage(BaseDamage& damage) const;
        std::uint16_t getDefenseRating() const;
        std::uint16_t getAttackRating() const;
        void getResistance(BaseResistance& resist) const;

        bool isHired() const;
        void setIsHired(bool bIsHired);
        bool isDead() const;
        void setIsDead(bool bIsDead);
        const std::optional<d2ce::EnumCharClass>& getEquivClass() const;

        // Items
        size_t getNumberOfItems() const;
        const std::list<Item>& getItems() const;
        bool canEquipItem(const d2ce::Item& item, EnumEquippedId equipId) const;
        d2ce::EnumEquippedId verifyEquippedId(const d2ce::Item& item, EnumEquippedId equipId) const;

        bool getItemBonuses(std::vector<MagicalAttribute>& attribs) const;
        bool getDisplayedItemBonuses(std::vector<MagicalAttribute>& attribs) const;

        void asJson(Json::Value& parent, bool bSerializedFormat = false) const;

        static const std::vector<std::string>& getMercNames(EnumMercenaryClass mercClass);
        static const std::vector<std::string>& getMercClassNames();
        static const std::vector<std::string>& getMercAttributes(EnumMercenaryClass mercClass, EnumCharVersion version);
        static const std::string& getMercClassName(EnumMercenaryClass mercClass);
    };
}
//---------------------------------------------------------------------------

