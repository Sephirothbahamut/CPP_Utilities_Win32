#pragma once

#ifdef _WIN32

#ifdef NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <conio.h>
#else
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <conio.h>
#undef NOMINMAX
#endif

#else
#error "CPP_Utilities_Win32 only supports Microsoft Windows"
#endif