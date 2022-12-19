#pragma once

//TODO complete

#include "window.h"

#include "shobjidl_core.h"

namespace utils::win32::window
	{
	enum class progress_state { error, paused, normal, indeterminate };

	/*
1	TBPF_ERROR
2	TBPF_PAUSED
3	TBPF_NORMAL
4	TBPF_INDETERMINATE
	*/


	struct taskbar : virtual utils::win32::window::base
		{
		void set_taskbar_visibility(bool b)
			{
			auto style{GetWindowLongPtr(get_handle(), GWL_STYLE)};
			if (b)
				{
				style |= WS_EX_TOOLWINDOW;
				}
			else
				{
				style &= !WS_EX_TOOLWINDOW;
				}
			SetWindowLongPtr(get_handle(), GWL_STYLE, style);
			}

		void set_taskbar_colour(progress_state progress_state)
			{
			
			/*switch (progress_state)
				{
				case utils::win32::window::progress_state::error:
					ITaskbarList3::SetProgressState(get_handle(), TBPF_ERROR);
					break;
				case utils::win32::window::progress_state::paused:
					SetProgressState(get_handle(), TBPF_ERROR);
					break;
				case utils::win32::window::progress_state::normal:
					SetProgressState(get_handle(), TBPF_ERROR);
					break;
				case utils::win32::window::progress_state::indeterminate:
					SetProgressState(get_handle(), TBPF_ERROR);
					break;
				default:
					break;
				}*/
			}

		void set_taskbar_progress()
			{}
		};
	}