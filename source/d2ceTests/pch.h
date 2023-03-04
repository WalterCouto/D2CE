// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define NOMINMAX
#include <algorithm>
using std::max;
using std::min;

#include "windows.h"

#include <cstdint>
#include <locale>
#include <filesystem>
#include "../d2ce/bitmask.hpp"
#include <system_error>
#include <map>
#include <bitset>
#include <sstream>
#include <string>
#include "../d2ce/Character.h"
#include <fstream>

#include <tchar.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#define STORMLIB_LITTLE_ENDIAN
#define    BSWAP_INT16_UNSIGNED(a)          (a)
#define    BSWAP_INT16_SIGNED(a)            (a)
#define    BSWAP_INT32_UNSIGNED(a)          (a)
#define    BSWAP_INT32_SIGNED(a)            (a)
#define    BSWAP_INT64_SIGNED(a)            (a)
#define    BSWAP_INT64_UNSIGNED(a)          (a)
#define    BSWAP_ARRAY16_UNSIGNED(a,b)      {}
#define    BSWAP_ARRAY32_UNSIGNED(a,b)      {}
#define    BSWAP_ARRAY64_UNSIGNED(a,b)      {}
#define    BSWAP_PART_HEADER(a)             {}
#define    BSWAP_TMPQHEADER(a,b)            {}
#define    BSWAP_TMPKHEADER(a)              {}

#ifdef _WIN64
#define STORMLIB_64BIT
#else
#define STORMLIB_32BIT
#endif

#define STORMLIB_CDECL __cdecl

#define STORMLIB_WINDOWS
#define STORMLIB_PLATFORM_DEFINED                 // The platform is known now

#define __STORMLIB_SELF__
#define STORMLIB_NO_AUTO_LINK

#include <wininet.h>

#include "../d2ce/ThirdParty/StormLib/src/StormLib.h"
#include "../d2ce/ThirdParty/StormLib/src/StormCommon.h"

#endif //PCH_H
