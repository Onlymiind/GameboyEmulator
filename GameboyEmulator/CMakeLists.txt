cmake_minimum_required(VERSION 3.20)
project(GameboyEmulator)

add_executable(emulator 
	src/utils/Utils.cpp
	src/core/Application.cpp
	src/core/gb/cpu/CPU.cpp
	src/core/gb/cpu/instructions.cpp
	src/core/gb/AddressBus.cpp
	src/core/gb/IO.cpp
	src/core/gb/Timer.cpp
	src/Source.cpp
	src/core/gb/cpu/Decoder.cpp
	)

include_directories(
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/dependencies/GLM)
if(WIN32)
	include_directories(emulator ${PROJECT_SOURCE_DIR}/dependencies/SFML/include)
endif()

set(SFML_STATIC_LIBRARIES ON)
target_compile_features(emulator PUBLIC cxx_std_17)


if(UNIX)
	target_link_libraries(emulator sfml-system sfml-window sfml-graphics)
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




