#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#define NOMINMAX
#include <algorithm>
using std::max;
using std::min;


#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include <afxdisp.h>        // MFC Automation classes

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

#include <windef.h>
#include <Windows.h>
#include <Tlhelp32.h>

#include <string>
#include <memory>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <bitset>
#include <filesystem>
#include <iostream>
#include <locale>
#include <iostream>
#include <cstdio>
#include <system_error>

#include "d2ce/bitmask.hpp"
#include <gdiplus.h>

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

#include "d2ce/ThirdParty/StormLib/src/StormLib.h"
#include "d2ce/ThirdParty/StormLib/src/StormCommon.h"

#pragma comment(lib, "version.lib")
#pragma comment(lib, "gdiplus.lib")

const UINT WM_OPEN_DLG_FILE = WM_USER + 400;

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


