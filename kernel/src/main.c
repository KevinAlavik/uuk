#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <boot/limine.h>
#include <lib/flanterm/flanterm.h>
#include <lib/flanterm/backends/fb.h>
#include <dev/serial.h>
#include <util/kprintf.h>

// === Limine stuff ===
    __attribute__((used, section(".limine_requests")))
    static volatile LIMINE_BASE_REVISION(3);
    __attribute__((used, section(".limine_requests")))
    static volatile struct limine_framebuffer_request framebuffer_request = {
        .id = LIMINE_FRAMEBUFFER_REQUEST,
        .revision = 0
    };
    __attribute__((used, section(".limine_requests_start")))
    static volatile LIMINE_REQUESTS_START_MARKER;
    __attribute__((used, section(".limine_requests_end")))
    static volatile LIMINE_REQUESTS_END_MARKER;
// end

struct flanterm_context* ft_ctx = NULL;

// TODO: Move
static void hlt(void) {
    for (;;) {
        asm ("hlt");
    }
}

void uuk_entry(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hlt();
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hlt();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];


// === Setup output ===
    ft_ctx = flanterm_fb_init(
        NULL, NULL, framebuffer->address, framebuffer->width,
        framebuffer->height, framebuffer->pitch, framebuffer->red_mask_size,
        framebuffer->red_mask_shift, framebuffer->green_mask_size,
        framebuffer->green_mask_shift, framebuffer->blue_mask_size,
        framebuffer->blue_mask_shift, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, 0, 0, 1, 0, 0, 0);
    serial_init(COM1);
// end

    kprintf("Hello, World!\n");
    hlt();
}