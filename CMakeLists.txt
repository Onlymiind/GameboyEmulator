cmake_minimum_required(VERSION 3.19)
project(GameboyEmulator)

option(BUILD_TESTS "Build tests" ON)
option(TESTS_ONLY "Build only tests" OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

set(EMULATOR_LIB
    src/gb/cpu/cpu.cpp
    src/gb/cpu/instructions.cpp
    src/gb/address_bus.cpp
    src/gb/timer.cpp
    src/gb/cpu/decoder.cpp
    src/gb/cpu/operation.cpp
    src/util/util.h
    src/gb/address_bus.h
    src/gb/interrupt_register.h
    src/gb/timer.h
    src/gb/memory/basic_components.h
    src/gb/cpu/cpu.h
    src/gb/cpu/cpu_utils.h
    src/gb/cpu/decoder.h
    src/gb/cpu/operation.h
    src/gb/ppu/ppu.h
    src/gb/ppu/ppu.cpp
)

add_library(emulator_lib
    ${EMULATOR_LIB}
)
target_include_directories(emulator_lib PRIVATE src)

set(IMGUI_LIB
    imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/misc/cpp/imgui_stdlib.h
    imgui/misc/cpp/imgui_stdlib.cpp
)

set(IMGUI_BACKEND
    imgui/backends/imgui_impl_opengl3.h
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_glfw.h
    imgui/backends/imgui_impl_glfw.cpp
)

set(GLAD
    glad/src/gl.c
    glad/include/glad/gl.h
    glad/include/KHR/khrplatform.h
)


if(NOT TESTS_ONLY)
    add_subdirectory(glfw)

    add_executable(
        emulator

        ${IMGUI_LIB}
        ${IMGUI_BACKEND}
        ${GLAD}
        
        src/breakpoint.h
        src/breakpoint.cpp
        src/renderer.h
        src/renderer.cpp
        src/application.h
        src/application.cpp
        src/main.cpp
    )


    target_link_libraries(emulator PUBLIC emulator_lib glfw)
    target_include_directories(emulator PRIVATE src imgui glfw/include glad/include)

    target_compile_features(emulator PUBLIC cxx_std_17)

endif()

if(BUILD_TESTS OR TESTS_ONLY)

    add_subdirectory(Catch2)

    add_executable(tests
        src/tests/decoder_test.cpp
        src/tests/timer_test.cpp
        src/tests/integration/intergration_test.cpp
        src/tests/memory_breakpoints_test.cpp

        src/breakpoint.h
        src/breakpoint.cpp
    )
    target_link_libraries(tests PRIVATE emulator_lib Catch2WithMain)
    target_include_directories(tests PUBLIC src src/tests)

    include(CTest)
    include(Catch)
    catch_discover_tests(tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/tests/integration/roms)

endif()




