#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include <utils/memory.h>
#include <utils/containers/handled_container.h>
#include <utils/math/vec2.h>

#include "window.h"


// For future textbox shortcuts
// https://learn.microsoft.com/en-us/windows/win32/learnwin32/accelerator-tables

namespace utils::input
	{
	class mouse
		{
		//TODO scrolling wheel
		public:
			mouse(uintptr_t device_handle = 0) : device_handle{device_handle} {}
			uintptr_t get_device_handle() { return device_handle; }

			enum class button { left, right, middle, backward, forward };
			struct state_t
				{
				utils::math::vec2l position;

				// I know this is wrong, but I like to live dangerously. Until a bug is reported I'll keep it.
				// To fix: replace the non-array union members with msvc's properties
				union
					{
					std::array<bool, 5> buttons;
					struct
						{
						bool left;
						bool right;
						bool middle;
						bool backward;
						bool forward;
						};
					};
				} state;

			void move_to(utils::math::vec2l position)
				{
				state.position = position;
				for (auto& action : move_to_actions) { action(position); }
				}

			void move_by(utils::math::vec2l delta)
				{
				for (auto& action : move_by_actions) { action(delta); }
				}

			void button_update(button button, bool state) 
				{
				bool previous_state{this->state.buttons[static_cast<size_t>(button)]};
				if(state != previous_state)
					{
					if (state) { for (auto& action : button_down_actions) { action(button); } }
					else       { for (auto& action : button_up_actions  ) { action(button); } }
					}
				}

			void button_down(button button) 
				{
				state.buttons[static_cast<size_t>(button)] = true;
				for (auto& action : button_down_actions) { action(button); }
				}

			void button_up(button button) 
				{
				state.buttons[static_cast<size_t>(button)] = false;
				for (auto& action : button_up_actions) { action(button); }
				}

			utils::containers::handled_container<std::function<void(utils::math::vec2l position)>> move_to_actions;
			utils::containers::handled_container<std::function<void(utils::math::vec2l delta   )>> move_by_actions;
			utils::containers::handled_container<std::function<void(button button              )>> button_down_actions;
			utils::containers::handled_container<std::function<void(button button              )>> button_up_actions;
			
		private:
			uintptr_t device_handle;
		};


	//class keyboard
	//	{
	//	public:
	//
	//		struct button_pressed_data  { uintptr_t device_handle; size_t key_index; };
	//		struct button_released_data { uintptr_t device_handle; size_t key_index; };
	//
	//		inner<button_pressed_data > key_pressed;
	//		inner<button_released_data> key_released;
	//		
	//	private:
	//	};

	//TODO controller controller
	//TODO virtual /*analog 1d, analog 2d, digital, customizable from mouse, keyboard and controller*/
	}


