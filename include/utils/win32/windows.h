#pragma once

#ifdef _WIN32

#ifdef NOMINMAX
	#ifdef WINDOWS_LEAN_AND_MEAN
		#include <windows.h>
		#include <windowsx.h>
		#include <conio.h>
	#else
		#define WINDOWS_LEAN_AND_MEAN
		#include <windows.h>
		#include <windowsx.h>
		#include <conio.h>
		#undef WINDOWS_LEAN_AND_MEAN
	#endif
#else
	#define NOMINMAX
	#ifdef WINDOWS_LEAN_AND_MEAN
		#include <windows.h>
		#include <windowsx.h>
		#include <conio.h>
	#else
		#define WINDOWS_LEAN_AND_MEAN
		#include <windows.h>
		#include <windowsx.h>
		#include <conio.h>
		#undef WINDOWS_LEAN_AND_MEAN
	#endif
	#undef NOMINMAX
#endif

#else
#error "CPP_Utilities_Win32 only supports Microsoft Windows"
#endif