#pragma once

////////////////// NOTE: Your project must include "Dwmapi.lib" and "psapi.lib"

#include "../../windows.h"

#include <WinUser.h>
#include <dwmapi.h>
#pragma comment (lib, "dwmapi.lib")//without this dwmapi.h doesn't work :shrugs: no idea whatsoever where the compiler is taking this file from
#include <tchar.h>
#include <Shlwapi.h>

#pragma region DWM import
typedef enum _WINDOWCOMPOSITIONATTRIB
	{
	WCA_UNDEFINED = 0,
	WCA_NCRENDERING_ENABLED = 1,
	WCA_NCRENDERING_POLICY = 2,
	WCA_TRANSITIONS_FORCEDISABLED = 3,
	WCA_ALLOW_NCPAINT = 4,
	WCA_CAPTION_BUTTON_BOUNDS = 5,
	WCA_NONCLIENT_RTL_LAYOUT = 6,
	WCA_FORCE_ICONIC_REPRESENTATION = 7,
	WCA_EXTENDED_FRAME_BOUNDS = 8,
	WCA_HAS_ICONIC_BITMAP = 9,
	WCA_THEME_ATTRIBUTES = 10,
	WCA_NCRENDERING_EXILED = 11,
	WCA_NCADORNMENTINFO = 12,
	WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
	WCA_VIDEO_OVERLAY_ACTIVE = 14,
	WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
	WCA_DISALLOW_PEEK = 16,
	WCA_CLOAK = 17,
	WCA_CLOAKED = 18,
	WCA_ACCENT_POLICY = 19,
	WCA_FREEZE_REPRESENTATION = 20,
	WCA_EVER_UNCLOAKED = 21,
	WCA_VISUAL_OWNER = 22,
	WCA_HOLOGRAPHIC = 23,
	WCA_EXCLUDED_FROM_DDA = 24,
	WCA_PASSIVEUPDATEMODE = 25,
	WCA_USEDARKMODECOLORS = 26,
	WCA_CORNER_STYLE = 27,
	WCA_PART_COLOR = 28,
	WCA_DISABLE_MOVESIZE_FEEDBACK = 29,
	WCA_LAST = 30
	} WINDOWCOMPOSITIONATTRIB;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA
	{
	WINDOWCOMPOSITIONATTRIB Attrib;
	PVOID pvData;
	SIZE_T cbData;
	} WINDOWCOMPOSITIONATTRIBDATA;

typedef enum _ACCENT_STATE
	{
	ACCENT_DISABLED = 0,
	ACCENT_ENABLE_GRADIENT = 1,
	ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
	ACCENT_ENABLE_BLURBEHIND = 3,
	ACCENT_ENABLE_ACRYLICBLURBEHIND = 4, // RS4 1803
	ACCENT_ENABLE_HOSTBACKDROP = 5, // RS5 1809
	ACCENT_INVALID_STATE = 6
	} ACCENT_STATE;

typedef struct _ACCENT_POLICY
	{
	ACCENT_STATE AccentState;
	DWORD AccentFlags;
	DWORD GradientColor;
	DWORD AnimationId;
	} ACCENT_POLICY;

typedef BOOL(WINAPI* pfnGetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

typedef BOOL(WINAPI* pfnSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
#pragma endregion DWM import

namespace utils::window
	{
	inline bool make_glass_CompositionAttribute(HWND hwnd)
		{
		if (HMODULE hUser = GetModuleHandleA("user32.dll"))
			{
			//Windows >= 10
			pfnSetWindowCompositionAttribute SetWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
			if (SetWindowCompositionAttribute)
				{
				ACCENT_POLICY accent = {ACCENT_ENABLE_BLURBEHIND, 0, 0, 0};

				WINDOWCOMPOSITIONATTRIBDATA data;
				data.Attrib = WCA_ACCENT_POLICY;
				data.pvData = &accent;
				data.cbData = sizeof(accent);
				SetWindowCompositionAttribute(hwnd, &data);
				return true;
				}
			}
		return false;
		}
	inline bool make_glass_DWM_BlurBehind(HWND hwnd)
		{
		DWM_BLURBEHIND bb = {0};
		bb.dwFlags = DWM_BB_ENABLE;
		bb.fEnable = true;
		bb.hRgnBlur = NULL;
		HRESULT result = DwmEnableBlurBehindWindow(hwnd, &bb);
		return result == S_OK;
		}
	inline bool make_glass_DWM_margin(HWND hwnd)
		{
		// Negative margins have special meaning to DwmExtendFrameIntoClientArea.
		// Negative margins create the "sheet of glass" effect, where the client area
		// is rendered as a solid surface with no window border.
		MARGINS margins = {-1};
		HRESULT hr = S_OK;

		// Extend the frame across the entire window.
		hr = DwmExtendFrameIntoClientArea(hwnd, &margins);
		return (SUCCEEDED(hr));
		}

	inline bool make_transparent_Layered(HWND hWnd, BYTE opacity = 127)
		{
		//Transparent window
		SetWindowLong(hWnd, GWL_EXSTYLE,
			GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

		//HBRUSH brush = CreateSolidBrush(RGB(0, 0, 255));
		//SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG)brush);

		return SetLayeredWindowAttributes(hWnd, 0, opacity, LWA_ALPHA);
		}
	}
