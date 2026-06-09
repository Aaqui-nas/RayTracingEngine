include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(run_tests
    test_vec3.cpp
    test_ray.cpp
    test_shapes.cpp
    test_scene.cpp
    test_materials.cpp
    test_aabb.cpp
    test_camera.cpp
)
target_include_directories(run_tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(run_tests PRIVATE Catch2::Catch2WithMain)
target_compile_features(run_tests PRIVATE cxx_std_17)
target_compile_options(run_tests PRIVATE -Wall -Wextra)

include(CTest)
include(Catch)
catch_discover_tests(run_tests)
