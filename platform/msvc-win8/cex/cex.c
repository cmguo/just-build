// cex.c

#include <windows.h>

#include <stdlib.h>

#ifdef  __cplusplus
extern "C" {
#endif

char * __cdecl getenv(
	_In_z_ const char * _VarName
	)
{
	char const * const * p = environ;
	size_t len = strlen(_VarName);
	while (*p) {
		if (strncmp(*p, _VarName, len) == 0 && (*p)[len] == '=') {
			return (char *)*p + len + 1;
		}
		++p;
	}
	return 0;
}

#ifdef  __cplusplus
}
#endif
