cmake_minimum_required(VERSION 3.10)
project(dist NONE)

# ==========================================
# Paths
# ==========================================
set(KERNEL_BIN ${CMAKE_BINARY_DIR}/kernel/kernel)
set(ISO_ROOT ${CMAKE_BINARY_DIR}/iso_root)
set(ISO_OUT ${CMAKE_BINARY_DIR}/uuk.iso)
set(LIMINE_DIR ${CMAKE_SOURCE_DIR}/limine)
set(LIMINE_CONF ${CMAKE_CURRENT_SOURCE_DIR}/limine.conf)
set(OVMF_FD ${CMAKE_BINARY_DIR}/ovmf/ovmf-code-x86_64.fd)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/iso_root)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ovmf)

# ==========================================
# Limine fetch & build
# ==========================================
add_custom_command(
        OUTPUT ${LIMINE_DIR}/limine-bios.sys
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${LIMINE_DIR}
        COMMAND git clone --branch v9.x-binary --depth 1 https://github.com/limine-bootloader/limine.git ${LIMINE_DIR}
        COMMAND ${CMAKE_COMMAND} -E chdir ${LIMINE_DIR} make CC=${CMAKE_C_COMPILER} CFLAGS=-g\ -O2\ -pipe
        COMMENT "Cloning and building Limine..."
)

add_custom_target(limine ALL DEPENDS ${LIMINE_DIR}/limine-bios.sys)


# ==========================================
# OVMF fetch
# ==========================================
add_custom_command(
        OUTPUT ${OVMF_FD}
        COMMAND curl -Lo ${OVMF_FD} https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-code-x86_64.fd
        COMMENT "Downloading OVMF firmware..."
)

add_custom_target(ovmf ALL DEPENDS ${OVMF_FD})

# ==========================================
# ISO generation
# ==========================================
add_custom_command(
        OUTPUT ${ISO_OUT}
        DEPENDS kernel limine ovmf ${KERNEL_BIN} ${LIMINE_CONF}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${ISO_ROOT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_ROOT}/boot/limine
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_ROOT}/EFI/BOOT
        COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BIN} ${ISO_ROOT}/boot/
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_CONF} ${ISO_ROOT}/boot/limine/
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-bios.sys ${ISO_ROOT}/boot/limine/
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-bios-cd.bin ${ISO_ROOT}/boot/limine/
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-uefi-cd.bin ${ISO_ROOT}/boot/limine/
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/BOOTX64.EFI ${ISO_ROOT}/EFI/BOOT/
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/BOOTIA32.EFI ${ISO_ROOT}/EFI/BOOT/
        COMMAND xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus
        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin
        -efi-boot-part --efi-boot-image --protective-msdos-label
        ${ISO_ROOT} -o ${ISO_OUT}
        COMMAND ${LIMINE_DIR}/limine bios-install ${ISO_OUT}
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
        -cdrom ${ISO_OUT}
        -smp 4
        -serial file:${CMAKE_BINARY_DIR}/com1.log
        -debugcon stdio
        DEPENDS uuk_iso ovmf
        COMMENT "Running QEMU with the built ISO..."
)
