// Url.cpp

#include "framework_test/Common.h"

#include <framework/string/Url.h>
using namespace framework::string;
using namespace framework::configure;

static void test_url(Config & conf)
{
    Url url("http://www.google.cn/search?hl=zh-CN&newwindow=1&rlz=1T4GGLL_zh-CNCN330CN330&q=%E9%94%9A%E9%93%BE+%E7%BF%BB%E8%AF%91#1gsdg");
}

static TestRegister test("url", test_url);
