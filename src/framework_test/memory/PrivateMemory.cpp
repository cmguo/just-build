// PrivateMemory.cpp

#include "framework_test/Common.h"

#include <framework/memory/PrivateMemory.h>
using namespace framework::configure;
using namespace framework::memory;

static void test_private_memory(Config & conf)
{
    PrivateMemory mem;
    const size_t size = 15 * 1024 * 1024;
    void * block = mem.alloc_block(size);
    mem.free_block(block, size);
}

static TestRegister test("private_memory", test_private_memory);
