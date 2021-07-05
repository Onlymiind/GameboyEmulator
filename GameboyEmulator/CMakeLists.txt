
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
	)

include_directories(
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/dependencies/GLM)

set(CMAKE_CXX_SANDART 17)
set(SFML_STATIC_LIBRARIES TRUE)
set(CMAKE_CXX_FLAGS_RELEASE TRUE)


target_link_libraries(emulator sfml-system sfml-window sfml-graphics)




