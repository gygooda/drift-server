#pragma once

#include "conf.h"

LibSys::Config& get_config();

#define CONFIG get_config()
