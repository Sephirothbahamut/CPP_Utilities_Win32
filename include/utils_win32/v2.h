#pragma once
// IMPORTANT: If you include this header, remember to define WIN32_LEAN_AND_MEAN and NOMINMAX

#include <string>
#include <optional>
#include <vector>
#include <functional>

#include <utils/math/vec2.h>
#include <utils/graphics/color.h>
#include <utils/memory.h>
#include <utils/math/vec2.h>

#include <Windows.h>

#include "error_to_exception.h"

namespace utils::win32::window
	{

	/// <summary>
	/// Custom procedures must return std::nullopt for messages that are not being evaluated.
	/// When multiple procedures evaluate the same event, only the last one is returned to the OS API.
	/// </summary>
	using procedure_t = std::function<std::optional<LRESULT>(UINT msg, WPARAM wparam, LPARAM lparam)>;//std::optional<LRESULT>(*)(UINT msg, WPARAM wparam, LPARAM lparam);

	class base;
	template <std::derived_from<base> ...window_implementation_ts>
	class t;

	class base
		{
		template <std::derived_from<base> ...window_implementation_ts>
		friend class t;

		public:
			struct user_data_t { utils::observer_ptr<base> window; void* userdata; };
#pragma region con/de-structors
			inline base(HWND handle = nullptr) : handle{handle} {}
			inline base(const base& copy) = delete;
			inline base& operator=(const base& copy) = delete;
			inline base(base&& move) noexcept
				{
				handle = move.handle;
				move.handle = nullptr;
				set_window_ptr();
				}
			inline base& operator=(base&& move) noexcept
				{
				handle = move.handle;
				move.handle = nullptr;
				set_window_ptr();
				}
			inline ~base() noexcept { if (handle) { ::DestroyWindow(handle); } }

#pragma endregion con/de-structors

			inline void set_user_data(void* data) noexcept { user_data.userdata = data; }
			inline void* get_user_data() noexcept { return user_data.userdata; }

			inline bool is_open() const noexcept { return handle; }
			inline const HWND get_handle() const noexcept { return handle; }

#pragma region Properties

		utils::math::vec2i get_position() const noexcept
			{
			RECT rect;
			GetWindowRect(handle, &rect);
			return {rect.left, rect.top};
			}
		utils::math::vec2u get_size() const noexcept
			{
			RECT rect;
			GetClientRect(handle, &rect);
			return {static_cast<unsigned>(rect.right), static_cast<unsigned>(rect.bottom)};
			}
		void set_position(const utils::math::vec2i& position) noexcept { SetWindowPos(handle, NULL, position.x, position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER); }
		void set_size(const utils::math::vec2u& size)     noexcept
			{
			// SetWindowPos wants the total size of the window (including title bar and borders),
			// so we have to compute it
			RECT rectangle = {0, 0, static_cast<long>(size.x), static_cast<long>(size.y)};
			AdjustWindowRect(&rectangle, GetWindowLongPtr(handle, GWL_STYLE), false);
			int width  = rectangle.right - rectangle.left;
			int height = rectangle.bottom - rectangle.top;

			SetWindowPos(handle, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
			}

		int       get_x()      const noexcept { return get_position().x; }
		int       get_y()      const noexcept { return get_position().y; }
		unsigned  get_width()  const noexcept { return get_size().x; }
		unsigned  get_height() const noexcept { return get_size().y; }

		void      set_x(int x)           noexcept { set_position({x, get_y()}); }
		void      set_y(int y)           noexcept { set_position({get_x(), y}); }
		void      set_width(unsigned x)  noexcept { set_size({x, get_height()}); }
		void      set_height(unsigned y) noexcept { set_size({get_width(), y}); }

		__declspec(property(get = get_x, put = set_x))               int                x;
		__declspec(property(get = get_y, put = set_y))               int                y;
		__declspec(property(get = get_width, put = set_width))       unsigned           width;
		__declspec(property(get = get_height, put = set_height))     unsigned           height;
		__declspec(property(get = get_position, put = set_position)) utils::math::vec2i position;
		__declspec(property(get = get_size, put = set_size))         utils::math::vec2u size;
#pragma endregion
		private:
			HWND handle{nullptr};
			inline static constexpr wchar_t class_name[27]{L"CPP_Utilities Window Class"};

			user_data_t user_data{this, nullptr};

			void set_window_ptr() { user_data.window = this; SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&user_data)); }
			inline static utils::observer_ptr<base> get_window_ptr(HWND handle) { auto tmp = reinterpret_cast<user_data_t*>(GetWindowLongPtr(handle, GWLP_USERDATA)); return tmp ? tmp->window : nullptr; }
		};
	
	template <typename T>
	concept has_procedure =
		requires(T& t, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			{ t.procedure(msg, wparam, lparam) } -> std::same_as<std::optional<LRESULT>>;
		};


	template <std::derived_from<base> ...window_implementation_ts>
	class t : virtual public base, virtual public window_implementation_ts...
		{
		static_assert((std::is_base_of<utils::win32::window::base, window_implementation_ts>::value && ...), "All window implementations must inherit from utils::window.");
		private:
			inline static LRESULT __stdcall window_procedure(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
				{
				if (msg == WM_NCCREATE)
					{
					auto* userdata = &reinterpret_cast<t*>(reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams)->user_data;
					// store window instance pointer in window user data
					::SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
					return ::DefWindowProc(handle, msg, wparam, lparam);
					}
				auto window_ptr{reinterpret_cast<utils::observer_ptr<t>>(get_window_ptr(handle))};
				if (window_ptr) { return window_ptr->procedure(msg, wparam, lparam); }
				else { return ::DefWindowProc(handle, msg, wparam, lparam); }
				}

		public:
			struct initializer
				{
				inline initializer()
					{
					auto hBrush{CreateSolidBrush(RGB(0, 0, 0))};
					WNDCLASSEXW wcx
						{
						.cbSize        {sizeof(wcx)                      },
						.style         {CS_HREDRAW | CS_VREDRAW          },
						.lpfnWndProc   {window_procedure                 },
						.hInstance     {nullptr                          },
						.hCursor       {::LoadCursorW(nullptr, IDC_ARROW)},
						.hbrBackground {hBrush                           },
						.lpszClassName {class_name},
						};
					RegisterClassEx(&wcx);
					}
				inline ~initializer() { UnregisterClass(class_name, nullptr); }
				};

#pragma region con/de-structors
			struct create_info
				{
				std::wstring title{L"Untitled Window"};

				std::optional<utils::math::vec2i> position{std::nullopt};
				utils::math::vec2u size{800u, 600u};
				utils::graphics::color background_color{utils::graphics::color::black};
				bool glass{false};
				bool shadow{true};
				};

			inline t(const create_info& create_info)
				{
				int x{create_info.position ? create_info.position.value().x : CW_USEDEFAULT};
				int y{create_info.position ? create_info.position.value().y : CW_USEDEFAULT};

				handle = ::CreateWindowExW
				(
					0, class_name, create_info.title.c_str(),
					WS_OVERLAPPEDWINDOW, x, y,
					create_info.size.x, create_info.size.y, nullptr, nullptr, nullptr, this
				);

				if (!handle) { throw last_error("Failed to create window. Did you forget to create an utils::win32::window::initializer instance?"); }


				// redraw frame
				::SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
				::ShowWindow(handle, SW_SHOW);
				}
#pragma endregion con/de-structors

			inline bool poll_event() const
				{
				MSG msg;
				bool ret = PeekMessage(&msg, get_handle(), 0, 0, PM_REMOVE);
				if (ret)
					{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
					return true;
					}
				else { return false; }
				}

		private:
			
			inline LRESULT procedure(UINT msg, WPARAM wparam, LPARAM lparam) noexcept
				{
				std::optional<LRESULT> last_result;

				/*([&]<typename T>(std::type_identity<T>)
					{
					if constexpr (has_procedure<T>)
						{
						if (auto result{T::qwe(msg, wparam, lparam)})
							{
							last_result = result;
							}
						}
					}(std::type_identity<window_implementation_ts>{}), ...);// ??¿?!?¿!?!?!?!?¿

				switch (msg)
					{
					case WM_NCDESTROY: handle = nullptr; return 0;
					}*/

				return last_result ? last_result.value() : DefWindowProc(handle, msg, wparam, lparam);
				}
		};
	};