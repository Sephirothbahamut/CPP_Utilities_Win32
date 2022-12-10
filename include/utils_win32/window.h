#pragma once
// IMPORTANT: If you include this header, remember to define WIN32_LEAN_AND_MEAN and NOMINMAX

/* NOTES: this mess is standard as per paper https://cplusplus.github.io/CWG/issues/257.html
Since stuff is virtual, we bypass the problem of not knowing if and what parameters the constructors of each T stuff inherits from require. The user is awaare of which types stuff will inherit from,
thereby he can write his own stuff child which only purpose is bridging between the rest of the program and the individual constructors of the various types stuff inherits from.

#include <iostream>
#include <string>

// library

template <typename ...Ts>
struct stuff : virtual Ts...
	{
	stuff(char c) {}

	virtual void i_am_abstract() = 0;
	};


template <typename ...Ts>
struct simple_stuff : stuff<Ts...>
	{
	using stuff<Ts...>::stuff;
	virtual void i_am_abstract() final override {};
	};

// user
struct goofy    { int q;                         };
struct donald   {        float s;                };
struct scroodge { scroodge(const char* s) : c{s} {}                std::string c; };
struct mickey   {                                };

struct leaf : stuff<goofy, donald, scroodge, mickey>
	{
	leaf(char c) : stuff(c), goofy{3}, donald{2.f}, scroodge{"hello"} {}
	virtual void i_am_abstract() final override {};
	};

using loaf = simple_stuff<goofy, donald, mickey>;

int main()
	{
	leaf thing{'g'};
	std::cout << thing.q << ", " << thing.s << ", " << thing.c << std::endl;

	loaf thong{'f'};
	}
*/

#include <string>
#include <optional>
#include <vector>
#include <functional>

#include <utils/math/vec2.h>
#include <utils/graphics/colour.h>
#include <utils/memory.h>
#include <utils/math/vec2.h>
#include <utils/math/geometry/aabb.h>
#include <utils/virtual.h>

#include "window.h"

#include "error_to_exception.h"
#include "details/style.h"

