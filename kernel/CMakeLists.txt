cmake_minimum_required(VERSION 3.10)

# ==========================================
# Dependencies
# ==========================================
set(LIMINE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/boot/limine.h)
set(LIMINE_URL https://raw.githubusercontent.com/limine-bootloader/limine/v9.x-binary/limine.h)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/boot)

if(NOT EXISTS ${LIMINE_HEADER})
    message(STATUS ">>> Downloading limine.h from ${LIMINE_URL}")
    file(DOWNLOAD
            ${LIMINE_URL}
            ${LIMINE_HEADER}
            SHOW_PROGRESS
    )
endif()

set(FLANTERM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/flanterm)
set(FLANTERM_GIT_URL https://github.com/mintsuki/flanterm.git)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/lib)

if(NOT EXISTS ${FLANTERM_DIR}/.git)
    message(STATUS ">>> Cloning Flanterm repository from ${FLANTERM_GIT_URL}")
    execute_process(COMMAND git clone --depth 1 ${FLANTERM_GIT_URL} ${FLANTERM_DIR}
            RESULT_VARIABLE GIT_CLONE_RESULT
            OUTPUT_QUIET
            ERROR_QUIET
    )
    if(NOT GIT_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "!!! Failed to clone Flanterm repository.")
    endif()
endif()

# ==========================================
# Clean up deps
# ==========================================
if(EXISTS ${FLANTERM_DIR}/src)
    message(STATUS ">>> Reorganizing Flanterm source files")

    file(MAKE_DIRECTORY ${FLANTERM_DIR}/backends)

    foreach(file flanterm.c flanterm.h flanterm_private.h)
        if(EXISTS ${FLANTERM_DIR}/src/${file})
            file(RENAME ${FLANTERM_DIR}/src/${file} ${FLANTERM_DIR}/${file})
        endif()
    endforeach()

    if(EXISTS ${FLANTERM_DIR}/src/flanterm_backends)
        file(GLOB BACKEND_FILES "${FLANTERM_DIR}/src/flanterm_backends/*")
        foreach(f ${BACKEND_FILES})
            get_filename_component(fname ${f} NAME)
            file(RENAME ${f} ${FLANTERM_DIR}/backends/${fname})
        endforeach()
    endif()

    file(REMOVE_RECURSE ${FLANTERM_DIR}/src)
endif()


# ==========================================
# Source
# ==========================================
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(kernel ${SOURCES} ${FLANTERM_SOURCES})

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
)

# ==========================================
# Include directories
# ==========================================
target_include_directories(kernel PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ==========================================
# Linker
# ==========================================
target_link_options(kernel PRIVATE
        -nostdlib
        -static
        -z max-page-size=0x1000
        -Wl,--gc-sections
        -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
        -Wl,-m,elf_x86_64
)
