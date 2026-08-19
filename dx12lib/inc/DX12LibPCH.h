// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <DirectXMath.h>
#include "Helpers.h"
#include <d3dcompiler.h>
using namespace DirectX;
#include "dx12lib.h"
// STL Headers
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <math.h>
#include <map>
#include <mutex>
#include <new>
#include <unordered_map>
#include <thread>
#include <vector>
#include <memory>
#include <string>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#endif //PCH_H