namespace utils::win32::window
	{
	using rect_t = utils::math::geometry::aabb<long>;

	class base;
	template <std::derived_from<base> ...window_implementation_ts>
	class t;

	class base
		{
		template <std::derived_from<base> ...window_implementation_ts>
		friend class t;

		public:
#pragma region con/de-structors
			struct create_info
				{
				std::wstring title{L"Untitled Window"};

				std::optional<utils::math::vec2i> position{std::nullopt};
				utils::math::vec2u size{800u, 600u};
				utils::graphics::colour::rgba background_color{utils::graphics::colour::black};
				};

			inline base(const create_info& create_info) : handle{create_window(create_info)}
				{
				::SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
				::ShowWindow(handle, SW_SHOW);
				}
			inline base(HWND handle = nullptr) : handle{handle} {}
			inline base(const base& copy) = delete;
			inline base& operator=(const base& copy) = delete;
			inline base(base&& move) noexcept
				{
				handle = move.handle;
				move.handle = nullptr;
				}
			inline base& operator=(base&& move) noexcept
				{
				handle = move.handle;
				move.handle = nullptr;
				}
			inline virtual ~base() noexcept { if (handle) { ::DestroyWindow(handle); } }

#pragma endregion con/de-structors

			inline void  set_user_data(void* data) noexcept { win32_user_data.user_data = data; }
			inline void* get_user_data() noexcept { return win32_user_data.user_data; }

			inline bool is_open() const noexcept { return handle; }

			inline const HWND get_handle() const noexcept
				{
				return handle;
				}

#pragma region Properties
			rect_t get_rect() const noexcept
				{
				RECT rect;
				GetWindowRect(handle, &rect);
				return {.rr{rect.right}, .up{rect.top}, .ll{rect.left}, .dw{rect.bottom}};
				}
			utils::math::vec2i get_position() const noexcept { return get_rect().pos ; }
			utils::math::vec2u get_size    () const noexcept { return get_rect().size; }

			void set_rect(rect_t rect) noexcept
				{
				// SetWindowPos wants the total size of the window (including title bar and borders),
				// so we have to compute it
				RECT rectangle = {.left{rect.ll}, .top{rect.up}, .right{rect.rr}, .bottom{rect.down}};
				AdjustWindowRect(&rectangle, static_cast<DWORD>(GetWindowLongPtr(handle, GWL_STYLE)), false);

				long width  = rectangle.right  - rectangle.left;
				long height = rectangle.bottom - rectangle.top;

				SetWindowPos(handle, NULL, rectangle.left, rectangle.top, width, height, SWP_NOZORDER);
				}
			void set_position(const utils::math::vec2i& position) noexcept { SetWindowPos(handle, NULL, position.x, position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER); }
			void set_size(const utils::math::vec2u& size)     noexcept
				{
				// SetWindowPos wants the total size of the window (including title bar and borders),
				// so we have to compute it
				RECT rectangle = {0, 0, static_cast<long>(size.x), static_cast<long>(size.y)};
				AdjustWindowRect(&rectangle, static_cast<DWORD>(GetWindowLongPtr(handle, GWL_STYLE)), false);

				int width  = rectangle.right - rectangle.left;
				int height = rectangle.bottom - rectangle.top;

				SetWindowPos(handle, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
				}

			int       get_x     () const noexcept { return get_position().x; }
			int       get_y     () const noexcept { return get_position().y; }
			unsigned  get_width () const noexcept { return get_size    ().x; }
			unsigned  get_height() const noexcept { return get_size    ().y; }

			void      set_x     (int x)      noexcept { set_position({x          , get_y     ()}); }
			void      set_y     (int y)      noexcept { set_position({get_x()    , y           }); }
			void      set_width (unsigned x) noexcept { set_size    ({x          , get_height()}); }
			void      set_height(unsigned y) noexcept { set_size    ({get_width(), y           }); }

			__declspec(property(get = get_x       , put = set_x       )) int                               x;
			__declspec(property(get = get_y       , put = set_y       )) int                               y;
			__declspec(property(get = get_width   , put = set_width   )) unsigned                          width;
			__declspec(property(get = get_height  , put = set_height  )) unsigned                          height;
			__declspec(property(get = get_position, put = set_position)) utils::math::vec2i                position;
			__declspec(property(get = get_size    , put = set_size    )) utils::math::vec2u                size;
			__declspec(property(get = get_rect    , put = set_rect    )) rect_t rect;
#pragma endregion
		private:
			HWND create_window(const create_info& create_info)
				{
				int x{create_info.position ? create_info.position.value().x : CW_USEDEFAULT};
				int y{create_info.position ? create_info.position.value().y : CW_USEDEFAULT};

				HWND ret = ::CreateWindowExW
				(
					0, class_name, create_info.title.c_str(),
					WS_OVERLAPPEDWINDOW, 
					x, y,
					create_info.size.x, create_info.size.y, nullptr, nullptr, nullptr, this
				);

				if (!ret) { throw last_error("Failed to create window. Did you forget to create an initializer instance for your window class? (utils::win32::window::t<[your types]>::initializer)"); }
				return ret;
				}

			struct win32_user_data_t { void* window; void* user_data; };
			win32_user_data_t win32_user_data{nullptr, nullptr};

			HWND handle{nullptr};
			inline static constexpr wchar_t class_name[27]{L"CPP_Utilities Window Class"};
		};

	template <typename T>
	concept has_procedure =
		requires(T & t, UINT msg, WPARAM wparam, LPARAM lparam)
		{
				{ t.procedure(msg, wparam, lparam) } -> std::same_as<std::optional<LRESULT>>;
		};

	// TODO: ensure that derived_from<base> is virtual base
	template <std::derived_from<base> ...window_implementation_ts>
	class t : public virtual base, public virtual window_implementation_ts..., public virtual utils::virtualize
		{
		static_assert((std::is_base_of<utils::win32::window::base, window_implementation_ts>::value && ...), "All window implementations must inherit from utils::window.");

		void set_window_ptr() { win32_user_data.window = this; SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&win32_user_data)); }
		inline static utils::observer_ptr<t> get_window_ptr(HWND handle)
			{
			auto   tmp = reinterpret_cast<win32_user_data_t*>(GetWindowLongPtr(handle, GWLP_USERDATA));
			return tmp ? reinterpret_cast<utils::observer_ptr<t>>(tmp->window) : nullptr;
			}

		private:

			inline static LRESULT __stdcall window_procedure(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
				{
				auto window_ptr{get_window_ptr(handle)};
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

			t()
				{
				// store window instance pointer in window user data
				set_window_ptr();
				//::SetWindowLongPtrW(get_handle(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(user_data));

				SetWindowPos(get_handle(), 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DRAWFRAME);
				::ShowWindow(get_handle(), SW_SHOW);
				}
			inline t(t&& move) noexcept
				{

				}
			inline t& operator=(t&& move) noexcept
				{
				}

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
				//Note: the callbacks return an optional; the optional is evaluated into a boolean with the operator || between the various callbacks;
				//this triggers short-circuiting, making it so that once one callback returns a valid values, all the other ones aren't evaluated anymore.
				std::optional<LRESULT> result;

				([&]<typename T>(std::type_identity<T>) -> bool
					{
					if constexpr (has_procedure<T>) { return (result = this->T::procedure(msg, wparam, lparam)).has_value(); }
					else { return false; }
					}(std::type_identity<window_implementation_ts>{}) || ...);// Dark magic here ??¿?!?¿!?!?!?!?¿

				switch (msg)
					{
					case WM_NCDESTROY: handle = nullptr; return 0;
					}

				return result ? result.value() : DefWindowProc(handle, msg, wparam, lparam);
				}
		};

	template <std::derived_from<base> ...window_implementation_ts>
	class simple_t : public t<window_implementation_ts...>, utils::devirtualize
		{
		public:
			//using t<window_implementation_ts...>::t;
			simple_t(base::create_info cinfo) : base{cinfo} {}
		};
	}