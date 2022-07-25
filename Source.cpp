#include "include/window.h"

#include <iostream>
#include <functional>

class vulkan_window : public utils::window<void>
	{
	public:
		vulkan_window(utils::window<void>::initializer_t initializer) : utils::window<void>{initializer}
		{
		procedures.push_back([this](UINT msg, WPARAM wparam, LPARAM lparam) -> std::optional<LRESULT> { return this->callback(msg, wparam, lparam); });

		}

	private:

		std::optional<LRESULT> callback(UINT msg, WPARAM wparam, LPARAM lparam)
			{
			switch (msg)
				{
				case WM_CLOSE:
				{
				std::cout << "Troll";
				return 0;
				}
				}
			return std::nullopt;
			}
	};

int main()
	{

	try
		{
		vulkan_window::initializer i;

		vulkan_window window{vulkan_window::initializer_t
			{
			.title{L"Pippo"}
			}};

		while (window.poll_event());
		}
	catch (const std::system_error& e) { ::MessageBoxA(nullptr, e.what(), "Unhandled Exception", MB_OK | MB_ICONERROR); }
	}