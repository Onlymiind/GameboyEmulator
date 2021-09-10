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
	src/ConsoleOutput.cpp
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
	${PROJECT_SOURCE_DIR}/src
	)

target_compile_features(emulator PUBLIC cxx_std_17)
target_compile_features(decoder_test PUBLIC cxx_std_17)
target_compile_features(interrupt_reg_test PUBLIC cxx_std_17)
target_compile_features(bus_test PUBLIC cxx_std_17)
target_compile_features(input_test PUBLIC cxx_std_17)


if(UNIX)
	target_compile_options(emulator "$<$<CONFIG:DEBUG>:-g>")
endif()




