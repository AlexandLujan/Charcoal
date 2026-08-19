#pragma once

#ifdef DX12LIB_API_EXPORTS
#define DX12LIB_API __declspec(dllexport)
#else
#define DX12LIB_API __declspec(dllimport)
#endif