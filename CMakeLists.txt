cmake_minimum_required(VERSION 3.15)
project(RayTracer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# ── Exécutable principal ──────────────────────────────────────────────────────
add_executable(raytracer main.cpp)
target_link_libraries(raytracer PRIVATE Threads::Threads)
target_compile_options(raytracer PRIVATE -O2 -Wall -Wextra)

# ── Tests et benchmarks ───────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)
add_subdirectory(benchmarks)
