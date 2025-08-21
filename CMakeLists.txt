cmake_minimum_required(VERSION 3.10)
project(uuk C ASM)

# ==========================================
# Toolchain
# ==========================================
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# ==========================================
# Targets
# ==========================================
add_subdirectory(kernel)
add_subdirectory(dist)