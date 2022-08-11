/*#include "include/utils_win32/window.h"

#include <iostream>
#include <functional>

// TODO think
//template <typename T>
//concept has_procedure =
//	requires(T t, UINT msg, WPARAM wparam, LPARAM lparam)
//	{
//		{ t.qwe(msg, wparam, lparam) } -> std::same_as<std::optional<LRESULT>>;
//	};
//
//template <std::derived_from<utils::win32::window> ...window_implementation_ts>
//class test_window : virtual public window_implementation_ts...
//	{
//	public:
//		inline test_window(utils::win32::window::initializer_t initializer) : window_implementation_ts{initializer}..., utils::win32::window{initializer} {}
//		static_assert((std::is_base_of<utils::win32::window, window_implementation_ts>::value && ...), "All window implementations must inherit from utils::window.");
//
//		virtual inline LRESULT procedure(UINT msg, WPARAM wparam, LPARAM lparam) noexcept final override
//			{
//
//			std::optional<LRESULT> last_result;
//
//			([&]<typename T>(std::type_identity<T>) 
//				{
//				if constexpr (has_procedure<T>) 
//					{
//					if (auto result{T::qwe(msg, wparam, lparam)})
//						{
//						last_result = result;
//						}
//					}
//				}(std::type_identity<window_implementation_ts>{}), ...);// ??¿?!?¿!?!?!?!?¿
//
//			return last_result ? last_result.value() : DefWindowProc(utils::win32::window::handle, msg, wparam, lparam);
//			}
//
//	private:
//
//	};

/*class vulkan_window : public virtual utils::win32::window
	{
	public:
		vulkan_window(utils::win32::window::create_info create_info) : utils::win32::window{create_info}
		{
		procedures.push_back([this](UINT msg, WPARAM wparam, LPARAM lparam) -> std::optional<LRESULT> { return this->procedure(msg, wparam, lparam); });
		}

	private:

		std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam)
			{
			switch (msg)
				{
				case WM_CLOSE:
					if (true)
						{
						std::cout << "Troll";
						return 0;
						}
				}
			return std::nullopt;
			}
	};*/

//using mytestwindow = test_window<vulkan_window>;

//int main()
//	{
//	/*try
//		{
//		mytestwindow::initializer i;
//
//		mytestwindow window{utils::win32::window::initializer_t
//			{
//			.title{L"Pippo"}
//			}};
//
//		while (window.poll_event());
//		}
//	catch (const std::system_error& e) { ::MessageBoxA(nullptr, e.what(), "Unhandled Exception", MB_OK | MB_ICONERROR); }/*/
//	try
//		{
//		vulkan_window::initializer i;
//
//		vulkan_window window{vulkan_window::initializer_t
//			{
//			.title{L"Pippo"}
//			}};
//
//		while (window.poll_event());
//		}
//	catch (const std::system_error& e) { ::MessageBoxA(nullptr, e.what(), "Unhandled Exception", MB_OK | MB_ICONERROR); }/**/
//	}