#include <utils/enum.h>
#include <iostream>

#include "include/utils/win32/window/window.h"
#include "include/utils/win32/window/style.h"
#include "include/utils/win32/window/taskbar.h"
#include "include/utils/win32/window/regions.h"
#include "include/utils/win32/window/input_system.h"
// Let it be recorded to history that I wanted to use '🗔' instead of "window" for the window namespace

class resize_printer : public virtual utils::win32::window::base
	{
	public:
		std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam)
			{
			switch (msg)
				{
				case WM_ENTERSIZEMOVE:
					std::cout << "entered resizing\n";
					break;
				case WM_EXITSIZEMOVE:
					std::cout << "exited resizing\n";
					break;
				case WM_SIZE:
					std::cout << "Window: " << window_rect.width << ", " << window_rect.height << "\n";
					std::cout << "Client: " << client_rect.width << ", " << client_rect.height << "\n";
					break;
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

struct window : 
	public utils::win32::window::t
		<
		utils::win32::window::style,
		utils::win32::window::resizable_edge,
		utils::win32::window::regions,
		utils::win32::window::taskbar,
		utils::win32::window::input::mouse,
		resize_printer,
		troll_close_button
		>, 
	utils::devirtualize
	{
	struct create_info
		{
		utils::win32::window::base          ::create_info base          ;
		utils::win32::window::style         ::create_info style         ;
		utils::win32::window::resizable_edge::create_info resizable_edge;
		utils::win32::window::regions       ::create_info regions       ;
		};

	window(create_info create_info) : 
		utils::win32::window::base          {create_info.base          },
		utils::win32::window::style         {create_info.style         },
		utils::win32::window::resizable_edge{create_info.resizable_edge},
		utils::win32::window::regions       {create_info.regions       }
		{}
	};
/*/
using window = utils::win32::window::simple_t<>;
/**/
int main()
	{
	//try
		{
		using namespace utils::output;

		window::initializer window_initializer;

		utils::input::mouse default_mouse;
		default_mouse.button_down_actions.emplace([](utils::input::mouse::button button)
			{
			std::cout << "Mouse button down: " << utils::magic_enum::enum_name(button) << "\n";
			});
		default_mouse.button_up_actions.emplace([](utils::input::mouse::button button)
			{
			std::cout << "Mouse button up:   " << utils::magic_enum::enum_name(button) << "\n";
			});
		default_mouse.move_to_actions.emplace([](utils::math::vec2l position)
			{
			std::cout << "Mouse move to:     " << position << "\n";
			});
		default_mouse.move_by_actions.emplace([](utils::math::vec2l delta)
			{
			std::cout << "Mouse move by:     " << delta << "\n";
			});
		
		window window{window::create_info
			{
			.base
				{
				.title{L"Pippo"}
				},
			.style
				{
				.transparency{window::style::transparency_t::none},
				.borders{window::style::value_t::enable},
				.shadow{window::style::value_t::_default}
				},
			.resizable_edge
				{
				.thickness{8}
				},
			.regions
				{
				.default_hit_type{utils::win32::window::hit_type::client}
				}
			}};

		window.mice_ptrs.emplace(&default_mouse);

		while (window.is_open())
			{
			while (window.poll_event());
			//window.set_taskbar_visibility(false);
			}
		}
	//catch (const std::system_error& e) { ::MessageBoxA(nullptr, e.what(), "Unhandled Exception", MB_OK | MB_ICONERROR); }
	}