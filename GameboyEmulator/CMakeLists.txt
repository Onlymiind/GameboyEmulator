cmake_minimum_required(VERSION 3.10)
project(GameboyEmulator)

option(BUILD_TESTS "Build tests" OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/)

add_executable(emulator 
	src/utils/Utils.cpp
	src/Application.cpp
	src/gb/cpu/CPU.cpp
	src/gb/cpu/instructions.cpp
	src/gb/memory/Memory.cpp
	src/gb/AddressBus.cpp
	src/gb/Timer.cpp
	src/gb/memory/BasicComponents.cpp
	src/Source.cpp
	src/gb/cpu/Decoder.cpp
	src/gb/cpu/Operation.cpp
	src/ConsoleInput.cpp
	src/ConsoleOutput.cpp
	)

include_directories(
	${PROJECT_SOURCE_DIR}/src
	)
	
target_compile_features(emulator PUBLIC cxx_std_17)

if(BUILD_TESTS) 
	add_executable(
		decoder_test
		src/gb/cpu/Decoder.cpp
		src/tests/DecoderTest.cpp
	)

	add_executable(
		timer_test
		src/gb/Timer.cpp
		src/tests/TimerTest.cpp
	)

	add_executable(
		interrupt_reg_test
		src/gb/memory/Memory.cpp
		src/tests/InterruptRegisterTest.cpp
	)

	add_executable(
		bus_test
		src/gb/memory/Memory.cpp
		src/gb/AddressBus.cpp
		src/tests/AddressBusTest.cpp
	)

	add_executable(
		input_test
		src/ConsoleInput.cpp
		src/tests/ConsoleInputTest.cpp
	)

	add_executable(
		integration_test
		src/utils/Utils.cpp
		src/Application.cpp
		src/gb/cpu/CPU.cpp
		src/gb/cpu/instructions.cpp
		src/gb/memory/Memory.cpp
		src/gb/memory/BasicComponents.cpp
		src/gb/AddressBus.cpp
		src/gb/Timer.cpp
		src/tests/integration/IntegrationTest.cpp
		src/gb/cpu/Decoder.cpp
		src/gb/cpu/Operation.cpp
		src/ConsoleInput.cpp
		src/ConsoleOutput.cpp
	)

	target_compile_features(decoder_test PUBLIC cxx_std_17)
	target_compile_features(interrupt_reg_test PUBLIC cxx_std_17)
	target_compile_features(bus_test PUBLIC cxx_std_17)
	target_compile_features(input_test PUBLIC cxx_std_17)
	target_compile_features(integration_test PUBLIC cxx_std_17)
	target_compile_features(timer_test PUBLIC cxx_std_17)
endif()



if(UNIX)
	target_compile_options(emulator "$<$<CONFIG:DEBUG>:-g>")
endif()




