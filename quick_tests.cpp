#include "include/utils_win32/window.h"
#include "include/utils_win32/style.h"
#include "include/utils_win32/regions.h"
#include <iostream>
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
					std::cout << width << ", " << height << "\n";
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
		window::initializer window_initializer;

		
		window window{window::create_info
			{
			.base
				{
				.title{L"Pippo"}
				},
			.style
				{
				.transparency{window::style::transparency_t::composition_attribute},
				.borders{window::style::value_t::disable},
				.shadow{window::style::value_t::_default}
				},
			.resizable_edge
				{
				.thickness{8}
				},
			.regions
				{
				.default_hit_type{utils::win32::window::hit_type::drag}
				}
			}};

		while (window.is_open())
			{
			while (window.poll_event());
			}
		}
	//catch (const std::system_error& e) { ::MessageBoxA(nullptr, e.what(), "Unhandled Exception", MB_OK | MB_ICONERROR); }
	}