cmake_minimum_required(VERSION 3.19)
project(GameboyEmulator)

option(BUILD_TESTS "Build tests" ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/)
cmake_policy(SET CMP0110 NEW)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)
enable_testing()
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
	src/gb/cpu/CPUUtils.cpp
	)

set(HDRS
	src/utils/Utils.h
	src/Application.h
	src/ConsoleInput.h
	src/ConsoleOutput.h
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

add_executable(
	emulator 
	${SOURCE_FILES}
	${HDRS}
	src/Source.cpp
	)

include_directories(
	${PROJECT_SOURCE_DIR}/src
	)
	
target_compile_features(emulator PUBLIC cxx_std_17)

if(BUILD_TESTS) 

	set(NO_ARGS_TESTS
		decoder_test
		interrupt_reg_test
		bus_test
		input_test
		timer_test
		coroutine_test
		)

	set(TEST_ROMS
		"01-special"
		"02-interrupts"
		"03-op sp,hl"
		"04-op r,imm"
		"05-op rp"
		"06-ld r,r"
		"07-jr,jp,call,ret,rst"
		"08-misc instrs"
		"09-op r,r"
		"10-bit ops"
		"11-op a,(hl)"
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
		run_test_rom
		${SOURCE_FILES}
		src/tests/integration/IntegrationTest.cpp
		src/tests/integration/IntegrationTest.h
		src/tests/integration/RunTestRom.cpp
	)

	add_executable(
		coroutine_test
		src/tests/TestCoroutine.cpp
		)


	foreach(EXE IN LISTS NO_ARGS_TESTS)
		target_compile_features(${EXE} PUBLIC cxx_std_17)
		add_test(NAME ${EXE} COMMAND ${EXE})
	endforeach()

	target_compile_features(run_test_rom PUBLIC cxx_std_17)
	foreach(ROM IN LISTS TEST_ROMS)
		add_test(NAME ${ROM} COMMAND run_test_rom ${ROM} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
	endforeach()

endif()




