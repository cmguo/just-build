// Base64.cpp

#include "framework_test/Common.h"

#include <framework/string/Base64.h>
using namespace framework::string;
using namespace framework::configure;

static void test_base64(Config & conf)
{
    std::string str = "http://www.google.cn/search?hl=zh-CN&newwindow=1&rlz=1T4GGLL_zh-CNCN330CN330&q=%E9%94%9A%E9%93%BE+%E7%BF%BB%E8%AF%91#1gsd";
    std::string str1 = Base64::encode(str);
    std::string str2 = Base64::decode(str1);
    assert(str == str2);
}

static TestRegister test("base64", test_base64);
