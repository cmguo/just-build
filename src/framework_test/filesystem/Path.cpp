// Path.cpp

#include "framework_test/Common.h"

#include <framework/filesystem/Path.h>
using namespace framework::filesystem;
using namespace framework::configure;

static void test_filesystem(Config & conf)
{
    std::string pp = bin_file().file_string();
}

static TestRegister test("filesystem", test_filesystem);
