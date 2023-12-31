#pragma once
#include <color-sync.hpp>	//< for color::sync

namespace calc {
	color::sync csync{};

	static struct {
		bool caretIsExponent = false;
	} settings;
}
