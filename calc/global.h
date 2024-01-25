#pragma once
#include <color-sync.hpp>	//< for color::sync
#include <fileio.hpp>

#include <filesystem>

namespace calc {
	color::sync csync{};

	struct config_content {
		bool caretIsExponent{ false };
	};

	static config_content settings;
}
