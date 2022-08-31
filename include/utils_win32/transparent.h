#pragma once

#include "window.h"
#include "../../../CPP_Utilities/include/utils/window.h"

namespace utils::win32::window
	{
	enum class transparency_t 
		{
		composition_attribute,
		DWM_blurbehind,
		DWM_margin,
		layered
		};

	template <transparency_t transparency>
	struct transparent : virtual utils::win32::window::base
		{
		transparent()
			{
			if constexpr (transparency == transparency_t::composition_attribute)
				{
				utils::window::make_glass_CompositionAttribute(get_handle());
				}
			if constexpr (transparency == transparency_t::DWM_blurbehind)
				{
				utils::window::make_glass_DWM_BlurBehind(get_handle());
				}
			if constexpr (transparency == transparency_t::DWM_margin)
				{
				utils::window::make_glass_DWM_margin(get_handle());
				}
			if constexpr (transparency == transparency_t::layered)
				{
				utils::window::make_transparent_Layered(get_handle());
				}
			}
		};
	}