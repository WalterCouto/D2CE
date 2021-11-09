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

#pragma once

#include "Constants.h"
#include <string>

namespace d2ce
{
    enum class EnumMercenaryClass : std::uint8_t { None = 0, RogueScout, DesertMercenary, IronWolf, Barbarian};

    const std::string MercClassNames[5] = { "", "Rogue Scout", "Desert Mercenary", "Iron Wolf", "Barbarian" };

    const std::string RogueMercNames[41] = { "Aliza", "Amplisa", "Annor", "Abhaya", "Elly", "Paige", "Basanti", "Blaise", "Kyoko", "Klaudia", "Kundri",
        "Kyle", "Visala", "Elexa", "Floria", "Fiona", "Gwinni", "Gaile", "Hannah", "Heather", "Iantha", "Diane",
        "Isolde", "Divo", "Ithera", "Itonya", "Liene", "Maeko", "Mahala", "Liaza", "Meghan", "Olena", "Oriana",
        "Ryann", "Rozene", "Raissa", "Sharyn", "Shikha", "Debi", "Tylena", "Wendy" };

    const std::string RogueMercAttributes[2] = { "Fire Arrow", "Cold Arrow" };

    const std::string DesertMercenaryNames[21] = { "Hazade", "Alhizeer", "Azrael", "Ahsab", "Chalan", "Haseen", "Razan", "Emilio", "Pratham", "Fazel",
        "Jemali", "Kasim",  "Gulzar", "Mizan", "Leharas", "Durga", "Neeraj", "Ilzan", "Zanarhi", "Waheed", "Vikhyat" };

    const std::string DesertMercenaryAttributes[3] = { "Combat", "Defense", "Offense" };

    const std::string IronWolfNames[20] = { "Jelani", "Barani", "Jabari", "Devak", "Raldin", "Telash", "Ajheed", "Narphet", "Khaleel", "Phaet", "Geshef",
        "Vanji", "Haphet", "Thadar", "Yatiraj", "Rhadge", "Yashied", "Lharhad", "Flux", "Scorch" };

    const std::string IronWolfAttributes[3] = { "Fire Spells", "Cold Spells", "Lightning Spells" };

    const std::string BarbarianMercNames[67] = { "Vardhaka", "Khan", "Klisk", "Bors", "Brom", "Wiglaf", "Hrothgar", "Scyld", "Healfdane", "Heorogar",
        "Halgaunt", "Hygelac", "Egtheow", "Bohdan", "Wulfgar", "Hild", "Heatholaf", "Weder", "Vikhyat", "Unferth", "Sigemund", "Heremod", "Hengest",
        "Folcwald", "Frisian", "Hnaef", "Guthlaf", "Oslaf", "Yrmenlaf", "Garmund", "Freawaru", "Eadgils", "Onela", "Damien", "Tryneus", "Klar", "Wulf",
        "Bulwye", "Lief", "Magnus", "Klatu", "Drus", "Hoku", "Kord", "Uther", "Ip", "Ulf", "Tharr", "Kaelim", "Ulric", "Alaric", "Ethelred", "Caden",
        "Elgifu", "Tostig", "Alcuin", "Emund", "Sigurd", "Gorm", "Hollis", "Ragnar", "Torkel", "Wulfstan", "Alban", "Barloc", "Bill", "Theodoric" };
}
//---------------------------------------------------------------------------
