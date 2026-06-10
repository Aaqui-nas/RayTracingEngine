include(FetchContent)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(run_benchmarks
    bench_vec3.cpp
    bench_hit.cpp
    bench_camera.cpp
    bench_textures.cpp
    bench_normals.cpp
    bench_ibl.cpp
    bench_direct_lighting.cpp
    ${CMAKE_SOURCE_DIR}/src/materials/materials.cpp
)
target_include_directories(run_benchmarks PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(run_benchmarks PRIVATE benchmark::benchmark benchmark::benchmark_main)
target_compile_features(run_benchmarks PRIVATE cxx_std_17)
target_compile_options(run_benchmarks PRIVATE -O3 -march=native)
