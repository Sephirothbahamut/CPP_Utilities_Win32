#pragma once
// IMPORTANT: If you include this header, remember to define WIN32_LEAN_AND_MEAN and NOMINMAX

#include <string>
#include <optional>
#include <vector>
#include <functional>

#include <utils/math/vec2.h>
#include <utils/graphics/color.h>
#include <utils/memory.h>

#include <Windows.h>

#include "error_to_exception.h"

namespace utils
	{

	template <typename T = void>
	class window
		{
		private: //Setup stuff
			/// <summary>
			/// Custom procedures must return std::nullopt for messages that are not being evaluated.
			/// When multiple procedures evaluate the same event, only the last one is returned to the OS API.
			/// </summary>
			using procedure_t = std::function<std::optional<LRESULT>(UINT msg, WPARAM wparam, LPARAM lparam)>;//std::optional<LRESULT>(*)(UINT msg, WPARAM wparam, LPARAM lparam);


			void set_window_ptr() { SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&user_data)); }
			inline static observer_ptr<window> get_window_ptr(HWND handle) { auto tmp = reinterpret_cast<user_data_t*>(GetWindowLongPtr(handle, GWLP_USERDATA)); return tmp ? tmp->window : nullptr; }

			inline static LRESULT __stdcall window_procedure(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
				{
				if (msg == WM_NCCREATE)
					{
					auto* userdata = &reinterpret_cast<window*>(reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams)->user_data;
					// store window instance pointer in window user data
					::SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
					return ::DefWindowProc(handle, msg, wparam, lparam);
					}

				if (observer_ptr<window> window_ptr{get_window_ptr(handle)}) { return window_ptr->procedure(msg, wparam, lparam); }
				else { return ::DefWindowProc(handle, msg, wparam, lparam); }
				}

			struct user_data_t { observer_ptr<window> window; T* userdata; };

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
			struct initializer_t
				{
				std::wstring title{L"Untitled Window"};

				std::optional<utils::math::vec2i> position{std::nullopt};
				utils::math::vec2u size{800u, 600u};
				utils::graphics::color background_color{utils::graphics::color::black};
				bool glass{false};
				bool shadow{true};
				};

			inline window(const initializer_t& initializer) 
				{
				int x{initializer.position ? initializer.position.value().x : CW_USEDEFAULT};
				int y{initializer.position ? initializer.position.value().y : CW_USEDEFAULT};

				handle = ::CreateWindowExW
				(
					0, class_name, initializer.title.c_str(),
					WS_OVERLAPPEDWINDOW, x, y,
					initializer.size.x, initializer.size.y, nullptr, nullptr, nullptr, this
				);

				if (!handle) { throw last_error("failed to create window"); }

				
				// redraw frame
				::SetWindowPos(handle, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
				::ShowWindow(handle, SW_SHOW);
				}
			inline window(const window& copy) = delete;
			inline window& operator=(const window& copy) = delete;
			inline window(window&& move) noexcept
				{
				handle = move.handle;
				move.handle = nullptr;
				set_window_ptr();
				}
			inline window& operator=(window&& move) noexcept
				{
				handle = move.handle;
				move.handle = nullptr;
				set_window_ptr();
				}
			inline ~window() noexcept { if (handle) { ::DestroyWindow(handle); } }

#pragma endregion con/de-structors
			void set_user_data(T* data) noexcept { user_data.userdata = data; }
			T* get_user_data() noexcept { return user_data.userdata; }

			bool poll_event() const
				{
				MSG msg;
				BOOL ret = GetMessage(&msg, handle, 0, 0);

				if (ret < 0) { throw last_error("Error polling window events"); }
				else if (ret > 0)
					{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
					return true;
					}
				else { return false; }
				}
		protected:
			std::vector<procedure_t> procedures;

		private:
			inline static constexpr wchar_t class_name[27]{L"CPP_Utilities Window Class"};

			HWND handle{nullptr};
			user_data_t user_data{this, nullptr};

			inline LRESULT procedure(UINT msg, WPARAM wparam, LPARAM lparam) noexcept
				{
				std::optional<LRESULT> last_result;
				for (auto procedure : procedures)
					{
					auto result{procedure(msg, wparam, lparam)};
					if (result) { last_result = result; }
					}

				return last_result ? last_result.value() : DefWindowProc(handle, msg, wparam, lparam);
				}
		};
	};