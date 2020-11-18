cmake_minimum_required(VERSION 3.10)

project(LIQUID)

add_compile_options(-std=c++17 -Wall -Ofast)


find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL REQUIRED
    sdl2
    SDL2_image
    )

add_executable(liquid
    src/main.cpp
    src/simulation.cpp
    src/fx.cpp
    )

target_include_directories(liquid PRIVATE
    ${SDL_INCLUDE_DIRS}
    )

target_link_libraries(liquid
    ${SDL_LIBRARIES}
    )
