cmake_minimum_required(VERSION 3.19)
project(GameboyEmulator)

option(BUILD_TESTS "Build tests" ON)
option(TESTS_ONLY "Build only tests" OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/)
set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

set(EMULATOR_LIB
    src/gb/cpu/CPU.cpp
    src/gb/cpu/instructions.cpp
    src/gb/memory/Memory.cpp
    src/gb/AddressBus.cpp
    src/gb/Timer.cpp
    src/gb/cpu/Decoder.cpp
    src/gb/cpu/Operation.cpp
    src/utils/Utils.h
    src/Error.h
    src/gb/AddressBus.h
    src/gb/InterruptRegister.h
    src/gb/Timer.h
    src/gb/memory/BasicComponents.h
    src/gb/memory/Memory.h
    src/gb/cpu/CPU.h
    src/gb/cpu/CPUUtils.h
    src/gb/cpu/Decoder.h
    src/gb/cpu/InstructionStep.h
    src/gb/cpu/Operation.h
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
    glad/src/glad.c
    glad/include/glad/glad.h
    glad/include/KHR/khrplatform.h
)


if(NOT TESTS_ONLY)
    add_subdirectory(glfw)

    add_executable(
        emulator

        ${IMGUI_LIB}
        ${IMGUI_BACKEND}
        ${GLAD}

        src/Application.h
        src/Application.cpp
        src/Source.cpp
    )


    target_link_libraries(emulator PUBLIC emulator_lib glfw)
    target_include_directories(emulator PRIVATE src imgui glfw/include glad/include)

    target_compile_features(emulator PUBLIC cxx_std_17)

endif()

if(BUILD_TESTS OR TESTS_ONLY)

    add_subdirectory(Catch2)

    add_executable(tests
        src/tests/AddressBusTest.cpp
        src/tests/DecoderTest.cpp
        src/tests/TimerTest.cpp
        src/tests/integration/IntegrationTest.cpp
    )
    target_link_libraries(tests PRIVATE emulator_lib Catch2WithMain)
    target_include_directories(tests PUBLIC src src/tests)

    include(CTest)
    include(Catch)
    catch_discover_tests(tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/tests/integration/roms)

endif()




