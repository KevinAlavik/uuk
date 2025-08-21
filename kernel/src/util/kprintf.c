//https://github.com/KevinAlavik/emk/blob/main/kernel/src/util/kprintf.h
#include <boot/uuk.h>
#include <dev/serial.h>
#include <util/kprintf.h>
#include <lib/flanterm/flanterm.h>
#include <sys/spinlock.h>

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_IMPLEMENTATION
#include <lib/nanoprintf.h>

static spinlock_t kprintf_lock = {0};

int kprintf(const char* fmt, ...) {
    spinlock_acquire(&kprintf_lock);

    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    int length = npf_vsnprintf(buffer, sizeof(buffer), fmt, args);

    if (length >= 0 && length < (int)sizeof(buffer)) {
        serial_write(COM1, (uint8_t*)buffer, length);
        if (ft_ctx)
            flanterm_write(ft_ctx, (char*)buffer, length);
    }

    va_end(args);

    spinlock_release(&kprintf_lock);
    return length;
}

int snprintf(char* buf, size_t size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int length = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return length;
}

int vsnprintf(char* buf, size_t size, const char* fmt, va_list args) {
    int length = npf_vsnprintf(buf, size, fmt, args);

    if (length >= (int)size) {
        buf[size - 1] = '\0';
    }

    return length;
}