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
#include "..\d2ce\bitmask.hpp"
#include <system_error>
#include <map>
#include <bitset>
#include <sstream>
#include <string>
#include "..\d2ce\Character.h"
#include <fstream>

#endif //PCH_H
