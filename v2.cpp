#include "include/utils_win32/v2.h"

#include <iostream>
// Let it be recorded to history that I wanted to use '🗔' instead of "window" for the window namespace
class mine : public virtual utils::win32::window::base
	{
	public:
		std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam)
			{
			switch (msg)
				{
				case WM_SIZE:
					{
					std::cout << width << ", " << height << "\n";
					}
				}
			return std::nullopt;
			}
	};

class troll_close_button : public virtual utils::win32::window::base
	{
	public:
		std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam)
			{
			switch (msg)
				{
				case WM_SIZE:
					{
					std::cout << "recreate swapchain\n";
					}
				case WM_CLOSE:
					{
					return 0;
					}
				case WM_SYSCOMMAND:
					{
					// Check your window state here
					switch (wparam)
						{
						case SC_MAXIMIZE:
							{
							::DestroyWindow(get_handle());
							return 0;
							}
						case SC_MINIMIZE:
							{
							ShowWindow(get_handle(), SW_MAXIMIZE);
							return 0;
							}
						}
					}
				}
			return std::nullopt;
			}
	};

using window = utils::win32::window::simple_t<mine, troll_close_button>;


int main()
	{
	//try
		{
		window::initializer i;

		window window{window::create_info
			{
			.title{L"Pippo"}
			}};

		while (window.is_open())
			{
			while (window.poll_event());
			}
		}
	//catch (const std::system_error& e) { ::MessageBoxA(nullptr, e.what(), "Unhandled Exception", MB_OK | MB_ICONERROR); }
	}