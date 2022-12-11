#pragma once
#include "DWM.h"

namespace utils::win32::window::details::style
	{
	enum class Style : DWORD
		{
		windowed         = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		aero_borderless  = WS_POPUP            | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		basic_borderless = WS_POPUP            | WS_THICKFRAME | WS_SYSMENU              | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
		};

	inline bool composition_enabled() noexcept
		{
		BOOL composition_enabled = FALSE;
		bool success = ::DwmIsCompositionEnabled(&composition_enabled) == S_OK;
		return composition_enabled && success;
		}

	inline Style select_borderless_style() noexcept
		{
		return composition_enabled() ? Style::aero_borderless : Style::basic_borderless;
		}

	inline void set_shadow(HWND handle, bool enabled)
		{
		if (composition_enabled())
			{
			static const MARGINS shadow_state[2]{{ 0,0,0,0 },{ 1,1,1,1 }};
			::DwmExtendFrameIntoClientArea(handle, &shadow_state[enabled]);
			}
		}

	inline auto maximized(HWND hwnd) -> bool
		{
		WINDOWPLACEMENT placement;
		if (!::GetWindowPlacement(hwnd, &placement))
			{
			return false;
			}

		return placement.showCmd == SW_MAXIMIZE;
		}

	inline auto adjust_maximized_client_rect(HWND window, RECT& rect) -> void
		{
		if (!maximized(window))
			{
			return;
			}

		auto monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
		if (!monitor)
			{
			return;
			}

		MONITORINFO monitor_info{};
		monitor_info.cbSize = sizeof(monitor_info);
		if (!::GetMonitorInfoW(monitor, &monitor_info))
			{
			return;
			}

		// when maximized, make the client area fill just the monitor (without task bar) rect,
		// not the whole window rect which extends beyond the monitor.
		rect = monitor_info.rcWork;
		}
	}