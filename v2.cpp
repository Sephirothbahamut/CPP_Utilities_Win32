#include "include/utils_win32/v2.h"

#include <iostream>

class mine : public virtual utils::win32::window::base
	{
	public:
		std::optional<LRESULT> procedure(mine& t, UINT msg, WPARAM wparam, LPARAM lparam)
			{
			std::cout << "yay\n";
			return std::nullopt;
			}
	};
using window = utils::win32::window::t<mine>;


int main()
	{
	try
		{
		window::initializer i;

		window window{window::create_info
			{
			.title{L"Pippo"}
			}};

		while (window.poll_event());
		}
	catch (const std::system_error& e) { ::MessageBoxA(nullptr, e.what(), "Unhandled Exception", MB_OK | MB_ICONERROR); }
	}