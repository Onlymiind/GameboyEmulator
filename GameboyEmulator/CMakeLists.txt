cmake_minimum_required(VERSION 3.10)
project(GameboyEmulator)

option(BUILD_TESTS "Build tests" OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/)
set(SOURCE_FILES
	src/utils/Utils.cpp
	src/Application.cpp
	src/gb/cpu/CPU.cpp
	src/gb/cpu/instructions.cpp
	src/gb/memory/Memory.cpp
	src/gb/AddressBus.cpp
	src/gb/Timer.cpp
	src/gb/memory/BasicComponents.cpp
	src/gb/cpu/Decoder.cpp
	src/gb/cpu/Operation.cpp
	src/ConsoleInput.cpp
	src/ConsoleOutput.cpp
	)

add_executable(
	emulator 
	${SOURCE_FILES}
	src/Source.cpp
	)

include_directories(
	${PROJECT_SOURCE_DIR}/src
	)
	
target_compile_features(emulator PUBLIC cxx_std_17)

if(BUILD_TESTS) 

	set(TESTS
		decoder_test
		interrupt_reg_test
		bus_test
		input_test
		integration_test
		timer_test
		coroutine_test
		)

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
		${SOURCE_FILES}
		src/tests/integration/IntegrationTest.cpp
		)

	add_executable(
		coroutine_test
		src/tests/TestCoroutine.cpp
		)


	foreach(EXE IN LISTS TESTS)
		target_compile_features(${EXE} PUBLIC cxx_std_17)
	endforeach()

endif()




