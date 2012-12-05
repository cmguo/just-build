// windows.h

#pragma once

#include <..\Include\stdlib.h> // orignal windows.h

#ifdef  __cplusplus
extern "C" {
#endif

extern char * environ[];

char * __cdecl getenv(
	_In_z_ const char * _VarName
	);

#ifdef  __cplusplus
}
#endif
