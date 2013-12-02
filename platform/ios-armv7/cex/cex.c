#include "iconv.h"
#include "execinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Assume the worst for the width of an address.  */
#define WORD_WIDTH 16

__BEGIN_DECLS

char ***_NSGetEnviron(void)
{
    return (char ***)NULL;
}

iconv_t iconv_open(__const char *__tocode, __const char *__fromcode)
{
    return (iconv_t)(-1);
}

size_t iconv(
    iconv_t __cd,
    char **__restrict __inbuf,
    size_t *__restrict __inbytesleft,
    char **__restrict __outbuf,
    size_t *__restrict __outbytesleft)
{
    return (size_t)(-1);
}

int iconv_close(iconv_t __cd)
{
    return -1;
}
__END_DECLS
