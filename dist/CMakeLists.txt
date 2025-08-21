cmake_minimum_required(VERSION 3.10)
project(dist NONE)

# ==========================================
# Paths
# ==========================================
set(KERNEL_BIN ${CMAKE_BINARY_DIR}/kernel/kernel)
set(ISO_OUT ${CMAKE_BINARY_DIR}/uuk.iso)
set(LIMINE_DIR ${CMAKE_BINARY_DIR}/limine)
set(LIMINE_CONF ${CMAKE_CURRENT_SOURCE_DIR}/limine.conf)
set(OVMF_FD ${CMAKE_BINARY_DIR}/ovmf/ovmf-code-x86_64.fd)

file(MAKE_DIRECTORY ${ISO_ROOT})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ovmf)

# ==========================================
# Detect platform
# ==========================================
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(IS_WINDOWS TRUE)
    set(LIMINE_EXE "${LIMINE_DIR}/limine.exe")
    set(XORRISO_CMD xorriso.exe)
else()
    set(IS_WINDOWS FALSE)
    set(LIMINE_EXE "${LIMINE_DIR}/limine")
    set(XORRISO_CMD xorriso)
endif()


# ==========================================
# Limine fetch & build
# ==========================================
if(IS_WINDOWS)
    add_custom_command(
            OUTPUT ${LIMINE_DIR}/limine-bios.sys
            COMMAND ${CMAKE_COMMAND} -E echo "Skipping Limine build on Windows (binary already exists)..."
            COMMENT "Cloning limine..."
    )
else()
    add_custom_command(
            OUTPUT ${LIMINE_DIR}/limine-bios.sys
            COMMAND ${CMAKE_COMMAND} -E chdir ${LIMINE_DIR} make CC=${CMAKE_C_COMPILER} CFLAGS="-g -O2 -pipe"
            COMMENT "Cloning and building limine..."
    )
endif()

add_custom_target(limine ALL DEPENDS ${LIMINE_DIR}/limine-bios.sys)

# ==========================================
# OVMF fetch
# ==========================================
add_custom_command(
        OUTPUT ${OVMF_FD}
        COMMAND ${CMAKE_COMMAND} -E echo "Downloading OVMF firmware..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/ovmf
        COMMAND curl -L -o ${OVMF_FD} https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-code-x86_64.fd
        COMMENT "Downloading OVMF firmware..."
)

add_custom_target(ovmf ALL DEPENDS ${OVMF_FD})

# ==========================================
# ISO generation
# ==========================================
set(ISO_ROOT iso_root) # WARNING: This is relative path based on ${CMAKE_BINARY_DIR}, work around for windows.
add_custom_command(
        OUTPUT ${ISO_OUT}
        DEPENDS kernel limine ovmf ${KERNEL_BIN} ${LIMINE_CONF}
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${ISO_ROOT}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ISO_ROOT}/boot/limine"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ISO_ROOT}/EFI/BOOT"
        COMMAND ${CMAKE_COMMAND} -E copy "${KERNEL_BIN}" "${ISO_ROOT}/boot/"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIMINE_CONF}" "${ISO_ROOT}/boot/limine/"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIMINE_DIR}/limine-bios.sys" "${ISO_ROOT}/boot/limine/"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIMINE_DIR}/limine-bios-cd.bin" "${ISO_ROOT}/boot/limine/"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIMINE_DIR}/limine-uefi-cd.bin" "${ISO_ROOT}/boot/limine/"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIMINE_DIR}/BOOTX64.EFI" "${ISO_ROOT}/EFI/BOOT/"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIMINE_DIR}/BOOTIA32.EFI" "${ISO_ROOT}/EFI/BOOT/"
        COMMAND ${XORRISO_CMD} -as mkisofs
        -R -r -J
        -b boot/limine/limine-bios-cd.bin
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus
        -apm-block-size 2048
        --efi-boot boot/limine/limine-uefi-cd.bin
        -efi-boot-part --efi-boot-image --protective-msdos-label
        "${ISO_ROOT}" -o "${ISO_OUT}"
        COMMAND "${LIMINE_EXE}" bios-install "${ISO_OUT}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating bootable ISO..."
)
add_custom_target(uuk_iso ALL DEPENDS ${ISO_OUT})

# ==========================================
# QEMU run
# ==========================================
add_custom_target(run
        COMMAND qemu-system-x86_64
        -M q35
        -drive if=pflash,unit=0,format=raw,file=${OVMF_FD},readonly=on
        -cdrom "${ISO_OUT}"
        -smp 4
        -serial file:${CMAKE_BINARY_DIR}/com1.log
        -debugcon stdio
        DEPENDS uuk_iso ovmf
        COMMENT "Running QEMU with the built ISO..."
)
