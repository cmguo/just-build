// Config.cpp

#include "framework_test/Common.h"

#include <framework/configure/Config.h>
using namespace framework::configure;

int i = 1;
float f = 2.0;

static void test_config(Config & conf)
{
    conf.register_module("Config")
        << CONFIG_PARAM_RDWR(i)
        << CONFIG_PARAM_RDWR(f);

    std::cout << "i = " << i << std::endl;
    std::cout << "f = " << f << std::endl;
}

static TestRegister test("config", test_config);
