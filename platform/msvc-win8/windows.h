// windows.h

#pragma once

#ifdef _WIN32_WINNT
#  undef _WIN32_WINNT
#endif

#define WINRT	1

#include <..\um\windows.h> // orignal windows.h

#include "winapi\SystemEmulation.h"
#include "winapi\FileSystemEmulation.h"
#include "winapi\ThreadEmulation.h"
#include "winapi\SocketEmulation.h"

using namespace SystemEmulation;
using namespace FileSystemEmulation;
using namespace ThreadEmulation;
using namespace SocketEmulation;
