#include <android/log.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

__BEGIN_DECLS

int __android_log_write(int prio, const char *tag, const char *text)
{
    puts(text);
    return 0;
}

int __android_log_vprint(int prio, const char *tag,
                         const char *fmt, va_list ap)
{
    char text[1024];
    int n = vsnprintf(text, sizeof(text), fmt, ap);
    if (n >= sizeof(text)) {
        if (text[sizeof(text) - 2] != '\n') {
            text[sizeof(text) - 1] = '\n';
        }
        n = sizeof(text);
    } else {
        if (text[n - 1] != '\n') {
            text[n] = '\n';
        }
        ++n;
    }
    return write(0, text, n);
}

int __android_log_print(int prio, const char *tag,  const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = __android_log_vprint(prio, tag, fmt, ap);
    va_end(ap);
    return n;
}

void __android_log_assert(const char *cond, const char *tag,
			  const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(0, tag, fmt, ap);
    va_end(ap);
}

__END_DECLS
