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
    test_textures.cpp
    test_normals.cpp
    test_ibl.cpp
    test_direct_lighting.cpp
    test_pbr.cpp
    test_concepts.cpp
    test_procedural.cpp
    test_transform.cpp
    test_primitives.cpp
    test_alloc.cpp
    ${CMAKE_SOURCE_DIR}/src/materials/materials.cpp
)
target_include_directories(run_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(run_tests PRIVATE Catch2::Catch2WithMain)
target_compile_features(run_tests PRIVATE cxx_std_20)
target_compile_options(run_tests PRIVATE -Wall -Wextra -mavx2 -mfma)

include(CTest)
include(Catch)
catch_discover_tests(run_tests)
