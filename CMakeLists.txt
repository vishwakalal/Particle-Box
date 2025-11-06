cmake_minimum_required(VERSION 3.10)
project(particle-box VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(WITH_SFML "Enable SFML rendering" OFF)

set(SOURCES
    src/main.cpp
    src/cli.cpp
    src/physics.cpp
    src/quadtree.cpp
    src/spatial_hash.cpp
    src/engine_quadtree.cpp
    src/engine_hash.cpp
    src/metrics.cpp
    src/csv.cpp
)

set(HEADERS
    src/cli.hpp
    src/sim_config.hpp
    src/particle.hpp
    src/body_ref.hpp
    src/physics.hpp
    src/engine_quadtree.hpp
    src/engine_hash.hpp
    src/quadtree.hpp
    src/spatial_hash.hpp
    src/rng.hpp
    src/metrics.hpp
    src/csv.hpp
)

#sfml
if(WITH_SFML)
    find_package(SFML 3 REQUIRED COMPONENTS Window Graphics)
    list(APPEND SOURCES src/render.cpp)
    list(APPEND HEADERS src/render.hpp)
    add_executable(particle-box ${SOURCES} ${HEADERS})
    target_link_libraries(particle-box PRIVATE SFML::Window SFML::Graphics)
    target_compile_definitions(particle-box PRIVATE WITH_SFML)
else()
    add_executable(particle-box ${SOURCES} ${HEADERS})
endif()

target_compile_options(particle-box PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /O2>
)


target_include_directories(particle-box PRIVATE src)


add_executable(test_quadtree tests/test_quadtree.cpp
    src/quadtree.cpp src/rng.hpp)
target_include_directories(test_quadtree PRIVATE src)
target_compile_options(test_quadtree PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -O2>
)

add_executable(test_spatial_hash tests/test_spatial_hash.cpp
    src/spatial_hash.cpp src/rng.hpp)
target_include_directories(test_spatial_hash PRIVATE src)
target_compile_options(test_spatial_hash PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -O2>
)

add_executable(test_physics tests/test_physics.cpp
    src/physics.cpp src/particle.hpp)
target_include_directories(test_physics PRIVATE src)
target_compile_options(test_physics PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -O2>
)

