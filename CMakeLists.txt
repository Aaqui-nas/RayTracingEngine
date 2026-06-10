cmake_minimum_required(VERSION 3.15)
project(RayTracer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# ── Exécutable principal ──────────────────────────────────────────────────────
add_executable(raytracer main.cpp src/materials/materials.cpp)
target_include_directories(raytracer PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(raytracer PRIVATE Threads::Threads)
target_compile_options(raytracer PRIVATE -O2 -Wall -Wextra -mavx2 -mfma)

# ── Tests et benchmarks ───────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)
add_subdirectory(benchmarks)
