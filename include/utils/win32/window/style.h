#pragma once

#include "window.h"
#include "details/style.h"

namespace utils::win32::window
	{
	struct style : virtual utils::win32::window::base
		{
		enum class transparency_t
			{
			composition_attribute,
			DWM_blurbehind,
			DWM_margin,
			layered,
			none
			};
		enum class value_t { enable, disable, _default };

		struct create_info
			{
			transparency_t transparency{transparency_t::none};
			value_t borders{value_t::_default};
			value_t shadow {value_t::_default};
			/// <summary> Only used by layered transparency </summary>
			uint8_t alpha{127};
			};

		style(create_info create_info) : borders{create_info.borders}
			{
			switch (create_info.transparency)
				{
				case transparency_t::composition_attribute:
					utils::window::make_glass_CompositionAttribute(get_handle());
					break;
				case transparency_t::DWM_blurbehind:
					utils::window::make_glass_DWM_BlurBehind(get_handle());
					break;
				case transparency_t::DWM_margin:
					utils::window::make_glass_DWM_margin(get_handle());
					break;
				case transparency_t::layered:
					utils::window::make_transparent_Layered(get_handle(), create_info.alpha);
					break;

				case transparency_t::none:
				default:
					break;
				}

			if (create_info.borders == value_t::disable)
				{
				DWORD style{static_cast<DWORD>(details::style::select_borderless_style())};
				SetWindowLongPtrW(get_handle(), GWL_STYLE, style);

				switch (create_info.shadow)
					{
					case value_t::enable : details::style::set_shadow(get_handle(), true ); break;
					case value_t::disable: details::style::set_shadow(get_handle(), false); break;
					case value_t::_default:
					default:
						break;
					}
				}
			}

		std::optional<LRESULT> procedure(UINT msg, WPARAM wparam, LPARAM lparam)
			{
			switch (msg)
				{
				case WM_NCCALCSIZE:
					if (wparam == TRUE && borders == value_t::disable) {
						auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
						details::style::adjust_maximized_client_rect(get_handle(), params.rgrc[0]);
						return 0;
						}
					break;
				}
			return std::nullopt;
			}

		private:
			value_t borders;
		};
	}