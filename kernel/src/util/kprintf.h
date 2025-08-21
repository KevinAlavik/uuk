// https://github.com/KevinAlavik/emk/blob/main/kernel/src/util/kprintf.h
#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>
#include <stddef.h>

/* Minimal kprintf using nanoprintf, see external/nanoprintf.h */
int kprintf(const char* fmt, ...);

/* Careful with these */
int snprintf(char* buf, size_t size, const char* fmt, ...);
int vsnprintf(char* buf, size_t size, const char* fmt, va_list args);

#endif // KPRINTF_H