#include "dconf.h"

static LibSys::Config g_config;

LibSys::Config& get_config()
{
    return g_config;
}
