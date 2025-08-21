cmake_minimum_required(VERSION 3.10)
project(uuk NONE)

# ==========================================
# Toolchain
# ==========================================
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# ==========================================
# Targets
# ==========================================
add_subdirectory(kernel)
add_subdirectory(dist)