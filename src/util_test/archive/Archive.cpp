// Archive.cpp

#include "util_test/Common.h"

#include <util/archive/TextIArchive.h>
#include <util/archive/TextOArchive.h>
#include <util/archive/BinaryIArchive.h>
#include <util/archive/BinaryOArchive.h>
#include <util/archive/ArchiveBuffer.h>
using namespace util::archive;

using namespace framework::configure;

#include <istream>
#include <ostream>

void test_archive(Config & conf)
{
    int a = 4;
    int b = 6;

    char cbuf[1024];
    ArchiveBuffer<char> buf(cbuf, sizeof(cbuf));

    {
        TextOArchive<char> oa(buf);

        oa << (a);
        if (a == 1) { 
            oa << (b);
        }

        TextIArchive<char> ia(buf);

        ia >> (a) >> (b);
    }

    {
        BinaryOArchive<char> oa(buf);

        oa << (a) << (b);

        BinaryIArchive<char> ia(buf);

        ia >> (a) >> (b);
    }
}

static TestRegister test("archive", test_archive);
