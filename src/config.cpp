#include "include/config.h"

namespace RaysterizerEngine
{
	RaysterizerConfig::RaysterizerConfig()
	{
		auto config_path = Constants::CONFIG_PATH;
		std::ifstream config_file(config_path.data());
		if (!config_file)
		{
			PANIC("Config does not exist at {}", config_path);
		}

		config_file >> j;
	}
}