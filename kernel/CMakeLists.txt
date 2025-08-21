cmake_minimum_required(VERSION 3.10)

# ==========================================
# Dependencies
# ==========================================
set(LIMINE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/boot/limine.h)
set(LIMINE_URL https://raw.githubusercontent.com/limine-bootloader/limine/v9.x-binary/limine.h)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/boot)

if(NOT EXISTS ${LIMINE_HEADER})
    message(STATUS ">>> Downloading limine.h from ${LIMINE_URL}")
    file(DOWNLOAD
            ${LIMINE_URL}
            ${LIMINE_HEADER}
            SHOW_PROGRESS
    )
endif()

# ==========================================
# Source
# ==========================================
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(kernel ${SOURCES})
# ==========================================
# Compiler
# ==========================================
if (NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "!!! To build uuk you need a GCC compiler which can output x86_64 ELF")
endif()

target_compile_options(kernel PRIVATE
        -g
        -O2
        -pipe
        -Wall
        -Wextra
        -Werror
        -std=gnu11
        -ffreestanding
        -fno-stack-protector
        -fno-stack-check
        -fno-PIC
        -ffunction-sections
        -fdata-sections
        -m64
        -march=x86-64
        -mno-80387
        -mno-mmx
        -mno-sse
        -mno-sse2
        -mno-red-zone
        -mcmodel=kernel
        -I${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_options(kernel PRIVATE
        -nostdlib
        -static
        -z max-page-size=0x1000
        -Wl,--gc-sections
        -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
        -Wl,-m,elf_x86_64
)
