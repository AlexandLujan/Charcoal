//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif



//#define WINVER 0x0A00
//#define _WIN32_WINNT 0x0A00
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS
//#include <afxwin.h>         // MFC core and standard components


#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <wrl.h>
#include <shellapi.h>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <utility>
