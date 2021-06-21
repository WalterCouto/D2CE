/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
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
#include <map>
#include <bitset>
#include <sstream>
#include "Item.h"

//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::uint16_t MAX_TOME_QUANTITY = 20;     // max # of scrolls in tome
    constexpr std::uint16_t MAX_KEY_QUANTITY = 12;      // max # of keys stacked
    constexpr std::uint16_t MAX_STACKED_QUANTITY = 511; // max # of items in a stack
    constexpr std::uint16_t MAX_DURABILITY = 0xFF;      // max durability of an item (0 is indestructable)

    constexpr std::uint8_t ITEM_MARKER[] = { 0x4A, 0x4D };        // alternatively "JM"
    constexpr std::uint8_t MERC_ITEM_MARKER[] = { 0x6A, 0x66 };   // alternatively "jf"
    constexpr std::uint8_t GOLEM_ITEM_MARKER[] = { 0x6B, 0x66 };  // alternatively "jk"

    constexpr std::uint32_t MIN_START_STATS_POS = 765;

#define readtemp_bits(data,start,size) \
    ((*((std::uint64_t*) &(data)[(start) / 8]) >> ((start) & 7))& (((std::uint64_t)1 << (size)) - 1))

#define read_uint32_bits(start,size) \
    ((*((std::uint32_t *) &data[(start) / 8]) >> ((start) & 7))& (((std::uint32_t)1 << (size)) - 1))

#define read_uint64_bits(start,size) \
    ((*((std::uint64_t*) &data[(start) / 8]) >> ((start) & 7))& (((std::uint64_t)1 << (size)) - 1))

#define set_bit(start) \
    ((data[(start) / 8]) |= (std::uint8_t)(1ul << ((start) % 7)))

