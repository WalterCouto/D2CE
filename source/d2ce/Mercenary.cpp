/*
    Diablo II Character Editor
    Copyright (C) 2021 Walter Couto

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
#include "Mercenary.h"
#include "MercenaryConstants.h"
#include "Character.h"

//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::uint16_t ROUGE_SCOUT_ATTRIBUTE_END = 5;
    constexpr std::uint16_t DESERT_MERCENARY_ATTRIBUTE_END = 14;
    constexpr std::uint16_t IRON_WOLF_ATTRIBUTE_END = 23;

    struct MercStatSkillInfo
    {
        std::uint32_t Id = 0;
        std::uint32_t Level = 1;
        std::uint32_t LevelPerLvl = 0;
    };

    struct MercStatInfo
    {
        std::uint32_t Level = 1;
        std::vector<std::uint32_t> ExpPerLevel;
        std::uint32_t Life = 0;
        std::uint32_t LifePerLvl = 0;
        std::uint16_t Defense = 0;
        std::uint16_t DefPerLvl = 0;
        std::uint32_t Strength = 0;
        std::uint32_t StrPerLvl = 0;
        std::uint32_t Dexterity = 0;
        std::uint32_t DexPerLvl = 0;
        std::uint16_t AttackRating = 0;
        std::uint16_t ARPerLvl = 0;
        BaseDamage Damage;
        std::uint16_t DmgPerLvl = 0;
        std::uint16_t Resist = 0;
        std::uint16_t ResistPerLvl = 0;
    };

    const std::vector<MercStatInfo>& getMercStatInfo(d2ce::EnumDifficulty diff, d2ce::EnumMercenaryClass mercClass)
    {
        static std::vector<MercStatInfo> RoughScoutNormal = { { 3, { 100, 105 }, 45, 9, 15, 8, 35, 10, 45, 16, 10, 12, { 1, 3 }, 2, 0, 8 },
            { 36, { 100, 105 }, 342, 18, 279, 15, 77, 10, 111, 16, 406, 24, { 9, 11 }, 4, 66, 7 },
            { 67, { 100, 105 }, 900, 30, 744, 22, 116, 10, 173, 16, 1150, 36, { 25, 27 }, 6, 121, 5 }
        };
        static std::vector<MercStatInfo> RoughScoutNightmare = { { 36, { 110, 115 }, 308, 18, 251, 15, 74, 10, 106, 16, 365, 24, { 8, 10 }, 4, 63, 7 },
            { 67, { 110, 115 }, 866, 30, 716, 22, 113, 10, 168, 16, 1109, 36, { 24, 26 }, 6, 118, 5 }
        };
        static std::vector<MercStatInfo> RoughScoutHell = { { 67, { 120, 125 }, 779, 30, 644, 22, 110, 10, 163, 16, 998, 36, { 23, 25 }, 6, 115, 5 } };

        static std::vector<MercStatInfo> DesertMercenaryNormal = { { 9, { 110 }, 120, 15, 45, 11, 57, 14, 40, 12, 20, 12, { 7, 14 }, 4, 18, 8 },
            { 43, { 110 }, 630, 25, 419, 19, 117, 14, 91, 12, 428, 24, { 24, 31 }, 6, 86, 7 },
            { 75, { 110 }, 1430, 40, 1027, 28, 173, 14, 139, 12, 1196, 36, { 48, 55 }, 8, 142, 4 }
        };
        static std::vector<MercStatInfo> DesertMercenaryNightmare = { { 43, { 120 }, 567, 25, 377, 19, 113, 14, 87, 12, 385, 24, { 22, 29 }, 6, 83, 7 },
            { 75, { 120 }, 1367, 40, 985, 28, 169, 14, 135, 12, 1153, 36, { 46, 53 }, 8, 139, 4 }
        };
        static std::vector<MercStatInfo> DesertMercenaryHell = { { 75, { 130 }, 1230, 40, 887, 28, 165, 14, 131, 12, 1038, 36, { 44, 51 }, 8, 136, 4 } };

        static std::vector<MercStatInfo> IronWolfNormal = { { 15, { 110, 120, 110 }, 160, 9, 80, 5, 49, 10, 40, 8, 15, 12, { 1, 7 }, 4, 25, 7 },
            { 49, { 110, 120, 110 }, 466, 18, 250, 13, 92, 10, 74, 8, 423, 24, { 18, 24 }, 4, 85, 7 },
            { 79, { 110, 120, 110 }, 1006, 27, 640, 25, 130, 10, 104, 8, 1143, 36, { 33, 39 }, 4, 138, 6 }
        };
        static std::vector<MercStatInfo> IronWolfNightmare = { { 49, { 120, 130, 120 }, 419, 18, 225, 13, 88, 10, 70, 8, 381, 24, { 16, 22 }, 4, 82, 7 },
            { 79, { 120, 130, 120 }, 959, 27, 615, 25, 126, 10, 100, 8, 1101, 36, { 31, 37 }, 4, 135, 4 }
        };
        static std::vector<MercStatInfo> IronWolfHell = { { 79, { 130, 140, 130 }, 863, 27, 554, 25, 122, 10, 96, 8, 991, 36, { 29, 35 }, 4, 132, 4 } };

        static std::vector<MercStatInfo> BarbarianNormal = { { 28, { 120 }, 288, 18, 180, 10, 101, 15, 63, 10, 150, 20, { 16, 20 }, 6, 56, 7 },
            { 58, { 120 }, 828, 27, 480, 35, 158, 15, 101, 10, 750, 35, { 39, 43 }, 8, 109, 7 },
            { 80, { 120 }, 1422, 45, 1250, 50, 200, 15, 129, 10, 1520, 45, { 61, 65 }, 8, 148, 4 }
        };
        static std::vector<MercStatInfo> BarbarianNightmare = { { 58, { 130 }, 745, 27, 432, 35, 155, 15, 96, 10, 675, 35, { 37, 41 }, 8, 106, 7 },
            { 80, { 130 }, 1339, 45, 1202, 50, 197, 15, 124, 10, 1445, 45, { 59, 63 }, 8, 145, 4 }
        };
        static std::vector<MercStatInfo> BarbarianHell = { { 80, { 140 }, 1205, 45, 1082, 50, 194, 15, 119, 10, 1301, 45, { 57, 61 }, 8, 142, 4 } };

        switch (mercClass)
        {
        case d2ce::EnumMercenaryClass::RogueScout:
            switch (diff)
            {
            case d2ce::EnumDifficulty::Normal:
                return RoughScoutNormal;

            case d2ce::EnumDifficulty::Nightmare:
                return RoughScoutNightmare;

            case d2ce::EnumDifficulty::Hell:
                return RoughScoutHell;
            }
            break;

        case d2ce::EnumMercenaryClass::DesertMercenary:
            switch (diff)
            {
            case d2ce::EnumDifficulty::Normal:
                return DesertMercenaryNormal;

            case d2ce::EnumDifficulty::Nightmare:
                return DesertMercenaryNightmare;

            case d2ce::EnumDifficulty::Hell:
                return DesertMercenaryHell;
            }
            break;

        case EnumMercenaryClass::IronWolf:
            switch (diff)
            {
            case d2ce::EnumDifficulty::Normal:
                return IronWolfNormal;

            case d2ce::EnumDifficulty::Nightmare:
                return IronWolfNightmare;

            case d2ce::EnumDifficulty::Hell:
                return IronWolfHell;
            }
            break;

        case d2ce::EnumMercenaryClass::Barbarian:
            switch (diff)
            {
            case d2ce::EnumDifficulty::Normal:
                return BarbarianNormal;

            case d2ce::EnumDifficulty::Nightmare:
                return BarbarianNightmare;

            case d2ce::EnumDifficulty::Hell:
                return BarbarianHell;
            }
            break;
        }

        // should not happend
        static std::vector<MercStatInfo> badStatInfo;
        return badStatInfo;
    }

    const MercStatInfo& getMercStatInfoForLevel(std::uint32_t level, const std::vector<MercStatInfo>& mercStats)
    {
        static MercStatInfo badStatInfo;
        if (mercStats.size() == 0)
        {
            return badStatInfo;
        }

        if (mercStats.back().Level <= level)
        {
            // last Merc level is lower then our level
            return mercStats.back();
        }

        if (mercStats.front().Level >= level)
        {
            // first Merc Level is higher then our level
            return mercStats.front();
        }

        // find appropriate Merc stat
        MercStatInfo statInfo;
        auto iter = mercStats.begin();
        auto iter_end = mercStats.end();
        auto prevIter = iter;

        for (; iter != iter_end; prevIter = iter, ++iter)
        {
            if (iter->Level > level)
            {
                // return previous value
                return *prevIter;
            }
        }

        return mercStats.back();
    }

    MercStatInfo getMercStatInfoForLevel(std::uint32_t level, d2ce::EnumDifficulty diff, d2ce::EnumMercenaryClass mercClass)
    {
        return getMercStatInfoForLevel(level, getMercStatInfo(diff, mercClass));
    }

    std::uint32_t getMinLevelForStrength(std::uint32_t strength, const MercStatInfo& statInfo)
    {
        if ((strength <= statInfo.Strength) || (statInfo.StrPerLvl == 0))
        {
            return statInfo.Level;
        }
        return (strength - statInfo.Strength + (statInfo.StrPerLvl - 1)) / statInfo.StrPerLvl;
    }

    std::uint32_t getMinLevelForDexterity(std::uint32_t dexterity, const MercStatInfo& statInfo)
    {
        if ((dexterity <= statInfo.Dexterity) || (statInfo.DexPerLvl == 0))
        {
            return statInfo.Level;
        }

        return (dexterity - statInfo.Dexterity + (statInfo.DexPerLvl - 1)) / statInfo.DexPerLvl;
    }

    std::uint32_t getMerExpPerLevel(d2ce::EnumDifficulty diff, d2ce::EnumMercenaryClass mercClass, std::uint8_t attributeId)
    {
        // all levels have the same ExprPerLevel values, so get the first one
        const auto& statInfo = getMercStatInfoForLevel(1, diff, mercClass);
        if (statInfo.ExpPerLevel.empty())
        {
            // should not happen
            return 100;
        }

        if (statInfo.ExpPerLevel.size() <= attributeId)
        {
            // this attribute must be the same value as the last item in the list
            return statInfo.ExpPerLevel.back();
        }

        return statInfo.ExpPerLevel.at(attributeId);
    }

    std::uint32_t getMercExprForLevel(std::uint32_t level, std::uint32_t expPerLevel, const std::vector<MercStatInfo>& mercStats)
    {
        // Mercenary's max level is 98
        level = std::max(std::uint32_t(1), std::min(level, NUM_OF_LEVELS - 1));
        const auto& statInfo = getMercStatInfoForLevel(level, mercStats);
        return expPerLevel * (level + 1) * level * level + d2ce::MinExpRequired[statInfo.Level];
    }

    std::uint32_t getMercExprForLevel(std::uint32_t level, d2ce::EnumDifficulty diff, d2ce::EnumMercenaryClass mercClass, std::uint8_t attributeId)
    {
        return getMercExprForLevel(level, getMerExpPerLevel(diff, mercClass, attributeId), getMercStatInfo(diff, mercClass));
    }

    std::uint32_t getMercLevelForExpr(std::uint32_t experience, d2ce::EnumDifficulty diff, d2ce::EnumMercenaryClass mercClass, std::uint8_t attributeId)
    {
        // Mercenary's max level is 98
        auto expPerLevel = getMerExpPerLevel(diff, mercClass, attributeId);
        const auto& mercStats = getMercStatInfo(diff, mercClass);
        const auto& statInfo = getMercStatInfoForLevel(1, mercStats); // get bottom lowest level
        std::uint32_t level = statInfo.Level;
        while (experience >= getMercExprForLevel(level + 1, expPerLevel, mercStats) && level < d2ce::NUM_OF_LEVELS - 1)
        {
            ++level;
        }

        return level;
    }
}

//---------------------------------------------------------------------------
d2ce::Mercenary::Mercenary(Character& charInfo) : CharInfo(charInfo)
{
}
//---------------------------------------------------------------------------
d2ce::Mercenary::~Mercenary()
{
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::readInfo(EnumCharVersion version, std::FILE* charfile)
{
    Version = version;
    if (Version < EnumCharVersion::v109)
    {
        clear();
        return false;
    }

    mercInfo_location = 177;
    std::fseek(charfile, mercInfo_location, SEEK_SET);
    std::fread(&Merc.Dead, sizeof(Merc.Dead), 1, charfile);
    std::fread(&Merc.Id, sizeof(Merc.Id), 1, charfile);
    std::fread(&Merc.NameId, sizeof(Merc.NameId), 1, charfile);
    std::fread(&Merc.Type, sizeof(Merc.Type), 1, charfile);
    std::fread(&Merc.Experience, sizeof(Merc.Experience), 1, charfile);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::writeInfo(std::FILE* charfile)
{
    if (Version < EnumCharVersion::v109)
    {
        return false;
    }

    std::fseek(charfile, mercInfo_location, SEEK_SET);
    std::fwrite(&Merc.Dead, sizeof(Merc.Dead), 1, charfile);
    std::fwrite(&Merc.Id, sizeof(Merc.Id), 1, charfile);
    std::fwrite(&Merc.NameId, sizeof(Merc.NameId), 1, charfile);
    std::fwrite(&Merc.Type, sizeof(Merc.Type), 1, charfile);
    std::fwrite(&Merc.Experience, sizeof(Merc.Experience), 1, charfile);
    return true;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::clear()
{
    Merc.Dead = 0;
    Merc.Id = 0;
    Merc.NameId = 0;
    Merc.Type = 0;
    Merc.Experience = 0;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::fillMercInfo(MercInfo& merc) const
{
    std::memcpy(&merc, &Merc, sizeof(MercInfo));
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::updateMercInfo(MercInfo& merc)
{
    if (Version < EnumCharVersion::v109)
    {
        return;
    }

    auto bHasItems = !getItems().empty();
    if (isHired() && (merc.Id == 0) && bHasItems)
    {
        // can't unhire if we have items
        return;
    }

    auto oldMercClass = getClass();
    auto oldMercId = Merc.Id == 0 ? merc.Id : Merc.Id;
    auto oldMercName = Merc.NameId;
    std::memcpy(&Merc, &merc, sizeof(MercInfo));

    if (!isHired())
    {
        clear();
        return;
    }

    if (bHasItems && (oldMercClass != getClass()))
    {
        // Can't change type if we have items
        Merc.NameId = oldMercName;
        Merc.Id = oldMercId;
        setClass(oldMercClass);
    }

    // Check NameId
    setNameId(Merc.NameId);

    // check experience
    setExperience(Merc.Experience);
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::fillMercStats(CharStats& cs) const
{
    cs.clear();
    if (!isHired())
    {
        return;
    }

    cs.Level = getLevel();
    cs.Experience = Merc.Experience;

    const auto& statInfo = getMercStatInfoForLevel(cs.Level, getDifficulty(), getClass());
    cs.Strength = statInfo.Strength;
    cs.Dexterity = statInfo.Dexterity;
    cs.CurLife = statInfo.Life;
    cs.MaxLife = cs.CurLife;
    if (statInfo.Level >= cs.Level)
    {
        // min level
        return;
    }

    auto levelDiff = cs.Level - statInfo.Level;
    cs.Strength += (levelDiff * statInfo.StrPerLvl) / 8;
    cs.Dexterity += (levelDiff * statInfo.DexPerLvl) / 8;
    cs.CurLife += (levelDiff * statInfo.LifePerLvl);
    cs.MaxLife = cs.CurLife;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getLevel() const
{
    if (!isHired())
    {
        return 1;
    }

    // Mercenary's max level is 98
    return getMercLevelForExpr(Merc.Experience, getDifficulty(), getClass(), getAttributeId());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getMinLevel() const
{
    if (!isHired())
    {
        return 1;
    }

    const auto& statInfo = getMercStatInfoForLevel(1, getDifficulty(), getClass());
    ItemRequirements req;
    std::uint32_t level = statInfo.Level;
    for (const auto& item : getItems())
    {
        item.getRequirements(req);
        if (req.Level != 0)
        {
            level = std::max(level, std::uint32_t(req.Level));
        }

        if (req.Strength != 0)
        {
            level = std::max(level, getMinLevelForStrength(req.Strength, statInfo));
        }

        if (req.Dexterity != 0)
        {
            level = std::max(level, getMinLevelForDexterity(req.Dexterity, statInfo));
        }
    }

    // Mercenary's max level is 98
    return std::min(level, NUM_OF_LEVELS - 1);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getMaxLevel() const
{
    if (!isHired())
    {
        return 1;
    }

    return std::min(CharInfo.getLevel(), NUM_OF_LEVELS - 1);
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setLevel(std::uint32_t level)
{
    if (!isHired())
    {
        return;
    }

    // Make sure we can still use our items, is not at a higher level then our character and is at least our base level
    level = std::max(std::min(level, getMaxLevel()), getMinLevel());
    if (getLevel() == level)
    {
        return;
    }

    Merc.Experience = getMercExprForLevel(level, getDifficulty(), getClass(), getAttributeId());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getExperience() const
{
    if (!isHired())
    {
        return 0;
    }

    return Merc.Experience;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getMinExperience() const
{
    if (!isHired())
    {
        return 0;
    }

    return getMercExprForLevel(getMinLevel(), getDifficulty(), getClass(), getAttributeId());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getMaxExperience() const
{
    if (!isHired())
    {
        return 0;
    }

    // The max experience is the max for the current character's level
    return getMercExprForLevel(std::min(CharInfo.getLevel() + 1, NUM_OF_LEVELS), getDifficulty(), getClass(), getAttributeId()) - 1;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setExperience(std::uint32_t experience)
{
    if (!isHired())
    {
        return;
    }

    // Make sure we can still use our items, is not at a higher level then our character and is at least our base level
    Merc.Experience = std::max(std::min(experience, getMaxExperience()), getMinExperience());
}
//---------------------------------------------------------------------------
d2ce::EnumMercenaryClass d2ce::Mercenary::getClass() const
{
    if (!isHired())
    {
        return EnumMercenaryClass::None;
    }

    if (Merc.Type <= ROUGE_SCOUT_ATTRIBUTE_END)
    {
        return EnumMercenaryClass::RogueScout;
    }

    if (Merc.Type <= DESERT_MERCENARY_ATTRIBUTE_END)
    {
        return EnumMercenaryClass::DesertMercenary;
    }

    if (Merc.Type <= IRON_WOLF_ATTRIBUTE_END)
    {
        return EnumMercenaryClass::IronWolf;
    }

    return  EnumMercenaryClass::Barbarian;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setClass(EnumMercenaryClass mercClass)
{
    // Can only change type if we carry no items
    if (!isHired() || (getClass() == mercClass) || !getItems().empty())
    {
        return;
    }

    d2ce::MercInfo oldMerc;
    fillMercInfo(oldMerc);
    auto oldDifficulty = getDifficulty();

    switch (mercClass)
    {
    case EnumMercenaryClass::None:
        clear();
        break;

    case EnumMercenaryClass::RogueScout:
        Merc.Type = 0;
        Merc.NameId = std::min(Merc.NameId, std::uint16_t(std::size(RogueMercNames) - 1));
        break;

    case EnumMercenaryClass::DesertMercenary:
        Merc.Type = ROUGE_SCOUT_ATTRIBUTE_END + 1;
        Merc.NameId = std::min(Merc.NameId, std::uint16_t(std::size(DesertMercenaryNames) - 1));
        break;

    case EnumMercenaryClass::IronWolf:
        Merc.Type = DESERT_MERCENARY_ATTRIBUTE_END + 1;
        Merc.NameId = std::min(Merc.NameId, std::uint16_t(std::size(IronWolfNames) - 1));
        break;

    case EnumMercenaryClass::Barbarian:
        Merc.Type = IRON_WOLF_ATTRIBUTE_END + 1;
        break;
    }

    // restore difficulty level
    setDifficulty(oldDifficulty);

    // ensure name is valid
    setNameId(Merc.NameId);

    if (getLevel() > CharInfo.getLevel())
    {
        // The new class has a level higher then our character which is not allowed
        updateMercInfo(oldMerc);
    }
}
//---------------------------------------------------------------------------
d2ce::EnumDifficulty d2ce::Mercenary::getDifficulty() const
{
    if (!isHired())
    {
        return EnumDifficulty::Normal;
    }

    if (Merc.Type <= ROUGE_SCOUT_ATTRIBUTE_END)
    {
        switch (Merc.Type)
        {
        case 0:
        case 1:
            return EnumDifficulty::Normal;

        case 2:
        case 3:
            return EnumDifficulty::Nightmare;

        case 4:
        case 5:
            return EnumDifficulty::Hell;
        }

        // should not happen
        return EnumDifficulty::Normal;
    }

    if (Merc.Type <= DESERT_MERCENARY_ATTRIBUTE_END)
    {
        switch (Merc.Type - (ROUGE_SCOUT_ATTRIBUTE_END + 1))
        {
        case 0:
        case 1:
        case 2:
            return EnumDifficulty::Normal;

        case 3:
        case 4:
        case 5:
            return EnumDifficulty::Nightmare;

        case 6:
        case 7:
        case 8:
            return EnumDifficulty::Hell;
        }

        // should not happen
        return EnumDifficulty::Normal;
    }

    if (Merc.Type <= IRON_WOLF_ATTRIBUTE_END)
    {
        switch (Merc.Type - (DESERT_MERCENARY_ATTRIBUTE_END + 1))
        {
        case 0:
        case 1:
        case 2:
            return EnumDifficulty::Normal;

        case 3:
        case 4:
        case 5:
            return EnumDifficulty::Nightmare;

        case 6:
        case 7:
        case 8:
            return EnumDifficulty::Hell;
        }

        // should not happen
        return EnumDifficulty::Normal;
    }

    switch (Merc.Type - (IRON_WOLF_ATTRIBUTE_END + 1))
    {
    case 0:
        return EnumDifficulty::Normal;

    case 1:
        return EnumDifficulty::Nightmare;

    case 2:
        return EnumDifficulty::Hell;
    }

    // should not happen
    return EnumDifficulty::Normal;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setDifficulty(EnumDifficulty difficulty)
{
    if (!isHired())
    {
        return;
    }

    d2ce::MercInfo oldMerc;
    fillMercInfo(oldMerc);
    auto oldDifficulty = static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(getDifficulty());
    auto newDifficulty = static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(difficulty);
    switch (getClass())
    {
    case EnumMercenaryClass::RogueScout:
        if (newDifficulty < oldDifficulty)
        {
            Merc.Type -= (oldDifficulty - newDifficulty) * 2;
        }
        else if (newDifficulty > oldDifficulty)
        {
            Merc.Type += (newDifficulty - oldDifficulty) * 2;
        }
        break;

    case EnumMercenaryClass::DesertMercenary:
    case EnumMercenaryClass::IronWolf:
        if (newDifficulty < oldDifficulty)
        {
            Merc.Type -= (oldDifficulty - newDifficulty) * 3;
        }
        else if (newDifficulty > oldDifficulty)
        {
            Merc.Type += (newDifficulty - oldDifficulty) * 3;
        }
        break;

    case EnumMercenaryClass::Barbarian:
        if (newDifficulty < oldDifficulty)
        {
            Merc.Type -= (oldDifficulty - newDifficulty);
        }
        else if (newDifficulty > oldDifficulty)
        {
            Merc.Type += (newDifficulty - oldDifficulty);
        }
    }

    // check experience
    setExperience(Merc.Experience);

    // Make sure our level still makes sense
    if ((newDifficulty > oldDifficulty) && (getLevel() > CharInfo.getLevel()))
    {
        // this difficuly's level is too high, go back oribinal difficulty
        updateMercInfo(oldMerc);
    }
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Mercenary::getAttributeId() const
{
    if (!isHired())
    {
        return 0;
    }

    if (Merc.Type <= ROUGE_SCOUT_ATTRIBUTE_END)
    {
        return std::uint8_t(Merc.Type % 2);
    }

    if (Merc.Type <= DESERT_MERCENARY_ATTRIBUTE_END)
    {
        return std::uint8_t((Merc.Type - (ROUGE_SCOUT_ATTRIBUTE_END + 1)) % 3);
    }

    if (Merc.Type <= IRON_WOLF_ATTRIBUTE_END)
    {
        return std::uint8_t((Merc.Type - (DESERT_MERCENARY_ATTRIBUTE_END + 1)) % 3);
    }

    return 0;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setAttributeId(std::uint8_t attributeId)
{
    if (!isHired())
    {
        return;
    }

    switch (getClass())
    {
    case EnumMercenaryClass::RogueScout:
        attributeId = std::min(attributeId, std::uint8_t(std::size(RogueMercAttributes) - 1));
        break;

    case EnumMercenaryClass::DesertMercenary:
        attributeId = std::min(attributeId, std::uint8_t(std::size(DesertMercenaryAttributes) - 1));
        break;

    case EnumMercenaryClass::IronWolf:
        attributeId = std::min(attributeId, std::uint8_t(std::size(IronWolfAttributes) - 1));
        break;

    case EnumMercenaryClass::Barbarian:
        attributeId = 0;
        break;
    }

    d2ce::MercInfo oldMerc;
    fillMercInfo(oldMerc);
    auto oldAttributeId = getAttributeId();
    if (oldAttributeId > attributeId)
    {
        Merc.Type -= (oldAttributeId - attributeId);
    }
    else if (attributeId > oldAttributeId)
    {
        Merc.Type += (attributeId - oldAttributeId);
    }

    // check experience
    setExperience(Merc.Experience);

    if (getLevel() > CharInfo.getLevel())
    {
        // The new attribute type has a level higher then our character which is not allowed
        updateMercInfo(oldMerc);
    }
}
//---------------------------------------------------------------------------
std::string d2ce::Mercenary::getAttributeName() const
{
    if (!isHired())
    {
        return "";
    }

    switch (getClass())
    {
    case EnumMercenaryClass::RogueScout:
        return RogueMercAttributes[getAttributeId()];

    case EnumMercenaryClass::DesertMercenary:
        return DesertMercenaryAttributes[getAttributeId()];

    case EnumMercenaryClass::IronWolf:
        return IronWolfAttributes[getAttributeId()];
    }

    return "";
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Mercenary::getNameId() const
{
    if (!isHired())
    {
        return 0;
    }

    return Merc.NameId;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setNameId(std::uint16_t nameId)
{
    if (!isHired())
    {
        return;
    }

    switch (getClass())
    {
    case EnumMercenaryClass::RogueScout:
        Merc.NameId = std::min(nameId, std::uint16_t(std::size(RogueMercNames) - 1));
        break;

    case EnumMercenaryClass::DesertMercenary:
        Merc.NameId = std::min(nameId, std::uint16_t(std::size(DesertMercenaryNames) - 1));
        break;

    case EnumMercenaryClass::IronWolf:
        Merc.NameId = std::min(nameId, std::uint16_t(std::size(IronWolfNames) - 1));
        break;

    case EnumMercenaryClass::Barbarian:
        Merc.NameId = std::min(nameId, std::uint16_t(std::size(BarbarianMercNames) - 1));
        break;
    }
}
//---------------------------------------------------------------------------
std::string d2ce::Mercenary::getName() const
{
    if (!isHired())
    {
        return "";
    }

    switch (getClass())
    {
    case EnumMercenaryClass::RogueScout:
        if (Merc.NameId < std::size(RogueMercNames))
        {
            return RogueMercNames[Merc.NameId];
        }
        break;

    case EnumMercenaryClass::DesertMercenary:
        if (Merc.NameId < std::size(DesertMercenaryNames))
        {
            return DesertMercenaryNames[Merc.NameId];
        }
        break;

    case EnumMercenaryClass::IronWolf:
        if (Merc.NameId < std::size(IronWolfNames))
        {
            return IronWolfNames[Merc.NameId];
        }
        break;

    case EnumMercenaryClass::Barbarian:
        if (Merc.NameId < std::size(BarbarianMercNames))
        {
            return BarbarianMercNames[Merc.NameId];
        }
        break;
    }

    return "";
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::getDamage(BaseDamage& damage) const
{
    damage.clear();
    if (!isHired())
    {
        return;
    }

    auto level = getLevel();

    const auto& statInfo = getMercStatInfoForLevel(level, getDifficulty(), getClass());
    std::uint16_t levelDiff = std::uint16_t(level - statInfo.Level);
    std::uint16_t dmgDiff = std::uint16_t((levelDiff * statInfo.DmgPerLvl) / 8);
    damage.Min = statInfo.Damage.Min + dmgDiff;
    damage.Max = statInfo.Damage.Max + dmgDiff;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Mercenary::getDefenseRating() const
{
    if (!isHired())
    {
        return 0;
    }

    auto level = getLevel();

    const auto& statInfo = getMercStatInfoForLevel(level, getDifficulty(), getClass());
    std::uint16_t levelDiff = std::uint16_t(level - statInfo.Level);
    return statInfo.Defense + levelDiff * statInfo.DefPerLvl;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Mercenary::getAttackRating() const
{
    if (!isHired())
    {
        return 0;
    }

    auto level = getLevel();

    const auto& statInfo = getMercStatInfoForLevel(level, getDifficulty(), getClass());
    std::uint16_t levelDiff = std::uint16_t(level - statInfo.Level);
    return statInfo.AttackRating + levelDiff * statInfo.ARPerLvl;
}
//---------------------------------------------------------------------------
std::int16_t d2ce::Mercenary::getResistance() const
{
    if (!isHired())
    {
        return 0;
    }

    auto level = getLevel();

    const auto& statInfo = getMercStatInfoForLevel(level, getDifficulty(), getClass());
    std::int16_t levelDiff = std::int16_t(level - statInfo.Level);
    std::int16_t resist = std::int16_t(statInfo.Resist + (levelDiff * statInfo.ResistPerLvl) / 4);

    // Apply current difficulty penalty
    switch (CharInfo.getDifficultyLastPlayed())
    {
    case EnumDifficulty::Nightmare:
        resist -= 40;
        break;

    case EnumDifficulty::Hell:
        resist -= 100;
        break;
    }

    return std::min(std::max(std::int16_t(-100), resist), std::int16_t(75));
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::isHired() const
{
    return Merc.Id != 0 ? true : false;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setIsHired(bool bIsHired)
{
    if ((isHired() == bIsHired) || (Version < EnumCharVersion::v109))
    {
        return;
    }

    if (!bIsHired)
    {
        clear();
        return;
    }

    Merc.Id = uint32_t(rand() | 0x10); // new id

    // check experience
    setExperience(Merc.Experience);
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::isDead() const
{
    if (!isHired())
    {
        return false;
    }

    return Merc.Dead != 0 ? true : false;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setIsDead(bool bIsDead)
{
    if (!isHired())
    {
        return;
    }

    Merc.Dead = bIsDead ? 1 : 0;
}
//---------------------------------------------------------------------------
size_t d2ce::Mercenary::getNumberOfItems() const
{
    return CharInfo.getMercItems().size();
}
//---------------------------------------------------------------------------
const std::vector<d2ce::Item>& d2ce::Mercenary::getItems() const
{
    return CharInfo.getMercItems();
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::getItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    return CharInfo.getMercItemBonuses(attribs);
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::getDisplayedItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    return CharInfo.getDisplayedMercItemBonuses(attribs);
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::asJson(std::stringstream& ss, const std::string& parentIndent) const
{
    ss << "\n" << parentIndent << "\"dead_merc\": " << std::dec << Merc.Dead;
    ss << ",\n" << parentIndent << "\"merc_id\": \"" << std::hex << Merc.Id << "\"";
    ss << ",\n" << parentIndent << "\"merc_name_id\": " << std::dec << Merc.NameId;
    ss << ",\n" << parentIndent << "\"merc_type\": " << std::dec << Merc.Type;
    ss << ",\n" << parentIndent << "\"merc_experience\": " << std::dec << Merc.Experience;
}
//---------------------------------------------------------------------------