namespace utils::win32::window::input
	{
	class mouse : public virtual utils::win32::window::base
		{
		public:
			mouse()
				{
				//mouse
				RAWINPUTDEVICE rid_mouse
					{
					.usUsagePage{0x01},
					.usUsage    {0x02},//Mouse
					.dwFlags    {RIDEV_INPUTSINK},
					.hwndTarget {get_handle()},
					};
				if (!RegisterRawInputDevices(&rid_mouse, 1, sizeof(rid_mouse))) { throw last_error("failed to register raw input devices"); }

				////controller
				//RAWINPUTDEVICE rid_controller
				//	{
				//	.usUsagePage{0x01},
				//	.usUsage    {0x05},//gamepad
				//	.dwFlags    {RIDEV_INPUTSINK},
				//	.hwndTarget {handle},
				//	};
				//if (!RegisterRawInputDevices(&rid_controller, 1, sizeof(rid_controller))) { throw last_error("failed to register raw input devices"); }
				//
				////keyboard
				//RAWINPUTDEVICE rid_keyboard
				//	{
				//	.usUsagePage{0x01},
				//	.usUsage    {0x06},//Keyboard
				//	.dwFlags    {RIDEV_INPUTSINK},
				//	.hwndTarget {handle},
				//	};
				//if (!RegisterRawInputDevices(&rid_keyboard, 1, sizeof(rid_keyboard))) { throw last_error("failed to register raw input devices"); }
				}

			std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam)
				{
				if (msg == WM_MOUSEMOVE)
					{
					auto x = GET_X_LPARAM(lparam);
					auto y = GET_Y_LPARAM(lparam);
					move_to(0, {x, y});
					return 0;
					}
				else if (msg == WM_LBUTTONUP)
					{
					auto x = GET_X_LPARAM(lparam);
					auto y = GET_Y_LPARAM(lparam);
					button_up(0, utils::input::mouse::button::left);
					return 0;
					}
				else if (msg == WM_LBUTTONDOWN)
					{
					auto x = GET_X_LPARAM(lparam);
					auto y = GET_Y_LPARAM(lparam);
					button_down(0, utils::input::mouse::button::left);
					return 0;
					}
				else if (msg == WM_INPUT)
					{
					UINT dataSize;
					GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER)); //Need to populate data size first
					//std::cout << GET_RAWINPUT_CODE_WPARAM(wparam) << " code thing\n";
					if (dataSize > 0)
						{
						std::vector<BYTE> rawdata(dataSize);

						if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, rawdata.data(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
							{
							RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.data());
							if (raw->header.dwType == RIM_TYPEMOUSE)
								{
								auto device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)};

								// https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse
								//TODO wheel
								const auto& rawmouse{raw->data.mouse};

								if ((rawmouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
									{
									const bool isVirtualDesktop = (rawmouse.usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP;

									const int width  = GetSystemMetrics(isVirtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
									const int height = GetSystemMetrics(isVirtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);

									auto const x = static_cast<long>((float(rawmouse.lLastX) / 65535.0f) * float(width ));
									auto const y = static_cast<long>((float(rawmouse.lLastY) / 65535.0f) * float(height));

									move_to(device_handle, {x, y});
									}
								else if (rawmouse.lLastX != 0 || rawmouse.lLastY != 0)
									{
									long x = rawmouse.lLastX;
									long y = rawmouse.lLastY;

									move_by(device_handle, {x, y});
									}
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) { button_up  (device_handle, utils::input::mouse::button::left    ); }
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP  ) { button_down(device_handle, utils::input::mouse::button::left    ); }
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) { button_up  (device_handle, utils::input::mouse::button::right   ); }
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP  ) { button_down(device_handle, utils::input::mouse::button::right   ); }
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) { button_up  (device_handle, utils::input::mouse::button::middle  ); }
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP  ) { button_down(device_handle, utils::input::mouse::button::middle  ); }
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) { button_up  (device_handle, utils::input::mouse::button::backward); }
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP  ) { button_down(device_handle, utils::input::mouse::button::backward); }
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) { button_up  (device_handle, utils::input::mouse::button::forward ); }
								if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP  ) { button_down(device_handle, utils::input::mouse::button::forward ); }
								}
							}

						return 0;
						}
					}
				return std::nullopt;
				}

			utils::containers::handled_container<utils::observer_ptr<utils::input::mouse>> mice_ptrs;

		private:
			void button_up(uintptr_t device_handle, utils::input::mouse::button button)
				{
				for (auto mouse_ptr : mice_ptrs)
					{
					const uintptr_t handle{mouse_ptr->get_device_handle()};
					if (handle == 0 || handle == device_handle) { mouse_ptr->button_up(button); }
					}
				}
			void button_down(uintptr_t device_handle, utils::input::mouse::button button)
				{
				for (auto mouse_ptr : mice_ptrs)
					{
					const uintptr_t handle{mouse_ptr->get_device_handle()};
					if (handle == 0 || handle == device_handle) { mouse_ptr->button_down(button); }
					}
				}
			void move_to(uintptr_t device_handle, utils::math::vec2l position)
				{
				for (auto mouse_ptr : mice_ptrs)
					{
					const uintptr_t handle{mouse_ptr->get_device_handle()};
					if (handle == 0 || handle == device_handle) { mouse_ptr->move_to(position); }
					}
				}
			void move_by(uintptr_t device_handle, utils::math::vec2l delta)
				{
				for (auto mouse_ptr : mice_ptrs)
					{
					const uintptr_t handle{mouse_ptr->get_device_handle()};
					if (handle == 0 || handle == device_handle) { mouse_ptr->move_by(delta); }
					}
				}
		};
	//class input : public virtual utils::win32::window::base
	//	{
	//	public:
	//		struct create_info { utils::input::manager& input_manager; };
	//
	//		input(create_info create_info) : input_manager_ptr{&create_info.input_manager} {}
	//
	//		std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam)
	//			{
	//			switch (msg)
	//				{
	//				case WM_ACTIVATE:
	//				case WM_ACTIVATEAPP:
	//				case WM_MOUSEMOVE:
	//				case WM_LBUTTONDOWN:
	//				case WM_LBUTTONUP:
	//				case WM_RBUTTONDOWN:
	//				case WM_RBUTTONUP:
	//				case WM_MBUTTONDOWN:
	//				case WM_MBUTTONUP:
	//				case WM_MOUSEWHEEL:
	//				case WM_XBUTTONDOWN:
	//				case WM_XBUTTONUP:
	//				case WM_MOUSEHOVER:
	//					break;
	//
	//				case WM_INPUT:
	//				{
	//				UINT dataSize;
	//				GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER)); //Need to populate data size first
	//				//std::cout << GET_RAWINPUT_CODE_WPARAM(wparam) << " code thing\n";
	//				if (dataSize > 0)
	//					{
	//					std::vector<BYTE> rawdata(dataSize);
	//
	//					if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, rawdata.data(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
	//						{
	//						RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.data());
	//						switch (raw->header.dwType)
	//							{
	//							case RIM_TYPEMOUSE:
	//							{
	//							// https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse
	//							//TODO wheel
	//							const auto& rawmouse{raw->data.mouse};
	//
	//							if ((rawmouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
	//								{
	//								const bool isVirtualDesktop = (rawmouse.usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP;
	//
	//								const int width  = GetSystemMetrics(isVirtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
	//								const int height = GetSystemMetrics(isVirtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);
	//
	//								auto const x = static_cast<long>((float(rawmouse.lLastX) / 65535.0f) * float(width ));
	//								auto const y = static_cast<long>((float(rawmouse.lLastY) / 65535.0f) * float(height));
	//
	//								//TODO lLastX/lLastY or x/y?
	//								input_manager_ptr->mouse.move_to({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .position{x, y}});
	//								}
	//							else if (rawmouse.lLastX != 0 || rawmouse.lLastY != 0)
	//								{
	//								int relativeX = rawmouse.lLastX;
	//								int relativeY = rawmouse.lLastY;
	//								
	//								input_manager_ptr->mouse.move_by({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .delta{relativeX, relativeY}});
	//								}
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) { input_manager_ptr->mouse.button_pressed ({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{1}}); }
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP  ) { input_manager_ptr->mouse.button_released({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{1}}); }
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) { input_manager_ptr->mouse.button_pressed ({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{2}}); }
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP  ) { input_manager_ptr->mouse.button_released({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{2}}); }
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) { input_manager_ptr->mouse.button_pressed ({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{3}}); }
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP  ) { input_manager_ptr->mouse.button_released({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{3}}); }
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) { input_manager_ptr->mouse.button_pressed ({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{4}}); }
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP  ) { input_manager_ptr->mouse.button_released({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{4}}); }
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) { input_manager_ptr->mouse.button_pressed ({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{5}}); }
	//							if (rawmouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP  ) { input_manager_ptr->mouse.button_released({.device_handle{reinterpret_cast<uintptr_t>(raw->header.hDevice)}, .button_index{5}}); }
	//							}
	//							break;
	//
	//							case RIM_TYPEKEYBOARD:
	//							{
	//							// https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawkeyboard
	//							std::cout << "Keyboard handle " << raw->header.hDevice << "\n";
	//							const auto& rawkb{raw->data.keyboard};
	//
	//							if (rawkb.MakeCode == KEYBOARD_OVERRUN_MAKE_CODE || rawkb.VKey == 0xff)//VK__none_
	//								{
	//								break;
	//								}
	//
	//							try
	//								{// https://stackoverflow.com/questions/38584015/using-getkeynametext-with-special-keys
	//								uint16_t scanCode = rawkb.MakeCode;
	//								scanCode |= (rawkb.Flags & RI_KEY_E0) ? 0xe000 : 0;
	//								scanCode |= (rawkb.Flags & RI_KEY_E1) ? 0xe100 : 0;
	//
	//								constexpr uint16_t c_BreakScanCode{0xe11d}; // emitted on Ctrl+NumLock
	//								constexpr uint16_t c_NumLockScanCode{0xe045};
	//								constexpr uint16_t c_PauseScanCode{0x0045};
	//								// These are special for historical reasons
	//								// https://en.wikipedia.org/wiki/Break_key#Modern_keyboards
	//								// Without it GetKeyNameTextW API will fail for these keys
	//								if (scanCode == c_BreakScanCode) { scanCode = c_PauseScanCode; }
	//								else if (scanCode == c_PauseScanCode) { scanCode = c_NumLockScanCode; }
	//
	//								std::cout << GetScanCodeName(scanCode) << " ";
	//
	//								if ((rawkb.Flags & RI_KEY_BREAK) == RI_KEY_BREAK) { std::cout << "released\n"; }
	//								else if ((rawkb.Flags & RI_KEY_MAKE) == RI_KEY_MAKE) { std::cout << "pressed \n"; }
	//								//else if ((rawkb.Flags & RI_KEY_E0   ) == RI_KEY_E0   ) { std::cout << "E0 whatever it means\n"; }
	//								//else if ((rawkb.Flags & RI_KEY_E1   ) == RI_KEY_E1   ) { std::cout << "E1 whatever it means\n"; }
	//								}
	//							catch (const std::system_error& error) { std::cout << error.what() << "\n"; }
	//							}
	//							break;
	//							//case RIM_TYPEMOUSE:
	//								//std::cout << "Mouse handle " << raw->header.hDevice << ": " << raw->data.mouse.lLastX << ", " << raw->data.mouse.lLastY << std::endl;
	//								//break;
	//							}
	//						}
	//
	//					return 0;
	//					}
	//				}
	//				}
	//			return std::nullopt;
	//			}
	//
	//	private:
	//		utils::observer_ptr<utils::input::manager> input_manager_ptr;
	//
	//		inline static std::string GetScanCodeName(uint16_t scanCode)
	//			{
	//			const bool isExtendedKey = (scanCode >> 8) != 0;
	//
	//			// Some extended keys doesn't work properly with GetKeyNameTextW API
	//			if (isExtendedKey)
	//				{
	//				const uint16_t vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
	//				switch (vkCode)
	//					{
	//					case VK_BROWSER_BACK:
	//						return "Browser Back";
	//					case VK_BROWSER_FORWARD:
	//						return "Browser Forward";
	//					case VK_BROWSER_REFRESH:
	//						return "Browser Refresh";
	//					case VK_BROWSER_STOP:
	//						return "Browser Stop";
	//					case VK_BROWSER_SEARCH:
	//						return "Browser Search";
	//					case VK_BROWSER_FAVORITES:
	//						return "Browser Favorites";
	//					case VK_BROWSER_HOME:
	//						return "Browser Home";
	//					case VK_VOLUME_MUTE:
	//						return "Volume Mute";
	//					case VK_VOLUME_DOWN:
	//						return "Volume Down";
	//					case VK_VOLUME_UP:
	//						return "Volume Up";
	//					case VK_MEDIA_NEXT_TRACK:
	//						return "Next Track";
	//					case VK_MEDIA_PREV_TRACK:
	//						return "Previous Track";
	//					case VK_MEDIA_STOP:
	//						return "Media Stop";
	//					case VK_MEDIA_PLAY_PAUSE:
	//						return "Media Play/Pause";
	//					case VK_LAUNCH_MAIL:
	//						return "Launch Mail";
	//					case VK_LAUNCH_MEDIA_SELECT:
	//						return "Launch Media Selector";
	//					case VK_LAUNCH_APP1:
	//						return "Launch App 1";
	//					case VK_LAUNCH_APP2:
	//						return "Launch App 2";
	//					}
	//				}
	//
	//			const LONG lParam = MAKELONG(0, (isExtendedKey ? KF_EXTENDED : 0) | (scanCode & 0xff));
	//			wchar_t name[128] = {};
	//			size_t nameSize = ::GetKeyNameTextW(lParam, name, sizeof(name));
	//
	//			std::string ret;
	//			std::transform(name, name + nameSize, std::back_inserter(ret), [](wchar_t c) {
	//				return (char)c;
	//				});
	//
	//			return ret;
	//			}
	//	};
	}