#define clear_bit(start) \
    ((data[(start) / 8]) &= ~(std::uint8_t)(1ul << ((start) % 7)))

    struct ItemStat
    {
        std::uint16_t id;
        std::uint8_t encode;
        std::uint16_t saveBits;
        std::int16_t saveAdd;
        std::uint16_t saveParamBits;
        std::uint16_t nextInChain;
        std::uint16_t charSaveBits;
    };

    const std::vector<ItemStat> itemStats = {
        {  0, 0,  8,  32,  0,  0, 10}, {  1, 0,  7, 32,  0,  0, 10}, {  2, 0,  7, 32,  0,  0, 10}, {  3, 0,  7,  32,  0,  0, 10},
        {  4, 0,  0,   0,  0,  0, 10}, {  5, 0,  0,  0,  0,  0,  8}, {  6, 0,  0,  0,  0,  0, 21}, {  7, 0,  9,  32,  0,  0, 21},
        {  8, 0,  0,   0,  0,  0, 21}, {  9, 0,  8, 32,  0,  0, 21}, { 10, 0,  0,  0,  0,  0, 21}, { 11, 0,  8,  32,  0,  0, 21},
        { 12, 0,  0,   0,  0,  0,  7}, { 13, 0,  0,  0,  0,  0, 32}, { 14, 0,  0,  0,  0,  0, 25}, { 15, 0,  0,   0,  0,  0, 25},
        { 16, 0,  9,   0,  0,  0,  0}, { 17, 0,  9,  0,  0, 18,  0}, { 18, 0,  9,  0,  0,  0,  0}, { 19, 0, 10,   0,  0,  0,  0},
        { 20, 0,  6,   0,  0,  0,  0}, { 21, 0,  6,  0,  0,  0,  0}, { 22, 0,  7,  0,  0,  0,  0}, { 23, 0,  6,   0,  0,  0,  0},
        { 24, 0,  7,   0,  0,  0,  0}, { 25, 0,  8,  0,  0,  0,  0}, { 26, 0,  8,  0,  0,  0,  0}, { 27, 0,  8,   0,  0,  0,  0},
        { 28, 0,  8,   0,  0,  0,  0}, { 29, 0,  0,  0,  0,  0,  0}, { 30, 0,  0,  0,  0,  0,  0}, { 31, 0, 11,  10,  0,  0,  0},
        { 32, 0,  9,   0,  0,  0,  0}, { 33, 0,  8,  0,  0,  0,  0}, { 34, 0,  6,  0,  0,  0,  0}, { 35, 0,  6,   0,  0,  0,  0},
        { 36, 0,  8,   0,  0,  0,  0}, { 37, 0,  8,  0,  0,  0,  0}, { 38, 0,  5,  0,  0,  0,  0}, { 39, 0,  8,  50,  0,  0,  0},
        { 40, 0,  5,   0,  0,  0,  0}, { 41, 0,  8, 50,  0,  0,  0}, { 42, 0,  5,  0,  0,  0,  0}, { 43, 0,  8,  50,  0,  0,  0},
        { 44, 0,  5,   0,  0,  0,  0}, { 45, 0,  8, 50,  0,  0,  0}, { 46, 0,  5,  0,  0,  0,  0}, { 47, 0,  0,   0,  0,  0,  0},
        { 48, 0,  8,   0,  0, 49,  0}, { 49, 0,  9,  0,  0,  0,  0}, { 50, 0,  6,  0,  0, 51,  0}, { 51, 0, 10,   0,  0,  0,  0},
        { 52, 0,  8,   0,  0, 53,  0}, { 53, 0,  9,  0,  0,  0,  0}, { 54, 0,  8,  0,  0, 55,  0}, { 55, 0,  9,   0,  0, 56,  0},
        { 56, 0,  8,   0,  0,  0,  0}, { 57, 0, 10,  0,  0, 58,  0}, { 58, 0, 10,  0,  0, 59,  0}, { 59, 0,  9,   0,  0,  0,  0},
        { 60, 0,  7,   0,  0,  0,  0}, { 61, 0,  0,  0,  0,  0,  0}, { 62, 0,  7,  0,  0,  0,  0}, { 63, 0,  0,   0,  0,  0,  0},
        { 64, 0,  0,   0,  0,  0,  0}, { 65, 0,  0,  0,  0,  0,  0}, { 66, 0,  0,  0,  0,  0,  0}, { 67, 0,  7,  30,  0,  0,  0},
        { 68, 0,  7,  30,  0,  0,  0}, { 69, 0,  0,  0,  0,  0,  0}, { 70, 0,  0,  0,  0,  0,  0}, { 71, 0,  8, 100,  0,  0,  0},
        { 72, 0,  9,   0,  0,  0,  0}, { 73, 0,  8,  0,  0,  0,  0}, { 74, 0,  6, 30,  0,  0,  0}, { 75, 0,  7,  20,  0,  0,  0},
        { 76, 0,  6,  10,  0,  0,  0}, { 77, 0,  6, 10,  0,  0,  0}, { 78, 0,  7,  0,  0,  0,  0}, { 79, 0,  9, 100,  0,  0,  0},
        { 80, 0,  8, 100,  0,  0,  0}, { 81, 0,  7,  0,  0,  0,  0}, { 82, 0,  9, 20,  0,  0,  0}, { 83, 0,  3,   0,  3,  0,  0},
        { 84, 0,  0,   0,  0,  0,  0}, { 85, 0,  9, 50,  0,  0,  0}, { 86, 0,  7,  0,  0,  0,  0}, { 87, 0,  7,   0,  0,  0,  0},
        { 88, 0,  1,   0,  0,  0,  0}, { 89, 0,  4,  4,  0,  0,  0}, { 90, 0, 24,  0,  0,  0,  0}, { 91, 0,  8, 100,  0,  0,  0},
        { 92, 0,  7,   0,  0,  0,  0}, { 93, 0,  7, 20,  0,  0,  0}, { 94, 0,  7, 64,  0,  0,  0}, { 95, 0,  0,   0,  0,  0,  0},
        { 96, 0,  7,  20,  0,  0,  0}, { 97, 1,  6,  0,  9,  0,  0}, { 98, 0,  1,  0,  8,  0,  0}, { 99, 0,  7,  20,  0,  0,  0},
        {100, 0,  0,   0,  0,  0,  0}, {101, 0,  0,  0,  0,  0,  0}, {102, 0,  7, 20,  0,  0,  0}, {103, 0,  0,   0,  0,  0,  0},
        {104, 0,  0,   0,  0,  0,  0}, {105, 0,  7, 20,  0,  0,  0}, {106, 0,  0,  0,  0,  0,  0}, {107, 1,  3,   0,  9,  0,  0},
        {108, 0,  1,   0,  0,  0,  0}, {109, 0,  9,  0,  0,  0,  0}, {110, 0,  8, 20,  0,  0,  0}, {111, 0,  9,  20,  0,  0,  0},
        {112, 0,  7,  -1,  0,  0,  0}, {113, 0,  7,  0,  0,  0,  0}, {114, 0,  6,  0,  0,  0,  0}, {115, 0,  1,   0,  0,  0,  0},
        {116, 0,  7,   0,  0,  0,  0}, {117, 0,  7,  0,  0,  0,  0}, {118, 0,  1,  0,  0,  0,  0}, {119, 0,  9,  20,  0,  0,  0},
        {120, 0,  7, 128,  0,  0,  0}, {121, 0,  9, 20,  0,  0,  0}, {122, 0,  9, 20,  0,  0,  0}, {123, 0, 10, 128,  0,  0,  0},
        {124, 0, 10, 128,  0,  0,  0}, {125, 0,  1,  0,  0,  0,  0}, {126, 0,  3,  0,  3,  0,  0}, {127, 0,  3,   0,  0,  0,  0},
        {128, 0,  5,   0,  0,  0,  0}, {129, 0,  0,  0,  0,  0,  0}, {130, 0,  0,  0,  0,  0,  0}, {131, 0,  0,   0,  0,  0,  0},
        {132, 0,  0,   0,  0,  0,  0}, {133, 0,  0,  0,  0,  0,  0}, {134, 0,  5,  0,  0,  0,  0}, {135, 0,  7,   0,  0,  0,  0},
        {136, 0,  7,   0,  0,  0,  0}, {137, 0,  7,  0,  0,  0,  0}, {138, 0,  7,  0,  0,  0,  0}, {139, 0,  7,   0,  0,  0,  0},
        {140, 0,  7,   0,  0,  0,  0}, {141, 0,  7,  0,  0,  0,  0}, {142, 0,  7,  0,  0,  0,  0}, {143, 0,  7,   0,  0,  0,  0},
        {144, 0,  7,   0,  0,  0,  0}, {145, 0,  7,  0,  0,  0,  0}, {146, 0,  7,  0,  0,  0,  0}, {147, 0,  7,   0,  0,  0,  0},
        {148, 0,  7,   0,  0,  0,  0}, {149, 0,  7,  0,  0,  0,  0}, {150, 0,  7,  0,  0,  0,  0}, {151, 0,  5,   0,  9,  0,  0},
        {152, 0,  1,   0,  0,  0,  0}, {153, 0,  1,  0,  0,  0,  0}, {154, 0,  7, 20,  0,  0,  0}, {155, 0,  7,   0, 10,  0,  0},
        {156, 0,  7,   0,  0,  0,  0}, {157, 0,  7,  0,  0,  0,  0}, {158, 0,  7,  0,  0,  0,  0}, {159, 0,  6,   0,  0,  0,  0},
        {160, 0,  7,   0,  0,  0,  0}, {161, 0,  0,  0,  0,  0,  0}, {162, 0,  0,  0,  0,  0,  0}, {163, 0,  0,   0,  0,  0,  0},
        {164, 0,  0,   0,  0,  0,  0}, {165, 0,  0,  0,  0,  0,  0}, {166, 0,  0,  0,  0,  0,  0}, {167, 0,  0,   0,  0,  0,  0},
        {168, 0,  0,   0,  0,  0,  0}, {169, 0,  0,  0,  0,  0,  0}, {170, 0,  0,  0,  0,  0,  0}, {171, 0,  0,   0,  0,  0,  0},
        {172, 0,  0,   0,  0,  0,  0}, {173, 0,  0,  0,  0,  0,  0}, {174, 0,  0,  0,  0,  0,  0}, {175, 0,  0,   0,  0,  0,  0},
        {176, 0,  0,   0,  0,  0,  0}, {177, 0,  0,  0,  0,  0,  0}, {178, 0,  0,  0,  0,  0,  0}, {179, 0,  9,   0, 10,  0,  0},
        {180, 0,  9,   0, 10,  0,  0}, {181, 0,  3,  0,  0,  0,  0}, {182, 0,  0,  0,  0,  0,  0}, {183, 0,  0,   0,  0,  0,  0},
        {184, 0,  0,   0,  0,  0,  0}, {185, 0,  0,  0,  0,  0,  0}, {186, 0,  0,  0,  0,  0,  0}, {187, 0,  0,   0,  0,  0,  0},
        {188, 0,  3,   0, 16,  0,  0}, {189, 0,  0,  0,  0,  0,  0}, {190, 0,  0,  0,  0,  0,  0}, {191, 0,  0,   0,  0,  0,  0},
        {192, 0,  0,   0,  0,  0,  0}, {193, 0,  0,  0,  0,  0,  0}, {194, 0,  4,  0,  0,  0,  0}, {195, 2,  7,   0, 16,  0,  0},
        {196, 2,  7,   0, 16,  0,  0}, {197, 2,  7,  0, 16,  0,  0}, {198, 2,  7,  0, 16,  0,  0}, {199, 2,  7,   0, 16,  0,  0},
        {200, 0,  0,   0,  0,  0,  0}, {201, 2,  7,  0, 16,  0,  0}, {202, 0,  0,  0,  0,  0,  0}, {203, 0,  0,   0,  0,  0,  0},
        {204, 3, 16,   0, 16,  0,  0}, {205, 3,  0,  0,  0,  0,  0}, {206, 3,  0,  0,  0,  0,  0}, {207, 3,  0,   0,  0,  0,  0},
        {208, 3,  0,   0,  0,  0,  0}, {209, 3,  0,  0,  0,  0,  0}, {210, 3,  0,  0,  0,  0,  0}, {211, 3,  0,   0,  0,  0,  0},
        {212, 3,  0,   0,  0,  0,  0}, {213, 3,  0,  0,  0,  0,  0}, {214, 0,  6,  0,  0,  0,  0}, {215, 0,  6,   0,  0,  0,  0},
        {216, 0,  6,   0,  0,  0,  0}, {217, 0,  6,  0,  0,  0,  0}, {218, 0,  6,  0,  0,  0,  0}, {219, 0,  6,   0,  0,  0,  0},
        {220, 0,  6,   0,  0,  0,  0}, {221, 0,  6,  0,  0,  0,  0}, {222, 0,  6,  0,  0,  0,  0}, {223, 0,  6,   0,  0,  0,  0},
        {224, 0,  6,   0,  0,  0,  0}, {225, 0,  6,  0,  0,  0,  0}, {226, 0,  6,  0,  0,  0,  0}, {227, 0,  6,   0,  0,  0,  0},
        {228, 0,  6,   0,  0,  0,  0}, {229, 0,  6,  0,  0,  0,  0}, {230, 0,  6,  0,  0,  0,  0}, {231, 0,  6,   0,  0,  0,  0},
        {232, 0,  6,   0,  0,  0,  0}, {233, 0,  6,  0,  0,  0,  0}, {234, 0,  6,  0,  0,  0,  0}, {235, 0,  6,   0,  0,  0,  0},
        {236, 0,  6,   0,  0,  0,  0}, {237, 0,  6,  0,  0,  0,  0}, {238, 0,  5,  0,  0,  0,  0}, {239, 0,  6,   0,  0,  0,  0},
        {240, 0,  6,   0,  0,  0,  0}, {241, 0,  6,  0,  0,  0,  0}, {242, 0,  6,  0,  0,  0,  0}, {243, 0,  6,   0,  0,  0,  0},
        {244, 0,  6,   0,  0,  0,  0}, {245, 0,  6,  0,  0,  0,  0}, {246, 0,  6,  0,  0,  0,  0}, {247, 0,  6,   0,  0,  0,  0},
        {248, 0,  6,   0,  0,  0,  0}, {249, 0,  6,  0,  0,  0,  0}, {250, 0,  6,  0,  0,  0,  0}, {251, 0,  0,   0,  0,  0,  0},
        {252, 0,  6,   0,  0,  0,  0}, {253, 0,  6,  0,  0,  0,  0}, {254, 0,  8,  0,  0,  0,  0}, {255, 0,  0,   0,  0,  0,  0},
        {256, 0,  0,   0,  0,  0,  0}, {257, 0,  0,  0,  0,  0,  0}, {258, 0,  0,  0,  0,  0,  0}, {259, 0,  0,   0,  0,  0,  0},
        {260, 0,  0,   0,  0,  0,  0}, {261, 0,  0,  0,  0,  0,  0}, {262, 0,  0,  0,  0,  0,  0}, {263, 0,  0,   0,  0,  0,  0},
        {264, 0,  0,   0,  0,  0,  0}, {265, 0,  0,  0,  0,  0,  0}, {266, 0,  0,  0,  0,  0,  0}, {267, 0,  0,   0,  0,  0,  0},
        {268, 4, 22,   0,  0,  0,  0}, {269, 4, 22,  0,  0,  0,  0}, {270, 4, 22,  0,  0,  0,  0}, {271, 4, 22,   0,  0,  0,  0},
        {272, 4, 22,   0,  0,  0,  0}, {273, 4, 22,  0,  0,  0,  0}, {274, 4, 22,  0,  0,  0,  0}, {275, 4, 22,   0,  0,  0,  0},
        {276, 4, 22,   0,  0,  0,  0}, {277, 4, 22,  0,  0,  0,  0}, {278, 4, 22,  0,  0,  0,  0}, {279, 4, 22,   0,  0,  0,  0},
        {280, 4, 22,   0,  0,  0,  0}, {281, 4, 22,  0,  0,  0,  0}, {282, 4, 22,  0,  0,  0,  0}, {283, 4, 22,   0,  0,  0,  0},
        {284, 4, 22,   0,  0,  0,  0}, {285, 4, 22,  0,  0,  0,  0}, {286, 4, 22,  0,  0,  0,  0}, {287, 4, 22,   0,  0,  0,  0},
        {288, 4, 22,   0,  0,  0,  0}, {289, 4, 22,  0,  0,  0,  0}, {290, 4, 22,  0,  0,  0,  0}, {291, 4, 22,   0,  0,  0,  0},
        {292, 4, 22,   0,  0,  0,  0}, {293, 4, 22,  0,  0,  0,  0}, {294, 4, 22,  0,  0,  0,  0}, {295, 4, 22,   0,  0,  0,  0},
        {296, 4, 22,   0,  0,  0,  0}, {297, 4, 22,  0,  0,  0,  0}, {298, 4, 22,  0,  0,  0,  0}, {299, 4, 22,   0,  0,  0,  0},
        {300, 4, 22,   0,  0,  0,  0}, {301, 4, 22,  0,  0,  0,  0}, {302, 4, 22,  0,  0,  0,  0}, {303, 4, 22,   0,  0,  0,  0},
        {304, 4,  0,   0,  0,  0,  0}, {305, 0,  8, 50,  0,  0,  0}, {306, 0,  8, 50,  0,  0,  0}, {307, 0,  8,  50,  0,  0,  0},
        {308, 0,  8,  50,  0,  0,  0}, {309, 0,  0,  0,  0,  0,  0}, {310, 0,  0,  0,  0,  0,  0}, {311, 0,  0,   0,  0,  0,  0},
        {312, 0,  0,   0,  0,  0,  0}, {313, 0,  0,  0,  0,  0,  0}, {314, 0,  0,  0,  0,  0,  0}, {315, 0,  0,   0,  0,  0,  0},
        {316, 0,  0,   0,  0,  0,  0}, {317, 0,  0,  0,  0,  0,  0}, {318, 0,  0,  0,  0,  0,  0}, {319, 0,  0,   0,  0,  0,  0},
        {320, 0,  0,   0,  0,  0,  0}, {321, 0,  0,  0,  0,  0,  0}, {322, 0,  0,  0,  0,  0,  0}, {323, 0,  0,   0,  0,  0,  0},
        {324, 0,  6,   0,  0,  0,  0}, {325, 0,  0,  0,  0,  0,  0}, {326, 0,  0,  0,  0,  0,  0}, {327, 0,  0,   0,  0,  0,  0},
        {328, 0,  0,   0,  0,  0,  0}, {329, 0,  9, 50,  0,  0,  0}, {330, 0,  9, 50,  0,  0,  0}, {331, 0,  9,  50,  0,  0,  0},
        {332, 0,  9,  50,  0,  0,  0}, {333, 0,  8,  0,  0,  0,  0}, {334, 0,  8,  0,  0,  0,  0}, {335, 0,  8,   0,  0,  0,  0},
        {336, 0,  8,   0,  0,  0,  0}, {337, 0,  8,  0,  0,  0,  0}, {338, 0,  7,  0,  0,  0,  0}, {339, 0,  7,   0,  0,  0,  0},
        {340, 0,  7,   0,  0,  0,  0}, {341, 0,  8,  0,  0,  0,  0}, {342, 0,  8,  0,  0,  0,  0}, {343, 0,  8,   0,  0,  0,  0},
        {344, 0,  8,   0,  0,  0,  0}, {345, 0,  8,  0,  0,  0,  0}, {346, 0,  8,  0,  0,  0,  0}, {347, 0,  8,   0,  0,  0,  0},
        {348, 0,  8,   0,  0,  0,  0}, {349, 0,  8,  0,  0,  0,  0}, {350, 0,  0,  0,  0,  0,  0}, {351, 0,  0,   0,  0,  0,  0},
        {352, 0,  0,   0,  0,  0,  0}, {353, 0,  0,  0,  0,  0,  0}, {354, 0,  0,  0,  0,  0,  0}, {355, 0,  0,   0,  0,  0,  0},
        {356, 0,  2,   0,  0,  0,  0}, {357, 0,  9, 50,  0,  0,  0}, {358, 0,  8,  0,  0,  0,  0}
    };

    const std::map<std::string, std::uint8_t> huffmanDecodeMap = {
        {"111101000", '\0'}, {       "01", ' '}, {"11011111", '0'}, { "0011111", '1'},
        {   "001100",  '2'}, {  "1011011", '3'}, {"01011111", '4'}, {"01101000", '5'},
        {  "1111011",  '6'}, {    "11110", '7'}, {  "001000", '8'}, {   "01110", '9'},
        {    "01111",  'a'}, {     "1010", 'b'}, {   "00010", 'c'}, {  "100011", 'd'},
        {   "000011",  'e'}, {   "110010", 'f'}, {   "01011", 'g'}, {   "11000", 'h'},
        {  "0111111",  'i'}, {"011101000", 'j'}, {  "010010", 'k'}, {   "10111", 'l'},
        {    "10110",  'm'}, {   "101100", 'n'}, { "1111111", 'o'}, {   "10011", 'p'},
        { "10011011",  'q'}, {    "00111", 'r'}, {    "0100", 's'}, {   "00110", 't'},
        {    "10000",  'u'}, {  "0111011", 'v'}, {   "00000", 'w'}, {   "11100", 'x'},
        {  "0101000",  'y'}, { "00011011", 'z'}
    };

    // Retrieves the huffman encoded chracter
    std::uint8_t getEncodedChar(const std::vector<std::uint8_t>& data, std::uint64_t& startOffset)
    {
        std::string bitStr;
        size_t startRead = startOffset;
        size_t readOffset = startRead;
        while (bitStr.size() < 9)
        {
            readOffset = startRead;
            std::stringstream ss2;
            ss2 << std::bitset<9>(read_uint32_bits(readOffset, bitStr.size() + 1));
            ++startOffset;
            bitStr = ss2.str().substr(8 - bitStr.size());
            auto iter = huffmanDecodeMap.find(bitStr);
            if (iter != huffmanDecodeMap.end())
            {
                return iter->second;
            }
        }

        // something went wrong
        return std::uint8_t(0xFF);
    }

    std::uint8_t getItemCodev115(const std::vector<std::uint8_t>& data, size_t startOffset, std::uint8_t(&strcode)[4])
    {
        size_t offset = startOffset;
        for (size_t i = 0; i < 4; ++i)
        {
            strcode[i] = getEncodedChar(data, offset);
        }

        return std::uint8_t(offset - startOffset);
    }

    // Retrieves encoded ItemCode (return number of bits set)
    void encodeItemCodev115(const std::uint8_t(&strcode)[4], std::uint64_t& encodedVal, std::uint8_t& numBitsSet)
    {
        encodedVal = 0;
        numBitsSet = 0;
        static const std::map<std::uint8_t, std::vector<std::uint16_t> > huffmanEncodeMap = {
            {'0', {223, 8}}, { '1', { 31, 7}}, {'2', { 12, 6}}, {'3', { 91, 7}},
            {'4', { 95, 8}}, { '5', {104, 8}}, {'6', {123, 7}}, {'7', { 30, 5}},
            {'8', {  8, 6}}, { '9', { 14, 5}}, {' ', {  1, 2}}, {'a', { 15, 5}},
            {'b', { 10, 4}}, { 'c', {  2, 5}}, {'d', { 35, 6}}, {'e', {  3, 6}},
            {'f', { 50, 6}}, { 'g', { 11, 5}}, {'h', { 24, 5}}, {'i', { 63, 7}},
            {'j', {232, 9}}, { 'k', { 18, 6}}, {'l', { 23, 5}}, {'m', { 22, 5}},
            {'n', { 44, 6}}, { 'o', {127, 7}}, {'p', { 19, 5}}, {'q', {155, 8}},
            {'r', {  7, 5}}, { 's', {  4, 4}}, {'t', {  6, 5}}, {'u', { 16, 5}},
            {'v', { 59, 7}}, { 'w', {  0, 5}}, {'x', { 28, 5}}, {'y', { 40, 7}},
            {'z', { 27, 8}}, {'\0', {488, 9}}
        };

        for (std::uint8_t i = 4; i > 0; --i)
        {
            auto iter = huffmanEncodeMap.find(strcode[i - 1]);
            if (iter == huffmanEncodeMap.end())
            {
                // 0
                encodedVal <<= 9;
                encodedVal |= 488;
                numBitsSet += 9;
            }
            else
            {
                encodedVal <<= iter->second[1];
                encodedVal |= iter->second[0];
                numBitsSet += std::uint8_t(iter->second[1]);
            }
        }
    }

    std::uint8_t getItemBaseCodeArmor(std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, int> armorBases = {
            {"cap", 4}, {"skp", 4}, {"hlm", 4}, {"fhl", 4}, {"ghm", 4}, {"crn", 4}, {"msk", 4},
            {"qui", 4}, {"lea", 4}, {"hla", 4}, {"stu", 4}, {"rng", 4}, {"scl", 4}, {"chn", 4},
            {"brs", 4}, {"spl", 4}, {"plt", 4}, {"fld", 4}, {"gth", 4}, {"ful", 4}, {"aar", 4},
            {"ltp", 4}, {"buc", 4}, {"sml", 4}, {"lrg", 4}, {"kit", 4}, {"tow", 4}, {"gts", 4},
            {"lgl", 4}, {"vgl", 4}, {"mgl", 4}, {"tgl", 4}, {"hgl", 4}, {"lbt", 4}, {"vbt", 4},
            {"mbt", 4}, {"tbt", 4}, {"hbt", 4}, {"lbl", 4}, {"vbl", 4}, {"mbl", 4}, {"tbl", 4},
            {"hbl", 4}, {"bhm", 4}, {"bsh", 4}, {"spk", 4}, {"xap", 4}, {"xkp", 4}, {"xlm", 4},
            {"xhl", 4}, {"xhm", 4}, {"xrn", 4}, {"xsk", 4}, {"xui", 4}, {"xea", 4}, {"xla", 4},
            {"xtu", 4}, {"xng", 4}, {"xcl", 4}, {"xhn", 4}, {"xrs", 4}, {"xpl", 4}, {"xlt", 4},
            {"xld", 4}, {"xth", 4}, {"xul", 4}, {"xar", 4}, {"xtp", 4}, {"xuc", 4}, {"xml", 4},
            {"xrg", 4}, {"xit", 4}, {"xow", 4}, {"xts", 4}, {"xlg", 4}, {"xvg", 4}, {"xmg", 4},
            {"xtg", 4}, {"xhg", 4}, {"xlb", 4}, {"xvb", 4}, {"xmb", 4}, {"xtb", 4}, {"xhb", 4},
            {"zlb", 4}, {"zvb", 4}, {"zmb", 4}, {"ztb", 4}, {"zhb", 4}, {"xh9", 4}, {"xsh", 4},
            {"xpk", 4}, {"dr1", 4}, {"dr2", 4}, {"dr3", 4}, {"dr4", 4}, {"dr5", 4}, {"ba1", 4},
            {"ba2", 4}, {"ba3", 4}, {"ba4", 4}, {"ba5", 4}, {"pa1", 4}, {"pa2", 4}, {"pa3", 4},
            {"pa4", 4}, {"pa5", 4}, {"ne1", 4}, {"ne2", 4}, {"ne3", 4}, {"ne4", 4}, {"ne5", 4},
            {"ci0", 4}, {"ci1", 4}, {"ci2", 4}, {"ci3", 4}, {"uap", 4}, {"ukp", 4}, {"ulm", 4},
            {"uhl", 4}, {"uhm", 4}, {"urn", 4}, {"usk", 4}, {"uui", 4}, {"uea", 4}, {"ula", 4},
            {"utu", 4}, {"ung", 4}, {"ucl", 4}, {"uhn", 4}, {"urs", 4}, {"upl", 4}, {"ult", 4},
            {"uld", 4}, {"uth", 4}, {"uul", 4}, {"uar", 4}, {"utp", 4}, {"uuc", 4}, {"uml", 4},
            {"urg", 4}, {"uit", 4}, {"uow", 4}, {"uts", 4}, {"ulg", 4}, {"uvg", 4}, {"umg", 4},
            {"utg", 4}, {"uhg", 4}, {"ulb", 4}, {"uvb", 4}, {"umb", 4}, {"utb", 4}, {"uhb", 4},
            {"ulc", 4}, {"uvc", 4}, {"umc", 4}, {"utc", 4}, {"uhc", 4}, {"uh9", 4}, {"ush", 4},
            {"upk", 4}, {"dr6", 4}, {"dr7", 4}, {"dr8", 4}, {"dr9", 4}, {"dra", 4}, {"ba6", 4},
            {"ba7", 4}, {"ba8", 4}, {"ba9", 4}, {"baa", 4}, {"pa6", 4}, {"pa7", 4}, {"pa8", 4},
            {"pa9", 4}, {"paa", 4}, {"ne6", 4}, {"ne7", 4}, {"ne8", 4}, {"ne9", 4}, {"nea", 4},
            {"drb", 4}, {"drc", 4}, {"drd", 4}, {"dre", 4}, {"drf", 4}, {"bab", 4}, {"bac", 4},
            {"bad", 4}, {"bae", 4}, {"baf", 4}, {"pab", 4}, {"pac", 4}, {"pad", 4}, {"pae", 4},
            {"paf", 4}, {"neb", 4}, {"neg", 4}, {"ned", 4}, {"nee", 4}, {"nef", 4}
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = armorBases.find(testStr);
        if (iter != armorBases.end())
        {
            return (std::uint8_t)iter->second;
        }

        return 0;
    }

    std::uint8_t getItemBaseCodeWeapon(std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, int> weaponBases = {
            {"hax", 2}, {"axe", 2}, {"2ax", 2}, {"mpi", 2}, {"wax", 2}, {"lax", 2}, {"bax", 2},
            {"btx", 2}, {"gax", 2}, {"gix", 2}, {"wnd", 2}, {"ywn", 2}, {"bwn", 2}, {"gwn", 2},
            {"clb", 2}, {"scp", 2}, {"gsc", 2}, {"wsp", 2}, {"spc", 2}, {"mac", 2}, {"mst", 2},
            {"fla", 2}, {"whm", 2}, {"mau", 2}, {"gma", 2}, {"ssd", 2}, {"scm", 2}, {"sbr", 2},
            {"flc", 2}, {"crs", 2}, {"bsd", 2}, {"lsd", 2}, {"wsd", 2}, {"2hs", 2}, {"clm", 2},
            {"gis", 2}, {"bsw", 2}, {"flb", 2}, {"gsd", 2}, {"dgr", 2}, {"dir", 2}, {"kri", 2},
            {"bld", 2}, {"tkf", 3}, {"tax", 3}, {"bkf", 3}, {"bal", 3}, {"jav", 3}, {"pil", 3},
            {"ssp", 3}, {"glv", 3}, {"tsp", 3}, {"spr", 2}, {"tri", 2}, {"brn", 2}, {"spt", 2},
            {"pik", 2}, {"bar", 2}, {"vou", 2}, {"scy", 2}, {"pax", 2}, {"hal", 2}, {"wsc", 2},
            {"sst", 2}, {"lst", 2}, {"cst", 2}, {"bst", 2}, {"wst", 2}, {"sbw", 2}, {"hbw", 2},
            {"lbw", 2}, {"cbw", 2}, {"sbb", 2}, {"lbb", 2}, {"swb", 2}, {"lwb", 2}, {"lxb", 2},
            {"mxb", 2}, {"hxb", 2}, {"rxb", 2}, {"gps", 3}, {"ops", 3}, {"gpm", 3}, {"opm", 3},
            {"gpl", 3}, {"opl", 3}, {"d33", 2}, {"g33", 2}, {"leg", 2}, {"hdm", 2}, {"hfh", 2},
            {"hst", 2}, {"msf", 2}, {"9ha", 2}, {"9ax", 2}, {"92a", 2}, {"9mp", 2}, {"9wa", 2},
            {"9la", 2}, {"9ba", 2}, {"9bt", 2}, {"9ga", 2}, {"9gi", 2}, {"9wn", 2}, {"9yw", 2},
            {"9bw", 2}, {"9gw", 2}, {"9cl", 2}, {"9sc", 2}, {"9qs", 2}, {"9ws", 2}, {"9sp", 2},
            {"9ma", 2}, {"9mt", 2}, {"9fl", 2}, {"9wh", 2}, {"9m9", 2}, {"9gm", 2}, {"9ss", 2},
            {"9sm", 2}, {"9sb", 2}, {"9fc", 2}, {"9cr", 2}, {"9bs", 2}, {"9ls", 2}, {"9wd", 2},
            {"92h", 2}, {"9cm", 2}, {"9gs", 2}, {"9b9", 2}, {"9fb", 2}, {"9gd", 2}, {"9dg", 2},
            {"9di", 2}, {"9kr", 2}, {"9bl", 2}, {"9tk", 3}, {"9ta", 3}, {"9bk", 3}, {"9b8", 3},
            {"9ja", 3}, {"9pi", 3}, {"9s9", 3}, {"9gl", 3}, {"9ts", 3}, {"9sr", 2}, {"9tr", 2},
            {"9br", 2}, {"9st", 2}, {"9p9", 2}, {"9b7", 2}, {"9vo", 2}, {"9s8", 2}, {"9pa", 2},
            {"9h9", 2}, {"9wc", 2}, {"8ss", 2}, {"8ls", 2}, {"8cs", 2}, {"8bs", 2}, {"8ws", 2},
            {"8sb", 2}, {"8hb", 2}, {"8lb", 2}, {"8cb", 2}, {"8s8", 2}, {"8l8", 2}, {"8sw", 2},
            {"8lw", 2}, {"8lx", 2}, {"8mx", 2}, {"8hx", 2}, {"8rx", 2}, {"qf1", 2}, {"qf2", 2},
            {"ktr", 2}, {"wrb", 2}, {"axf", 2}, {"ces", 2}, {"clw", 2}, {"btl", 2}, {"skr", 2},
            {"9ar", 2}, {"9wb", 2}, {"9xf", 2}, {"9cs", 2}, {"9lw", 2}, {"9tw", 2}, {"9qr", 2},
            {"7ar", 2}, {"7wb", 2}, {"7xf", 2}, {"7cs", 2}, {"7lw", 2}, {"7tw", 2}, {"7qr", 2},
            {"7ha", 2}, {"7ax", 2}, {"72a", 2}, {"7mp", 2}, {"7wa", 2}, {"7la", 2}, {"7ba", 2},
            {"7bt", 2}, {"7ga", 2}, {"7gi", 2}, {"7wn", 2}, {"7yw", 2}, {"7bw", 2}, {"7gw", 2},
            {"7cl", 2}, {"7sc", 2}, {"7qs", 2}, {"7ws", 2}, {"7sp", 2}, {"7ma", 2}, {"7mt", 2},
            {"7fl", 2}, {"7wh", 2}, {"7m7", 2}, {"7gm", 2}, {"7ss", 2}, {"7sm", 2}, {"7sb", 2},
            {"7fc", 2}, {"7cr", 2}, {"7bs", 2}, {"7ls", 2}, {"7wd", 2}, {"72h", 2}, {"7cm", 2},
            {"7gs", 2}, {"7b7", 2}, {"7fb", 2}, {"7gd", 2}, {"7dg", 2}, {"7di", 2}, {"7kr", 2},
            {"7bl", 2}, {"7tk", 3}, {"7ta", 3}, {"7bk", 3}, {"7b8", 3}, {"7ja", 3}, {"7pi", 3},
            {"7s7", 3}, {"7gl", 3}, {"7ts", 3}, {"7sr", 2}, {"7tr", 2}, {"7br", 2}, {"7st", 2},
            {"7p7", 2}, {"7o7", 2}, {"7vo", 2}, {"7s8", 2}, {"7pa", 2}, {"7h7", 2}, {"7wc", 2},
            {"6ss", 2}, {"6ls", 2}, {"6cs", 2}, {"6bs", 2}, {"6ws", 2}, {"6sb", 2}, {"6hb", 2},
            {"6lb", 2}, {"6cb", 2}, {"6s7", 2}, {"6l7", 2}, {"6sw", 2}, {"6lw", 2}, {"6lx", 2},
            {"6mx", 2}, {"6hx", 2}, {"6rx", 2}, {"ob1", 2}, {"ob2", 2}, {"ob3", 2}, {"ob4", 2},
            {"ob5", 2}, {"am1", 2}, {"am2", 2}, {"am3", 2}, {"am4", 2}, {"am5", 3}, {"ob6", 2},
            {"ob7", 2}, {"ob8", 2}, {"ob9", 2}, {"oba", 2}, {"am6", 2}, {"am7", 2}, {"am8", 2},
            {"am9", 2}, {"ama", 3}, {"obb", 2}, {"obc", 2}, {"obd", 2}, {"obe", 2}, {"obf", 2},
            {"amb", 2}, {"amc", 2}, {"amd", 2}, {"ame", 2}, {"amf", 3}
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = weaponBases.find(testStr);
        if (iter != weaponBases.end())
        {
            return (std::uint8_t)iter->second;
        }

        return 0;
    }

    std::uint8_t getItemBaseTome(std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, int> tomeBases = {
            {"tbk", 9},{"ibk", 9}
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = tomeBases.find(testStr);
        if (iter != tomeBases.end())
        {
            return (std::uint8_t)iter->second;
        }

        return 0;
    }

    std::uint8_t getItemBaseStackable(std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, int> stackableBases = { {"gld", 1},{"aqv", 1},{"cqv", 1},{"key", 1},{"rps", 1},{"rpl", 1},{"bps", 1},{"bpl", 1} };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = stackableBases.find(testStr);
        if (iter != stackableBases.end())
        {
            return (std::uint8_t)iter->second;
        }

        // Could be a Tome
        auto result = getItemBaseTome(strcode);
        if ((result & 0x01) != 0)
        {
            return result;
        }

        // Could be a weapon
        result = getItemBaseCodeWeapon(strcode);
        if ((result & 0x01) != 0)
        {
            return result;
        }

        return 0;
    }

    std::uint8_t getItemBase(std::uint8_t(&strcode)[4])
    {
        // could be stackable
        auto result = getItemBaseStackable(strcode);
        if (result != 0)
        {
            // It's a stackable item
            return result;
        }

        // Could be armor (all tomes are stackable so covered by call to getItemBaseStackable)
        result = getItemBaseCodeArmor(strcode);
        if (result != 0)
        {
            return result;
        }

        // Could be a weapon
        result = getItemBaseCodeWeapon(strcode);
        if (result != 0)
        {
            return result;
        }

        // assume item is a none of the above (could be an invalid code as well but we'll ignore that)
        return 0;
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
d2ce::Item::Item()
{
}
//---------------------------------------------------------------------------
d2ce::Item::Item(size_t itemsize) : data(itemsize)
{
}
d2ce::Item::Item(const Item& other)
{
    *this = other;
}
//---------------------------------------------------------------------------
d2ce::Item::~Item()
{
}
//---------------------------------------------------------------------------
d2ce::Item& d2ce::Item::operator=(const Item& other)
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    data = other.data;
    SocketedItems = other.SocketedItems;

    FileVersion = other.FileVersion;
    start_bit_offset = other.start_bit_offset;
    is_potion_bit_offset = other.is_potion_bit_offset;
    location_bit_offset = other.location_bit_offset;
    type_code_offset = other.type_code_offset;
    extended_data_offset = other.extended_data_offset;
    item_id_bit_offset = other.item_id_bit_offset;
    item_level_bit_offset = other.item_level_bit_offset;
    multi_graphic_bit_offset = other.multi_graphic_bit_offset;
    autoAffix_bit_offset = other.autoAffix_bit_offset;
    quality_bit_offset = other.quality_bit_offset;
    personalized_bit_offset = other.personalized_bit_offset;
    armor_or_weapon_bit_offset = other.armor_or_weapon_bit_offset;
    durability_bit_offset = other.durability_bit_offset;
    stackable_bit_offset = other.stackable_bit_offset;
    socket_count_bit_offset = other.socket_count_bit_offset;
    bonus_bits_bit_offset = other.bonus_bits_bit_offset;
    magical_props_bit_offset = other.magical_props_bit_offset;
    set_bonus_props_bit_offset = other.set_bonus_props_bit_offset;
    runeword_props_bit_offset = other.runeword_props_bit_offset;
    item_end_bit_offset = other.item_end_bit_offset;
    return *this;
}
d2ce::Item& d2ce::Item::operator=(Item&& other) noexcept
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    data.swap(other.data);
    other.data.clear();
    SocketedItems.swap(other.SocketedItems);
    other.SocketedItems.clear();
    FileVersion = std::exchange(other.FileVersion, APP_CHAR_VERSION);
    start_bit_offset = std::exchange(other.start_bit_offset, 16);
    is_potion_bit_offset = std::exchange(other.is_potion_bit_offset, 36);
    location_bit_offset = std::exchange(other.location_bit_offset, 58);
    type_code_offset = std::exchange(other.type_code_offset, 76);
    extended_data_offset = std::exchange(other.extended_data_offset, 108);
    item_id_bit_offset = std::exchange(other.item_id_bit_offset, 0);
    item_level_bit_offset = std::exchange(other.item_level_bit_offset, 0);
    multi_graphic_bit_offset = std::exchange(other.multi_graphic_bit_offset, 0);
    autoAffix_bit_offset = std::exchange(other.autoAffix_bit_offset, 0);
    quality_bit_offset = std::exchange(other.quality_bit_offset, 0);
    personalized_bit_offset = std::exchange(other.personalized_bit_offset, 0);
    armor_or_weapon_bit_offset = std::exchange(other.armor_or_weapon_bit_offset, 0);
    durability_bit_offset = std::exchange(other.durability_bit_offset, 0);
    stackable_bit_offset = std::exchange(other.stackable_bit_offset, 0);
    socket_count_bit_offset = std::exchange(other.socket_count_bit_offset, 0);
    bonus_bits_bit_offset = std::exchange(other.bonus_bits_bit_offset, 0);
    magical_props_bit_offset = std::exchange(other.magical_props_bit_offset, 0);
    set_bonus_props_bit_offset = std::exchange(other.set_bonus_props_bit_offset, 0);
    runeword_props_bit_offset = std::exchange(other.runeword_props_bit_offset, 0);
    item_end_bit_offset = std::exchange(other.item_end_bit_offset, 0);
    return *this;
}
//---------------------------------------------------------------------------
void d2ce::Item::swap(Item& other)
{
    std::swap(*this, other);
}
//---------------------------------------------------------------------------
std::uint8_t& d2ce::Item::operator [](size_t position) const
{
    return data[position];
}
//---------------------------------------------------------------------------
// returns the number of bytes not including socketed items
size_t d2ce::Item::size() const
{
    return data.size();
}
//---------------------------------------------------------------------------
// returns the number of bytes to store the item, including socketed items
size_t d2ce::Item::getFullSize() const
{
    size_t byteSize = data.size();
    for (auto& item : SocketedItems)
    {
        byteSize += item.getFullSize();
    }

    return byteSize;
}
//---------------------------------------------------------------------------
// itemsize is the number of bytes not including socketed items
void d2ce::Item::resize(size_t itemsize)
{
    data.resize(itemsize);
}
//---------------------------------------------------------------------------
// itemsize is the number of bytes not including socketed items
void d2ce::Item::reserve(size_t itemsize)
{
    data.reserve(itemsize);
}
//---------------------------------------------------------------------------
void d2ce::Item::clear()
{
    *this = Item();
}
//---------------------------------------------------------------------------
void d2ce::Item::push_back(const std::uint8_t& value)
{
    data.push_back(value);
}
//---------------------------------------------------------------------------
d2ce::EnumItemVersion d2ce::Item::Version() const
{
    std::uint16_t value = 0;
    if (FileVersion < EnumCharVersion::v115) // pre-1.15 character file
    {
        value = read_uint32_bits(start_bit_offset + 32, 8);
        switch (value)
        {
        case 0: // pre-1.08
            if (FileVersion < EnumCharVersion::v107)       // pre-1.07 character file
            {
                if (data.size() == 27)
                {
                    return EnumItemVersion::v100; // v1.00 - v1.03
                }

                return EnumItemVersion::v104;     // v1.04 - v1.06 item
            }

            return EnumItemVersion::v107;         // v1.07 item

        case 1:
        case 100:
            return EnumItemVersion::v108;         // v1.08/1.09 normal or expansion

        case 2:
        case 101:
            return EnumItemVersion::v110;         // v1.10 normal or expansion
        }

        return EnumItemVersion::v110;             // v1.10 normal or expansion
    }

    value = read_uint32_bits(start_bit_offset + 32, 3); // TODO: do we do anything with this value?
    return EnumItemVersion::v115;                 // v1.15 Diable II Resurrected
}
//---------------------------------------------------------------------------
bool d2ce::Item::isIdentified() const
{
    return read_uint32_bits(start_bit_offset + 4, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isPotion() const
{
    if (is_potion_bit_offset == 0)
    {
        is_potion_bit_offset = start_bit_offset + 10;
    }

    return read_uint32_bits(is_potion_bit_offset, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isSocketed() const
{
    return read_uint32_bits(start_bit_offset + 11, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isNew() const
{
    return read_uint32_bits(start_bit_offset + 13, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isEar() const
{
    return read_uint32_bits(start_bit_offset + 16, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isStarterItem() const
{
    return read_uint32_bits(start_bit_offset + 17, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
/*
   Returns true if this item is a simple item (i.e. 14 byte item)
*/
bool d2ce::Item::isSimpleItem() const
{
    if (FileVersion < EnumCharVersion::v107 && Version() == EnumItemVersion::v100)
    {
        // v1.00 - v1.03
        return true;
    }

    return read_uint32_bits(start_bit_offset + 21, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isEthereal() const
{
    return read_uint32_bits(start_bit_offset + 22, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isPersonalized() const
{
    return read_uint32_bits(start_bit_offset + 24, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRuneword() const
{
    return read_uint32_bits(start_bit_offset + 26, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getLocation() const
{
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        if ((data[9] & 0x80) == 0x80 &&
            (data[10] & 0x01) == 0x00)
        {
            return BELT;
        }
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06
        if ((data[8] & 0x03FC) == 0x03FC)
        {
            return GLUED_IN_SOCKET;
        }

        if ((data[8] & 0x03FC) == 0x01FC)
        {
            return BELT;
        }
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II Resurrected
        return (std::uint8_t)read_uint32_bits(location_bit_offset, 3);
    }

    return 0;
}
//---------------------------------------------------------------------------
bool d2ce::Item::calculateItemCode(std::uint8_t(&strcode)[4]) const
{
    std::memset(strcode, 0, sizeof(strcode));
    if (isEar())
    {
        return false;
    }

    std::uint64_t code = 0;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        code = ((std::uint64_t(data[8]) >> 4) & 0x0F) | (std::uint64_t(data[9]) << 4);
        switch (code)
        {
        case 0x10: // antidote potion
            strcode[0] = 'y';
            strcode[1] = 'p';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

        case 0x11: // stamina potion
            strcode[0] = 'v';
            strcode[1] = 'p';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

        case 0x12: // regular rejuvenation potion
            strcode[0] = 'r';
            strcode[1] = 'v';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

        case 0x13: // full rejuvenation potion
            strcode[0] = 'r';
            strcode[1] = 'v';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;

        case 0x14: // thawing potion
            strcode[0] = 'w';
            strcode[1] = 'm';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

        case 0x3C: // chipped amethyst
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;
        case 0x3D: // flawed amethyst
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;
        case 0x3E: // regular amethyst
            strcode[0] = 'g';
            strcode[1] = 'u';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;
        case 0x3F: // flawless amethyst
            strcode[0] = 'g';
            strcode[1] = 'z';
            strcode[2] = 'v';
            return true;
        case 0x40: // perfect amethyst
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;

        case 0x41: // chipped topaz
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x42: // flawed topaz
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x43: // regular topaz
            strcode[0] = 'g';
            strcode[1] = 'u';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x44: // flawless topaz
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x45: // perfect topaz
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;

        case 0x46: // chipped sapphire
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x47: // flawed sapphire
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x48: // regular sapphire
            strcode[0] = 'g';
            strcode[1] = 'u';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x49: // flawless sapphire
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            break;
        case 0x4A: // perfect sapphire
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;

        case 0x4B: // chipped emerald
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            break;
        case 0x4C: // flawed emerald
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x4D: // regular emerald
            strcode[0] = 'g';
            strcode[1] = 'u';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x4E: // flawless emerald
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x4F: // perfect emerald
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;

        case 0x50: // chipped ruby
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x510: // flawed ruby
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x52: // regular ruby
            strcode[0] = 'g';
            strcode[1] = 'u';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x53: // flawless ruby
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x54: // perfect ruby
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;

        case 0x55: // chipped diamond
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x56: // flawed diamond
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x57: // regular diamond
            strcode[0] = 'g';
            strcode[1] = 'u';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x58: // flawless diamond
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x59: // perfect diamond
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;

        case 0x5A: // minor healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '1';
            strcode[3] = 0x20;
            return true;
        case 0x5B: // light healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '2';
            strcode[3] = 0x20;
            break;
        case 0x5C: // regular healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '3';
            strcode[3] = 0x20;
            return true;
        case 0x5D: // Greater healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '4';
            strcode[3] = 0x20;
            return true;
        case 0x5E: // Super healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '5';
            return true;

        case 0x5F: // minor mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '1';
            strcode[3] = 0x20;
            break;
        case 0x60: // light mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '2';
            strcode[3] = 0x20;
            return true;
        case 0x61: // regular mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '3';
            strcode[3] = 0x20;
            return true;
        case 0x62: // Greater mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '4';
            strcode[3] = 0x20;
            return true;
        case 0x63: // Super mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '5';
            strcode[3] = 0x20;
            return true;

        case 0x64: // chipped skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x65: // flawed skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'f';
            strcode[3] = 0x20;
            return true;
        case 0x66: // regular skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'u';
            strcode[3] = 0x20;
            return true;
        case 0x67: // flawless skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            break;
        case 0x68: // perfect skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'z';
            strcode[3] = 0x20;
            return true;
        }  // end switch
        return false;

    case EnumItemVersion::v104: // v1.04 - v1.06
        code = data[13];
        code <<= 8;
        code += data[12];
        code <<= 8;
        code += data[11];
        code <<= 8;
        code += data[10];
        code >>= 2;

        strcode[0] = std::uint8_t(code & 0xFF);
        code >>= 8;
        strcode[1] = std::uint8_t(code & 0xFF);
        code >>= 8;
        strcode[2] = std::uint8_t(code & 0xFF);
        code >>= 8;
        strcode[3] = std::uint8_t(code & 0xFF);
        return true;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
        strcode[0] = std::uint8_t(read_uint32_bits(type_code_offset, 8));
        strcode[1] = std::uint8_t(read_uint32_bits(type_code_offset + 8, 8));
        strcode[2] = std::uint8_t(read_uint32_bits(type_code_offset + 16, 8));
        strcode[3] = std::uint8_t(read_uint32_bits(type_code_offset + 24, 8));
        return true;

    case EnumItemVersion::v115: // v1.15 Diable II Resurrected
        getItemCodev115(data, type_code_offset, strcode);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateGem(const std::uint8_t(&newgem)[4])
{
    if (isEar())
    {
        return false;
    }

    // Check for compatible location for update
    bool isPotion = false;
    const std::uint8_t& gem = newgem[0];
    const std::uint8_t& gemCondition = newgem[1];
    const std::uint8_t& gemColour = newgem[2];
    switch (gem)
    {
    case 'g': // gem
        if (getLocation() == BELT)
        {
            // can't put gem in belt
            return false;
        }
        break;

    case 's': // skull
        if (gemCondition == 'k')
        {
            if (getLocation() == BELT)
            {
                // can't put skull in belt
                return false;
            }
        }
        break;

    case 'h': // healing
    case 'm': // mana
        if (gemCondition == 'p')
        {
            isPotion = true;
            if (getLocation() == GLUED_IN_SOCKET)
            {
                // can't put potion in socket
                return false;
            }
        }
        break;

    case 'r': // rejuvenation potion
        if (gemCondition == 'v')
        {
            isPotion = true;
            if (getLocation() == GLUED_IN_SOCKET)
            {
                // can't put potion in socket
                return false;
            }
        }
        break;

    case 'y': // antidote potion
    case 'v': //stamina potion
        if (gemCondition == 'p' && gemColour == 's')
        {
            // these guys are potion but not marked as such
            if (getLocation() == GLUED_IN_SOCKET)
            {
                // can't put potion in socket
                return false;
            }
        }
        break;

    case 'w': // thawing potion
        if (gemCondition == 'm' && gemColour == 's')
        {
            // this guy is a potion but not marked as such
            if (getLocation() == GLUED_IN_SOCKET)
            {
                // can't put potion in socket
                return false;
            }
        }
        break;
    }

    std::uint8_t gemValue = 0;
    std::uint64_t code = 0;
    std::uint8_t numBitsSet = 0;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        switch (gem)
        {
        case 'g': // gem
            switch (gemColour)
            {
            case 'v': // amethyst
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x3C;
                    break;
                case 'f': // flawed
                    gemValue = 0x3D;
                    break;
                case 'u': // regular
                    gemValue = 0x3E;
                    break;
                case 'z': // flawless
                    gemValue = 0x3F;
                    break;
                default: // perfect
                    gemValue = 0x40;
                    break;
                }
                break;
            case 'y': // topaz
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x41;
                    break;
                case 'f': // flawed
                    gemValue = 0x42;
                    break;
                case 'u': // regular
                    gemValue = 0x43;
                    break;
                case 'l': // flawless
                    gemValue = 0x44;
                    break;
                default: // perfect
                    gemValue = 0x45;
                    break;
                }
                break;
            case 'b': // sapphire
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x46;
                    break;
                case 'f': // flawed
                    gemValue = 0x47;
                    break;
                case 'u': // regular
                    gemValue = 0x48;
                    break;
                case 'l': // flawless
                    gemValue = 0x49;
                    break;
                default: // perfect
                    gemValue = 0x4A;
                    break;
                }
                break;
            case 'g': // emerald
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x4B;
                    break;
                case 'f': // flawed
                    gemValue = 0x4C;
                    break;
                case 'u': // regular
                    gemValue = 0x4D;
                    break;
                case 'l': // flawless
                    gemValue = 0x4E;
                    break;
                default: // perfect
                    gemValue = 0x4F;
                    break;
                }
                break;
            case 'r': // ruby
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x50;
                    break;
                case 'f': // flawed
                    gemValue = 0x51;
                    break;
                case 'u': // regular
                    gemValue = 0x52;
                    break;
                case 'l': // flawless
                    gemValue = 0x53;
                    break;
                default: // perfect
                    gemValue = 0x54;
                    break;
                }
                break;
            case 'w': // diamond
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x55;
                    break;
                case 'f': // flawed
                    gemValue = 0x56;
                    break;
                case 'u': // regular
                    gemValue = 0x57;
                    break;
                case 'l': // flawless
                    gemValue = 0x58;
                    break;
                default: // perfect
                    gemValue = 0x59;
                    break;
                }
                break;
            }
            break;

        case 's': // skull
            if (gemCondition == 'k')
            {
                switch (gemColour)
                {
                case 'c': // chipped
                    gemValue = 0x64;
                    break;
                case 'f': // flawed
                    gemValue = 0x65;
                    break;
                case 'u': // regular
                    gemValue = 0x66;
                    break;
                case 'l': // flawless
                    gemValue = 0x67;
                    break;
                default: // perfect
                    gemValue = 0x68;
                    break;
                }
            }
            break;

        case 'h': // healing
            if (gemCondition == 'p')
            {
                switch (gemColour)
                {
                case '1': // Minor
                    gemValue = 0x5A;
                    break;
                case '2': // Light
                    gemValue = 0x5B;
                    break;
                case '3': // Regular
                    gemValue = 0x5C;
                    break;
                case '4': // Greater
                    gemValue = 0x5D;
                    break;
                default: // Super
                    gemValue = 0x5E;
                    break;
                }
            }
            break;

        case 'm': // mana
            if (gemCondition == 'p')
            {
                switch (gemColour)
                {
                case '1': // Minor
                    gemValue = 0x5F;
                    break;
                case '2': // Light
                    gemValue = 0x60;
                    break;
                case '3': // Regular
                    gemValue = 0x61;
                    break;
                case '4': // Greater
                    gemValue = 0x62;
                    break;
                default: // Super
                    gemValue = 0x63;
                    break;
                }
            }
            break;

        case 'r': // rejuvenation potion
            if (gemCondition == 'v')
            {
                switch (gemColour)
                {
                case 's': // Regular
                    gemValue = 0x12;
                    break;
                default: // Full
                    gemValue = 0x13;
                    break;
                }
            }
            break;

        case 'y': // antidote potion
            if (gemCondition == 'p' && gemColour == 's')
            {
                gemValue = 0x11;
            }
            break;

        case 'v': //stamina potion
            if (gemCondition == 'p' && gemColour == 's')
            {
                gemValue = 0x10;
            }
            break;

        case 'w': // thawing potion
            if (gemCondition == 'm' && gemColour == 's')
            {
                gemValue = 0x14;
            }
            break;
        }

        if (gemValue != 0)
        {
            data[8] |= (gemValue << 4);
            data[9] |= (gemValue >> 4 & 0x0F) | 0x10;
            return true;
        }

        return false;

    case EnumItemVersion::v104: // v1.04 - v1.06
        code = 0x20;
        code <<= 8;
        code += newgem[2];
        code <<= 8;
        code += newgem[1];
        code <<= 8;
        code += newgem[0];
        code <<= 2;

        data[10] = std::uint8_t(code & 0x000000FF);
        data[11] = std::uint8_t(code >> 8);
        data[12] = std::uint8_t(code >> 16);
        data[13] = std::uint8_t(code >> 24);
        return true;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
        code = *((std::uint32_t*)newgem);
        updateBits64(type_code_offset, 32, code);
        if (isPotion)
        {
            set_bit(is_potion_bit_offset);
        }
        else
        {
            clear_bit(is_potion_bit_offset);
        }
        return true;

    case EnumItemVersion::v115: // v1.15 normal or expansion
        encodeItemCodev115(newgem, code, numBitsSet);
        updateItemCodev115(code, numBitsSet);
        // TODO: what happens if number of bits changes?
        if (isPotion)
        {
            set_bit(is_potion_bit_offset);
        }
        else
        {
            clear_bit(is_potion_bit_offset);
        }
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::socketedItemCount() const
{
    return (std::uint8_t)read_uint32_bits(extended_data_offset, (isSimpleItem() ? 1 : 3));
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Item::Id() const
{
    if (isSimpleItem())
    {
        return 0;
    }

    if (item_id_bit_offset == 0)
    {
        item_id_bit_offset = start_bit_offset + 95;
    }

    return (std::uint32_t)(read_uint64_bits(item_id_bit_offset, 32) & 0xFFFF);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::Level() const
{
    if (isSimpleItem())
    {
        return 0;
    }

    if (item_level_bit_offset == 0)
    {
        item_level_bit_offset = start_bit_offset + 127;
    }

    return (std::uint8_t)read_uint32_bits(item_level_bit_offset + 127, 7);
}
//---------------------------------------------------------------------------
d2ce::EnumItemQuality d2ce::Item::Quality() const
{
    if (isSimpleItem())
    {
        return EnumItemQuality::UKNOWN;
    }

    if (quality_bit_offset == 0)
    {
        quality_bit_offset = start_bit_offset + 134;
    }

    auto value = read_uint32_bits(quality_bit_offset, 4);
    if (value > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
    {
        return EnumItemQuality::UKNOWN;
    }

    return static_cast<EnumItemQuality>(value);
}
//---------------------------------------------------------------------------
bool d2ce::Item::hasMultipleGraphics() const
{
    if (isSimpleItem())
    {
        return false;
    }

    if (multi_graphic_bit_offset == 0)
    {
        multi_graphic_bit_offset = start_bit_offset + 138;
    }

    return read_uint32_bits(multi_graphic_bit_offset, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isAutoAffix() const
{
    if (isSimpleItem())
    {
        return false;
    }

    if (autoAffix_bit_offset == 0)
    {
        autoAffix_bit_offset = start_bit_offset + 139;
        if (hasMultipleGraphics())
        {
            autoAffix_bit_offset += 3;
        }
    }

    return read_uint32_bits(autoAffix_bit_offset, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isArmor() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::uint8_t strcode[4];
    if (calculateItemCode(strcode))
    {
        base = getItemBase(strcode);
    }

    return (base & 4) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isWeapon() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::uint8_t strcode[4];
    if (calculateItemCode(strcode))
    {
        base = getItemBase(strcode);
    }

    return (base & 2) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isTome() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::uint8_t strcode[4];
    if (calculateItemCode(strcode))
    {
        base = getItemBase(strcode);
    }

    return (base & 8) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isStackable() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::uint8_t strcode[4];
    if (calculateItemCode(strcode))
    {
        base = getItemBase(strcode);
    }

    return (base & 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::totalNumberOfSockets() const
{
    if (isSimpleItem() || (socket_count_bit_offset == 0))
    {
        return false;
    }

    return (std::uint8_t)read_uint32_bits(socket_count_bit_offset, 4);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getQuantity() const
{
    if (!isStackable() || stackable_bit_offset == 0)
    {
        return 0;
    }

    return (std::uint8_t)read_uint32_bits(stackable_bit_offset, 9);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setQuantity(std::uint16_t quantity)
{
    if (!isStackable() || stackable_bit_offset == 0)
    {
        return false;
    }

    quantity = std::min(quantity, MAX_STACKED_QUANTITY);

    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    calculateItemCode(strcode);
    switch (gem)
    {
    case 't': // Town portal book
    case 'i': // Identify scroll book
        if (gemcondition == 'b' && gemcolour == 'k')
        {
            // can hold up to 20 scrolls
            quantity = std::min(quantity, MAX_TOME_QUANTITY);
        }
        break;
    case 'k':
        if (gemcondition == 'e' && gemcolour == 'y')
        {
            // only 12 keys can be sacked at need at least 1
            quantity = std::max(std::uint16_t(1), std::min(quantity, MAX_KEY_QUANTITY));
        }
        break;
    }  // end switch

    return updateBits(stackable_bit_offset, 9, quantity);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getMaxDurability() const
{
    if (durability_bit_offset == 0)
    {
        return 0;
    }

    return (std::uint16_t)read_uint32_bits(durability_bit_offset, 8);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMaxDurability(std::uint8_t durability)
{
    if (durability_bit_offset == 0 || getMaxDurability() == 0)
    {
        return false;
    }

    if (!updateBits(durability_bit_offset, 8, durability))
    {
        return false;
    }

    // Also update the current durability
    return updateBits(durability_bit_offset + 8, 8, durability);
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::Item::readBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
{
    size_t readOffset = current_bit_offset;

    // Ensure we read enough
    if (!skipBits(charfile, current_bit_offset, bits))
    {
        return 0;
    }

    if (bits > 64)
    {
        return 0;
    }

    return read_uint64_bits(readOffset, bits);
}
//---------------------------------------------------------------------------
bool d2ce::Item::skipBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
{
    if (feof(charfile))
    {
        return false;
    }

    size_t bytesRequired = (current_bit_offset + bits + 7) / 8;
    std::uint8_t value = 0;
    while (data.size() < bytesRequired)
    {
        if (feof(charfile))
        {
            return false;
        }

        std::fread(&value, sizeof(value), 1, charfile);
        data.push_back(value);
    }

    current_bit_offset += bits;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::readItem(EnumCharVersion version, std::FILE* charfile)
{
    FileVersion = version;
    data.clear();
    SocketedItems.clear();

    // reserve enough space to reduce chance of reallocation (haven't seen an item size bigger then 80
    data.reserve(80);

    start_bit_offset = 0;
    size_t current_bit_offset = start_bit_offset;
    if (FileVersion < EnumCharVersion::v115)
    {
        std::uint8_t value = (std::uint8_t)readBits(charfile, current_bit_offset, 8);
        if (value != ITEM_MARKER[0])
        {
            // corrupt file
            return false;
        }

        value = (std::uint8_t)readBits(charfile, current_bit_offset, 8);
        if (value != ITEM_MARKER[1])
        {
            // corrupt file
            return false;
        }

        start_bit_offset = current_bit_offset;

        // At least the Simple/Compact size is required
        if (!skipBits(charfile, current_bit_offset, 95))
        {
            return false;
        }

        is_potion_bit_offset = 26;
        location_bit_offset = 58;
        type_code_offset = 76;
        extended_data_offset = 108;
    }
    else
    {
        // flags
        if (!skipBits(charfile, current_bit_offset, 32))
        {
            return false;
        }

        is_potion_bit_offset = 10;

        // version
        if (!skipBits(charfile, current_bit_offset, 3))
        {
            return false;
        }

        // Location
        location_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 18))
        {
            return false;
        }

        if (isEar())
        {
            if (!skipBits(charfile, current_bit_offset, 10))
            {
                return false;
            }

            // up to 15 7 bit characters
            for (std::uint8_t idx = 0; !feof(charfile) && idx < 15 && readBits(charfile, current_bit_offset, 7) != 0; ++idx);
        }
        else
        {
            type_code_offset = current_bit_offset;
            for (size_t i = 0; i < 4; ++i)
            {
                if (getEncodedChar(charfile, current_bit_offset) == 0xFF)
                {
                    return false;
                }
            }

            extended_data_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, isSimpleItem() ? 1 : 3))
            {
                return false;
            }
        }
    }

    if (isSimpleItem())
    {
        item_end_bit_offset = current_bit_offset;
        return true;
    }

    item_id_bit_offset = current_bit_offset;
    if (!skipBits(charfile, current_bit_offset, 32))
    {
        return false;
    }

    item_level_bit_offset = current_bit_offset;
    if (!skipBits(charfile, current_bit_offset, 7))
    {
        return false;
    }

    quality_bit_offset = current_bit_offset;
    auto qualityval = readBits(charfile, current_bit_offset, 4);
    if (qualityval > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
    {
        return false;
    }

    auto quality = static_cast<EnumItemQuality>(qualityval);

    // If this is TRUE, it means the item has more than one picture associated with it.
    multi_graphic_bit_offset = current_bit_offset;
    if (readBits(charfile, current_bit_offset, 1) != 0)
    {
        // The next 3 bits contain the picture ID
        if (!skipBits(charfile, current_bit_offset, 3))
        {
            return false;
        }
    }

    // If this is TRUE, it means the item is class specific.
    autoAffix_bit_offset = current_bit_offset;
    if (readBits(charfile, current_bit_offset, 1) != 0)
    {
        // If the item is class specific, the next 11 bits will
        // contain the class specific data.
        if (!skipBits(charfile, current_bit_offset, 11))
        {
            return false;
        }
    }

    switch (quality)
    {
    case EnumItemQuality::INFERIOR:
    case EnumItemQuality::SUPERIOR:
        if (!skipBits(charfile, current_bit_offset, 3))
        {
            return false;
        }
        break;

    case EnumItemQuality::MAGIC:
        if (!skipBits(charfile, current_bit_offset, 22))
        {
            return false;
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        if (!skipBits(charfile, current_bit_offset, 16))
        {
            return false;
        }

        // Following the name IDs, we got 6 possible magical affixes, the pattern
        // is 1 bit id, 11 bit value... But the value will only exist if the prefix
        // is 1. So we'll read the id first and check it against 1.
        for (size_t i = 3; i > 0; --i)
        {
            if (readBits(charfile, current_bit_offset, 1) != 0)
            {
                if (!skipBits(charfile, current_bit_offset, 11))
                {
                    return false;
                }
            }

            if (feof(charfile))
            {
                return false;
            }

            if (readBits(charfile, current_bit_offset, 1) != 0)
            {
                if (!skipBits(charfile, current_bit_offset, 11))
                {
                    return false;
                }
            }

            if (feof(charfile))
            {
                return false;
            }
        }
        break;

    case EnumItemQuality::SET:
    case EnumItemQuality::UNIQUE:
        if (!skipBits(charfile, current_bit_offset, 12))
        {
            return false;
        }
        break;
    }

    if (isRuneword())
    {
        if (!skipBits(charfile, current_bit_offset, 16))
        {
            return false;
        }
    }

    if (isPersonalized())
    {
        personalized_bit_offset = current_bit_offset;

        // up to 15 7 bit characters
        for (std::uint8_t idx = 0; !feof(charfile) && idx < 15 && readBits(charfile, current_bit_offset, 7) != 0; ++idx);
    }

    std::uint8_t base = 0;
    std::uint8_t strcode[4];
    if (calculateItemCode(strcode))
    {
        base = getItemBase(strcode);
    }

    bool isArmor = (base & 4) != 0 ? true : false;
    bool isWeapon = (base & 2) != 0 ? true : false;
    bool isTome = (base & 8) != 0 ? true : false;
    bool isStackable = (base & 1) != 0 ? true : false;

    if (isTome)
    {
        // If the item is a tome, it will contain 5 extra bits, we're not
        // interested in these bits, the value is usually 1, but not sure
        // what it is.
        if (!skipBits(charfile, current_bit_offset, 5))
        {
            return false;
        }
    }

    // Realm Data Flag
    if (readBits(charfile, current_bit_offset, 1) != 0)
    {
        if (!skipBits(charfile, current_bit_offset, 96))
        {
            return false;
        }
    }

    if (feof(charfile))
    {
        return false;
    }

    if (isArmor || isWeapon)
    {
        armor_or_weapon_bit_offset = current_bit_offset;
        if (isArmor)
        {
            // Defense rating
            if (!skipBits(charfile, current_bit_offset, 11))
            {
                return false;
            }
        }

        // Some armor/weapons like phase blades don't have durability
        durability_bit_offset = current_bit_offset;
        if (readBits(charfile, current_bit_offset, 8) > 0)
        {
            // current durability value (8 bits + unknown single bit)
            if (!skipBits(charfile, current_bit_offset, 9))
            {
                return false;
            }
        }

        if (feof(charfile))
        {
            return false;
        }
    }

    if (isStackable)
    {
        // If the item is a stacked item, e.g. a javelin or something, these 9
        // bits will contain the quantity.
        stackable_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 9))
        {
            return false;
        }
    }

    if (isSocketed())
    {
        // If the item is socketed, it will contain 4 bits of data which are the
        // number of total sockets the item have, regardless of how many are occupied
        // by an item.
        socket_count_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 4))
        {
            return false;
        }
    }

    // If the item is part of a set, these bit will tell us how many lists
    // of magical properties follow the one regular magical property list.
    std::uint8_t setBonusBits = 0;
    if (quality == EnumItemQuality::SET)
    {
        bonus_bits_bit_offset = current_bit_offset;
        setBonusBits = (std::uint8_t)readBits(charfile, current_bit_offset, 5);
        if (feof(charfile))
        {
            return false;
        }
    }

    // magical properties
    magical_props_bit_offset = current_bit_offset;
    if (!readPropertyList(charfile, current_bit_offset))
    {
        return false;
    }

    if (setBonusBits > 0)
    {
        // Item has more magical property lists due to being a set item
        set_bonus_props_bit_offset = current_bit_offset;
        for (size_t i = 0; i < 5 && setBonusBits > 0; ++i, setBonusBits >>= 1)
        {
            if ((setBonusBits & 0x01) != 0)
            {
                if (!readPropertyList(charfile, current_bit_offset))
                {
                    return false;
                }
            }
        }
    }

    if (isRuneword())
    {
        // runewords have their own list of magical properties
        runeword_props_bit_offset = current_bit_offset;
        if (!readPropertyList(charfile, current_bit_offset))
        {
            return false;
        }
    }

    auto numSocketed = socketedItemCount();
    if (numSocketed > 0)
    {
        SocketedItems.reserve(numSocketed);
        for (std::uint8_t i = 0; !feof(charfile) && i < numSocketed; ++i)
        {
            SocketedItems.resize(SocketedItems.size() + 1);
            auto& childItem = SocketedItems.back();
            if (!childItem.readItem(FileVersion, charfile))
            {
                return false;
            }

            if (childItem.getLocation() != GLUED_IN_SOCKET)
            {
                // corrupt file
                return false;
            }
        }
    }

    item_end_bit_offset = current_bit_offset;
    return true;
}
bool d2ce::Item::writeItem(std::FILE* charfile)
{
    if (data.empty())
    {
        return true;
    }

    if (std::fwrite(&data[0], data.size(), 1, charfile) != 1)
    {
        std::fflush(charfile);
        return false;
    }

    // now write child items
    for (auto& item : SocketedItems)
    {
        if (!item.writeItem(charfile))
        {
            std::fflush(charfile);
            return false;
        }
    }

    std::fflush(charfile);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::readPropertyList(std::FILE* charfile, size_t& current_bit_offset)
{
    std::uint16_t id = (std::uint16_t)readBits(charfile, current_bit_offset, 9);
    if (feof(charfile))
    {
        return false;
    }

    while (id != 0x1FF)
    {
        if (id >= itemStats.size())
        {
            // corrupt file
            return false;
        }

        const ItemStat* stat = &itemStats[id];

        // saveBits being zero or >= 64 is unrecoverably bad, and
        // encode type 4 is only used by stats that were never implemented (time-based stats)
        if (stat->saveBits == 0 || stat->saveBits >= 64 || stat->encode == 4)
        {
            // corrupt file
            return false;
        }

        size_t numParms = 0;
        if (stat->encode == 2)
        {
            if (stat->saveBits != 7 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 3;
        }
        else if (stat->encode == 3)
        {
            if (stat->saveBits != 16 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 4;
        }
        else if (stat->saveParamBits > 0)
        {
            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 2;
        }
        else
        {
            if (!skipBits(charfile, current_bit_offset, stat->saveBits))
            {
                // corrupt file
                return false;
            }

            numParms = 1;
        }

        while (stat->nextInChain && numParms < 4)
        {
            if (stat->nextInChain >= itemStats.size())
            {
                // corrupt file
                return false;
            }

            stat = &itemStats[stat->nextInChain];
            if (stat->saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, stat->saveBits))
            {
                // corrupt file
                return false;
            }
            ++numParms;
        }

        id = (std::uint16_t)readBits(charfile, current_bit_offset, 9);
    }

    return true;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getEncodedChar(std::FILE* charfile, size_t& current_bit_offset)
{
    std::string bitStr;
    size_t startRead = current_bit_offset;
    size_t readOffset = startRead;
    while (bitStr.size() < 9)
    {
        readOffset = startRead;
        std::stringstream ss2;
        ss2 << std::bitset<9>(readBits(charfile, readOffset, bitStr.size() + 1));
        current_bit_offset = readOffset;
        bitStr = ss2.str().substr(8 - bitStr.size());
        auto iter = huffmanDecodeMap.find(bitStr);
        if (iter != huffmanDecodeMap.end())
        {
            return iter->second;
        }
    }

    // something went wrong
    return 0xFF;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateBits(size_t start, std::uint8_t size, std::uint32_t value)
{
    size_t startIdx = start / 8;
    size_t endIdx = (start + size) / 8;
    if (endIdx >= data.size())
    {
        // not enough space
        return false;
    }

    size_t startBit = start % 8;
    size_t numBytes = (endIdx - startIdx) + 1;
    if (numBytes > 5)
    {
        // 32 bit value can't consume more then this many bytes
        return false;
    }

    std::uint64_t mask = (((std::uint64_t)1 << size) - 1) << (startBit);
    std::uint64_t dest = 0;
    std::memcpy((std::uint8_t*)&dest, &data[startIdx], numBytes);
    dest = (dest & ~mask) | (((std::uint64_t)value << (startBit)) & mask);
    std::memcpy(&data[startIdx], (std::uint8_t*)&dest, numBytes);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateBits64(size_t start, std::uint8_t size, std::uint64_t value)
{
    size_t startIdx = start / 8;
    size_t endIdx = (start + size) / 8;
    if (endIdx >= data.size())
    {
        // not enough space
        return false;
    }

    size_t startBit = start % 8;
    size_t numBytes = (endIdx - startIdx) + 1;
    if (numBytes > 8)
    {
        // value is too big for us to handle
        return false;
    }

    std::uint64_t mask = (((std::uint64_t)1 << size) - 1) << (startBit);
    std::uint64_t dest = 0;
    std::memcpy((std::uint8_t*)&dest, &data[startIdx], numBytes);
    dest = (dest & ~mask) | (((std::uint64_t)value << (startBit)) & mask);
    std::memcpy(&data[startIdx], (std::uint8_t*)&dest, numBytes);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateItemCodev115(std::uint64_t code, std::uint8_t numBitsSet)
{
    auto oldNumBitsSet = std::uint8_t(std::int64_t(extended_data_offset) - std::int64_t(type_code_offset));
    if (oldNumBitsSet == numBitsSet)
    {
        // easy change
        return updateBits64(type_code_offset, numBitsSet, code);
    }

    // complex change
    size_t old_current_bit_offset = extended_data_offset;
    size_t bitsToCopy = item_end_bit_offset - extended_data_offset;

    std::int64_t diff = std::int64_t(numBitsSet) - std::int64_t(oldNumBitsSet);
    extended_data_offset += diff;
    item_end_bit_offset += diff;
    size_t current_bit_offset = extended_data_offset;

    // make a copy first
    std::vector<std::uint8_t> oldData(data);

    // truncate the original
    size_t newSize = (item_end_bit_offset + 7) / 8;
    data.resize(newSize, 0);
    updateBits64(type_code_offset, numBitsSet, code); // update the bits

    // now copy the remaining bits
    std::uint32_t value = 0;
    std::uint8_t bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBits(old_current_bit_offset, bits, value);
        current_bit_offset += bits;
        bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        value = 0;
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBits(old_current_bit_offset, bits, 0);
    }

    // update offsets after the extended_data_offset
    if (!isSimpleItem())
    {
        if (item_id_bit_offset != 0)
        {
            item_id_bit_offset += diff;
        }

        if (item_level_bit_offset != 0)
        {
            item_level_bit_offset += diff;
        }

        if (multi_graphic_bit_offset != 0)
        {
            multi_graphic_bit_offset += diff;
        }

        if (autoAffix_bit_offset != 0)
        {
            autoAffix_bit_offset += diff;
        }

        if (quality_bit_offset != 0)
        {
            quality_bit_offset += diff;
        }

        if (personalized_bit_offset != 0)
        {
            personalized_bit_offset += diff;
        }

        if (armor_or_weapon_bit_offset != 0)
        {
            armor_or_weapon_bit_offset += diff;
        }

        if (durability_bit_offset != 0)
        {
            durability_bit_offset += diff;
        }

        if (stackable_bit_offset != 0)
        {
            stackable_bit_offset += diff;
        }

        if (socket_count_bit_offset != 0)
        {
            socket_count_bit_offset += diff;
        }

        if (bonus_bits_bit_offset != 0)
        {
            bonus_bits_bit_offset += diff;
        }

        if (magical_props_bit_offset != 0)
        {
            magical_props_bit_offset += diff;
        }

        if (set_bonus_props_bit_offset != 0)
        {
            set_bonus_props_bit_offset += diff;
        }

        if (runeword_props_bit_offset != 0)
        {
            runeword_props_bit_offset += diff;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::findItems()
{
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();

    std::uint8_t strcode[4] = { 0 };
    const std::uint8_t& gem = strcode[0];
    const std::uint8_t& gemcondition = strcode[1];
    const std::uint8_t& gemcolour = strcode[2];
    for (auto& item : Inventory)
    {
        if (item.isStackable())
        {
            Stackables.push_back(item);

            if (item.isWeapon())
            {
                Weapons.push_back(item);
            }
            continue;
        }
        else if (item.isWeapon())
        {
            Weapons.push_back(item);
            continue;
        }
        else if (item.isArmor())
        {
            Armor.push_back(item);
            continue;
        }
        else if (!item.isSimpleItem())
        {
            continue;
        }

        item.calculateItemCode(strcode);
        switch (gem)
        {
        case 'g':
            switch (gemcondition)
            {
            case 'c': // chipped
            case 'f': // flawed
            case 's': // regular
            case 'l': // flawless
            case 'p': // perfect
                switch (gemcolour)
                {
                case 'v': // amethyst
                    if (gemcondition != 'l')
                    {
                        GPSs.push_back(item);
                    }
                    break;

                case 'w': // diamond
                case 'g': // emerald
                case 'r': // ruby
                case 'b': // sapphire
                case 'y': // topaz
                    GPSs.push_back(item);
                    break;
                }
                break;

            case 'z': // flawless amethyst
                if (gemcolour == 'v')
                {
                    GPSs.push_back(item);
                }
                break;
            }
            break;

        case 's': // skulls
            if (gemcondition == 'k')
            {
                switch (gemcolour)
                {
                case 'c': // chipped
                case 'f': // flawed
                case 'u': // regular
                case 'l': // flawless
                case 'z': // perfect
                    GPSs.push_back(item);
                    break;
                }
            }
            break;

        case 'r': 
            if (gemcondition == 'v' && // rejuvenation potions
                (gemcolour == 'l' || gemcolour == 's'))
            {
                GPSs.push_back(item);
            }
            else
            {
                switch (gemcondition)
                {
                case '0': // El Rune - Ort Rune
                    if (gemcolour >= '1' && gemcolour <= '9')
                    {
                        GPSs.push_back(item);
                    }
                    break;

                case '1': // Thul Rune - Fal Rune
                case '2': // Lem Rune - Sur Rune
                    if (gemcolour >= '0' && gemcolour <= '9')
                    {
                        GPSs.push_back(item);
                    }
                    break;

                case '3': // Ber Rune - Zod Rune
                    if (gemcolour >= '0' && gemcolour <= '3')
                    {
                        GPSs.push_back(item);
                    }
                    break;
                }
            }
            break;

        case 'h': // healing potions
        case 'm': // mana potions
            if (gemcondition == 'p' &&
                gemcolour >= '1' && gemcolour <= '5')
            {
                GPSs.push_back(item);
            }
            break;

        case 'v': // stamina potions
        case 'y': // antidote potions
            if (gemcondition == 'p' && gemcolour == 's')
            {
                GPSs.push_back(item);
            }
            break;

        case 'w': // thawing potions
            if (gemcondition == 'm' && gemcolour == 's')
            {
                GPSs.push_back(item);
            }
            break;

        default:
            if (item.isStackable())
            {
                Stackables.push_back(item);

                if (item.isWeapon())
                {
                    Weapons.push_back(item);
                }
            }
            else if (item.isWeapon())
            {
                Weapons.push_back(item);
            }
            else if (item.isArmor())
            {
                Armor.push_back(item);
            }
            break;
        }
    } // end for
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(std::FILE* charfile, std::uint32_t& location, std::uint16_t& numItems, std::vector<d2ce::Item>& items)
{
    numItems = 0;
    items.clear();
    if (update_locations)
    {
        // find items location
        location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)MIN_START_STATS_POS)
        {
            cur_pos = MIN_START_STATS_POS;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[1])
            {
                continue;
            }

            // found item marker
            std::fread(&numItems, sizeof(numItems), 1, charfile);
            location = std::ftell(charfile);
            break;
        }

        if (location == 0)
        {
            return false;
        }
    }
    else
    {
        if (location == 0)
        {
            return false;
        }

        std::fseek(charfile, location - sizeof(numItems), SEEK_SET);
        std::fread(&numItems, sizeof(numItems), 1, charfile);
    }

    if (numItems > 0)
    {
        items.reserve(numItems);
        if (!fillItemsArray(charfile, location, numItems, items))
        {
            // Corrupt file
            if (update_locations)
            {
                location = 0;
            }
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::fillItemsArray(std::FILE* charfile, std::uint32_t location, std::uint16_t numItems, std::vector<d2ce::Item>& items)
{
    std::fseek(charfile, location, SEEK_SET);
    while (items.size() < numItems)
    {
        if (feof(charfile))
        {
            return false;
        }

        items.resize(items.size() + 1);
        auto& item = items.back();
        if (!item.readItem(Version, charfile))
        {
            // corrupt file
            items.pop_back();
            return false;
        }
    }  // end while

    return numItems == items.size() ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readCorpseItems(std::FILE* charfile)
{
    return readItems(charfile, corpse_location, NumOfCorpseItems, CorpseItems);
}
//---------------------------------------------------------------------------
void d2ce::Items::readMercItems(std::FILE* charfile)
{
    if (update_locations)
    {
        bool bFoundMercMarker = false;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)MIN_START_STATS_POS)
        {
            cur_pos = MIN_START_STATS_POS;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != MERC_ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != MERC_ITEM_MARKER[1])
            {
                continue;
            }

            // found merc Item marker
            bFoundMercMarker = true;
            break;
        }

        if (!bFoundMercMarker)
        {
            return;
        }
    }

    readItems(charfile, merc_location, NumOfMercItems, MercItems);
    if (merc_location == 0)
    {
        return;
    }

    readGolemItem(charfile);
}
//---------------------------------------------------------------------------
void d2ce::Items::readGolemItem(std::FILE* charfile)
{
    HasGolem = 0;
    GolemItem.clear();
    if (update_locations)
    {
        // find items location
        golem_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)items_location)
        {
            cur_pos = items_location;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != GOLEM_ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != GOLEM_ITEM_MARKER[1])
            {
                continue;
            }

            // found golem item marker (0x464B). 
            std::fread(&HasGolem, sizeof(HasGolem), 1, charfile);
            golem_location = std::ftell(charfile);
            break;
        }

        if (golem_location == 0)
        {
            return;
        }
    }
    else
    {
        if (golem_location == 0)
        {
            return;
        }

        std::fseek(charfile, golem_location - sizeof(HasGolem), SEEK_SET);
        std::fread(&HasGolem, sizeof(HasGolem), 1, charfile);
    }

    if (HasGolem != 0)
    {
        if (!GolemItem.readItem(Version, charfile))
        {
            // Corrupt file
            if (update_locations)
            {
                golem_location = 0;
            }

            GolemItem.clear();
            HasGolem = false;
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeCorpseItems(std::FILE* charfile)
{
    std::fwrite(ITEM_MARKER, sizeof(ITEM_MARKER), 1, charfile);
    NumOfCorpseItems = (std::uint16_t)CorpseItems.size();
    std::fwrite(&NumOfCorpseItems, sizeof(NumOfCorpseItems), 1, charfile);
    corpse_location = std::ftell(charfile);
    for (auto& item : CorpseItems)
    {
        if (!item.writeItem(charfile))
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeMercItems(std::FILE* charfile)
{
    std::fwrite(MERC_ITEM_MARKER, sizeof(MERC_ITEM_MARKER), 1, charfile);
    std::fwrite(ITEM_MARKER, sizeof(ITEM_MARKER), 1, charfile);
    NumOfMercItems = (std::uint16_t)MercItems.size();
    std::fwrite(&NumOfMercItems, sizeof(NumOfMercItems), 1, charfile);
    merc_location = std::ftell(charfile);
    for (auto& item : MercItems)
    {
        if (!item.writeItem(charfile))
        {
            return false;
        }
    }

    return writeGolemItem(charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeGolemItem(std::FILE* charfile)
{
    std::fwrite(GOLEM_ITEM_MARKER, sizeof(GOLEM_ITEM_MARKER), 1, charfile);
    HasGolem = GolemItem.size() > 0 ? 1 : 0;
    std::fwrite(&HasGolem, sizeof(HasGolem), 1, charfile);
    golem_location = std::ftell(charfile);
    if (!GolemItem.writeItem(charfile))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(EnumCharVersion version, std::FILE* charfile, bool isExpansion)
{
    Version = version;
    update_locations = items_location == 0 ? true : false;
    isFileExpansionCharacter = isExpansion;
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    if (!readItems(charfile, items_location, NumOfItems, Inventory) || items_location == 0)
    {
        return false;
    }

    if (!readCorpseItems(charfile) || corpse_location == 0)
    {
        return false;
    }

    if (isExpansion)
    {
        readMercItems(charfile);
    }

    update_locations = false;
    findItems();
    return true;
}
//---------------------------------------------------------------------------
// write items in place at offset saved from reasding
bool d2ce::Items::writeItems(std::FILE* charfile, bool isExpansion)
{
    // Write Items
    std::fwrite(ITEM_MARKER, sizeof(ITEM_MARKER), 1, charfile);
    NumOfItems = (std::uint16_t)Inventory.size();
    std::fwrite(&NumOfItems, sizeof(NumOfItems), 1, charfile);
    for (auto& item : Inventory)
    {
        if (!item.writeItem(charfile))
        {
            return false;
        }
    }

    // Write Corpse Items
    if (!writeCorpseItems(charfile))
    {
        return false;
    }

    if (isExpansion)
    {
        if (!writeMercItems(charfile))
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
d2ce::Items::Items()
{
}
//---------------------------------------------------------------------------
d2ce::Items::Items(const Items& other)
{
    *this = other;
}
//---------------------------------------------------------------------------
d2ce::Items::~Items()
{
}
//---------------------------------------------------------------------------
d2ce::Items& d2ce::Items::operator=(const Items& other)
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    Version = other.Version;

    items_location = other.items_location;
    corpse_location = other.corpse_location;
    merc_location = other.merc_location;
    golem_location = other.golem_location;

    NumOfItems = other.NumOfItems;
    Inventory = other.Inventory;

    NumOfCorpseItems = other.NumOfCorpseItems;
    CorpseItems = other.CorpseItems;

    NumOfMercItems = other.NumOfMercItems;
    MercItems = other.MercItems;

    HasGolem = other.HasGolem;
    GolemItem = other.GolemItem;

    update_locations = other.update_locations;
    isFileExpansionCharacter = other.isFileExpansionCharacter;

    // refetch references to items
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    findItems();
    return *this;
}
d2ce::Items& d2ce::Items::operator=(Items&& other) noexcept
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    Version = std::exchange(other.Version, APP_CHAR_VERSION);

    items_location = std::exchange(other.items_location, 0);
    corpse_location = std::exchange(other.corpse_location, 0);
    merc_location = std::exchange(other.merc_location, 0);
    golem_location = std::exchange(other.golem_location, 0);

    // copy reference to items
    GPSs.swap(other.GPSs);
    other.Inventory.clear();
    Stackables.swap(other.Stackables);
    other.Stackables.clear();
    Armor.swap(other.Armor);
    other.Armor.clear();
    Weapons.swap(other.Weapons);
    other.Weapons.clear();

    NumOfItems = std::exchange(other.NumOfItems, std::uint16_t(0));
    Inventory.swap(other.Inventory);
    other.Inventory.clear();

    NumOfCorpseItems = std::exchange(other.NumOfCorpseItems, std::uint16_t(0));
    CorpseItems.swap(other.CorpseItems);
    other.CorpseItems.clear();

    NumOfMercItems = std::exchange(other.NumOfMercItems, std::uint16_t(0));
    MercItems.swap(other.MercItems);
    other.MercItems.clear();

    HasGolem = std::exchange(other.HasGolem, std::uint8_t(0));
    GolemItem.swap(other.GolemItem);
    other.GolemItem.clear();

    update_locations = std::exchange(other.update_locations, true);
    isFileExpansionCharacter = std::exchange(other.isFileExpansionCharacter, false);
    return *this;
}
//---------------------------------------------------------------------------
void d2ce::Items::swap(Items& other)
{
    std::swap(*this, other);
}
//---------------------------------------------------------------------------
void d2ce::Items::clear()
{
    *this = Items();
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-perfect gems.
   Returns false if no gems or all gems are perfect.
*/
bool d2ce::Items::anyUpgradableGems() const
{
    if (GPSs.empty())
    {
        return 0;
    }

    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    for (auto& item : GPSs)
    {
        item.get().calculateItemCode(strcode);
        switch (gem)
        {
        case 'g':
            switch (gemcondition)
            {
            case 'c': // chipped
            case 'f': // flawed
            case 's': // regular
            case 'l': // flawless
                switch (gemcolour)
                {
                case 'v': // amethyst
                    if (gemcondition != 'l')
                    {
                        return true;
                    }
                    break;

                case 'w': // diamond
                case 'g': // emerald
                case 'r': // ruby
                case 'b': // sapphire
                case 'y': // topaz
                    return true;
                }
                break;

            case 'z': // flawless amethyst
                if (gemcolour == 'v')
                {
                    return true;
                }
                break;
            }
            break;

        case 's': // skulls
            if (gemcondition == 'k')
            {
                switch (gemcolour)
                {
                case 'c': // chipped
                case 'f': // flawed
                case 'u': // regular
                case 'l': // flawless
                    return true;
                    break;
                }
            }
            break;
        }  // end switch
    }  // end for

    return false;
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-Super Health/Mana potions or
                non-Full Rejuvenation potions.
   Returns false if no potions or all potions are the top kind.
*/
bool d2ce::Items::anyUpgradablePotions() const
{
    if (GPSs.empty())
    {
        return false;
    }

    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    for (auto& item : GPSs)
    {
        item.get().calculateItemCode(strcode);
        switch (gem)
        {
        case 'r': // rejuvenation potions
            if (gemcondition == 'v' && gemcolour == 's')
            {
                return true;
            }
            break;

        case 'h': // healing potions
        case 'm': // mana potions
            if (gemcondition == 'p' &&
                gemcolour >= '1' && gemcolour <= '4')
            {
                return true;
            }
            break;
        }  // end switch
    }  // end for

    return false;
}
//---------------------------------------------------------------------------
/*
   Returns true if there are any non-Full Rejuvenation potions.
   Returns false if no potions or all potions Full Rejuvenation potions.
*/
bool d2ce::Items::anyUpgradableRejuvenations() const
{
    if (GPSs.empty())
    {
        return false;
    }

    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    for (auto& item : GPSs)
    {
        item.get().calculateItemCode(strcode);
        switch (gem)
        {
        case 'r': // rejuvenation potions
            if (gemcondition == 'v' && gemcolour == 's')
            {
                return true;
            }
            break;

        case 'h': // healing potions
        case 'm': // mana potions
            if (gemcondition == 'p' &&
                gemcolour >= '1' && gemcolour <= '5')
            {
                return true;
            }
            break;
        }  // end switch
    }  // end for

    return false;
}
//---------------------------------------------------------------------------
/*
   Returns the number of items in character's inventory.
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Items::getNumberOfItems() const
{
    return Inventory.size();
}
//---------------------------------------------------------------------------
// number of bytes to store all item sections
size_t d2ce::Items::getByteSize() const
{
    size_t byteSize = GolemItem.getFullSize();
    for (auto& item : Inventory)
    {
        byteSize += item.getFullSize();
    }

    for (auto& item : CorpseItems)
    {
        byteSize += item.getFullSize();
    }

    for (auto& item : MercItems)
    {
        byteSize += item.getFullSize();
    }

    return byteSize;
}
//---------------------------------------------------------------------------
/*
   Returns the number of Gems, Potions or Skulls in character's inventory.
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Items::getNumberOfGPSs()
{
    return GPSs.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getGPSs()
{
    return GPSs;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfStackables()
{
    return Stackables.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getStackables()
{
    return Stackables;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfArmor()
{
    return Armor.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getArmor()
{
    return Armor;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfWeapons()
{
    return Weapons.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getWeapons()
{
    return Weapons;
}
//---------------------------------------------------------------------------
/*
   Converts the all gems to their perfect state
   Returns the number of gems converted.
*/
size_t d2ce::Items::upgradeGems()
{
    if (GPSs.empty())
    {
        return 0;
    }

    size_t gemsconverted = 0;
    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    strcode[3] = 0x20;
    for (auto& item : GPSs)
    {
        item.get().calculateItemCode(strcode);
        switch (gem)
        {
        case 'g':
            switch (gemcondition)
            {
            case 'c': // chipped
            case 'f': // flawed
            case 's': // regular
            case 'l': // flawless
                switch (gemcolour)
                {
                case 'v': // amethyst
                    if (gemcondition != 'l')
                    {
                        gemcondition = 'p'; // perfect
                        if (item.get().updateGem(strcode))
                        {
                            ++gemsconverted;
                        }
                    }
                    break;

                case 'w': // diamond
                case 'g': // emerald
                case 'r': // ruby
                case 'b': // sapphire
                case 'y': // topaz
                    gemcondition = 'p'; // perfect
                    if (item.get().updateGem(strcode))
                    {
                        ++gemsconverted;
                    }
                    break;
                }
                break;

            case 'z': // flawless amethyst
                if (gemcolour == 'v')
                {
                    gemcondition = 'p'; // perfect
                    if (item.get().updateGem(strcode))
                    {
                        ++gemsconverted;
                    }
                }
                break;
            }
            break;

        case 's': // skulls
            if (gemcondition == 'k')
            {
                switch (gemcolour)
                {
                case 'c': // chipped
                case 'f': // flawed
                case 'u': // regular
                case 'l': // flawless
                    gemcolour = 'z'; // perfect
                    if (item.get().updateGem(strcode))
                    {
                        ++gemsconverted;
                    }
                    break;
                }
            }
            break;
        }  // end switch
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to their highest quiality.
   Returns the number of potions converted.
*/
size_t d2ce::Items::upgradePotions()
{
    if (GPSs.empty())
    {
        return 0;
    }

    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    strcode[3] = 0x20;
    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        item.get().calculateItemCode(strcode);
        switch (gem)
        {
        case 'r': // rejuvenation potions
            if (gemcondition == 'v' && gemcolour == 's')
            {
                gemcolour = 'l';
                if (item.get().updateGem(strcode))
                {
                    ++gemsconverted;
                }
            }
            break;

        case 'h': // healing potions
        case 'm': // mana potions
            if (gemcondition == 'p' &&
                gemcolour >= '1' && gemcolour <= '4')
            {
                gemcolour = '5';
                if (item.get().updateGem(strcode))
                {
                    ++gemsconverted;
                }
            }
            break;
        }  // end switch
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to Full Rejuvenation potions.
   Returns the number of potions converted.
*/
size_t d2ce::Items::upgradeRejuvenationPotions()
{
    if (GPSs.empty())
    {
        return 0;
    }

    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    strcode[3] = 0x20;
    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        item.get().calculateItemCode(strcode);
        switch (gem)
        {
        case 'r': // rejuvenation potions
            if (gemcondition == 'v' && gemcolour == 's')
            {
                gemcolour = 'l';
                if (item.get().updateGem(strcode))
                {
                    ++gemsconverted;
                }
            }
            break;

        case 'h': // healing potions
        case 'm': // mana potions
            if (gemcondition == 'p' &&
                gemcolour >= '1' && gemcolour <= '5')
            {
                gem = 'r';
                gemcondition = 'v';
                gemcolour = 'l';
                if (item.get().updateGem(strcode))
                {
                    ++gemsconverted;
                }
            }
            break;
        }  // end switch
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
/*
   Converts the specified original gems, potions or skulls to the specified 
   final gem, potion or skull.
   Returns the number of gems converted.
*/
size_t d2ce::Items::convertGPSs(const std::uint8_t(&existingGem)[4], const std::uint8_t(&desiredGem)[4])
{
    if (GPSs.empty())
    {
        return 0;
    }

    const std::uint8_t& oldgem = existingGem[0];
    const std::uint8_t& oldgemcondition = existingGem[1];
    const std::uint8_t& oldgemcolour = existingGem[2];

    std::uint8_t temp[4] = { 0 };
    std::uint8_t& currentgem = temp[0];
    std::uint8_t& currentgemcondition = temp[1];
    std::uint8_t& currentgemcolour = temp[2];

    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        item.get().calculateItemCode(temp);
        if (oldgem == currentgem && oldgemcondition == currentgemcondition && oldgemcolour == currentgemcolour)
        {
            // found a match, try to update it
            if (item.get().updateGem(desiredGem))
            {
                ++gemsconverted;
            }
        }
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::fillAllStackables()
{
    size_t itemsFilled = 0;
    for (auto& item : Stackables)
    {
        if (item.get().setQuantity(MAX_STACKED_QUANTITY))
        {
            ++itemsFilled;
        }
    }  // end for

    return itemsFilled;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::maxDurabilityAllItems()
{
    size_t itemsFilled = 0;
    for (auto& item : Armor)
    {
        if (item.get().setMaxDurability(MAX_DURABILITY))
        {
            ++itemsFilled;
        }
    }

    for (auto& item : Weapons)
    {
        if (item.get().setMaxDurability(MAX_DURABILITY))
        {
            ++itemsFilled;
        }
    }

    for (auto& item : MercItems)
    {
        if (item.setMaxDurability(MAX_DURABILITY))
        {
            ++itemsFilled;
        }
    }

    return itemsFilled;
}
//---------------------------------------------------------------------------