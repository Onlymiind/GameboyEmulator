cmake_minimum_required(VERSION 3.20)
project(GameboyEmulator)

add_executable(emulator 
	src/utils/Utils.cpp
	src/Application.cpp
	src/gb/cpu/CPU.cpp
	src/gb/cpu/instructions.cpp
	src/gb/AddressBus.cpp
	src/gb/IO.cpp
	src/gb/Timer.cpp
	src/Source.cpp
	src/gb/cpu/Decoder.cpp
	src/gb/cpu/Operation.cpp
	src/ConsoleInput.cpp
	)

add_executable(
	decoder_test
	src/gb/cpu/Decoder.cpp
	src/tests/DecoderTest.cpp
)

add_executable(
	interrupt_reg_test
	src/tests/InterruptRegisterTest.cpp
)

add_executable(
	bus_test
	src/gb/AddressBus.cpp
	src/tests/AddressBusTest.cpp
)

add_executable(
	input_test
	src/ConsoleInput.cpp
	src/tests/ConsoleInputTest.cpp
)

include_directories(
	${PROJECT_SOURCE_DIR}/src)
if(WIN32)
	include_directories(emulator ${PROJECT_SOURCE_DIR}/dependencies/SFML/include)
endif()

set(SFML_STATIC_LIBRARIES ON)
target_compile_features(emulator PUBLIC cxx_std_17)
target_compile_features(decoder_test PUBLIC cxx_std_17)
target_compile_features(interrupt_reg_test PUBLIC cxx_std_17)
target_compile_features(bus_test PUBLIC cxx_std_17)


if(UNIX)
	target_link_libraries(emulator sfml-system sfml-window sfml-graphics)
	target_compile_options(emulator "$<$<CONFIG:DEBUG>:-g>")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
elseif(WIN32)
	add_compile_definitions(SFML_STATIC)
	target_link_directories(emulator PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/SFML/lib)
		target_link_libraries(emulator 
			optimized sfml-system-s optimized sfml-window-s optimized sfml-graphics-s
			debug sfml-system-s-d debug sfml-window-s-d debug sfml-graphics-s-d)
	target_link_libraries(emulator opengl32)
	target_link_libraries(emulator winmm)
endif()




