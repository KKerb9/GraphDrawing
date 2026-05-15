cmake_minimum_required(VERSION 3.16)
project(graph_drawing VERSION 0.0.1 LANGUAGES CXX)

option(GD_ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(graph_drawing ${SOURCES})

target_compile_features(graph_drawing PRIVATE cxx_std_17)
target_link_libraries(graph_drawing PRIVATE nlohmann_json::nlohmann_json)

target_compile_options(graph_drawing PRIVATE
	-Wall
	-Wextra
	-Wpedantic
)

if(GD_ENABLE_SANITIZERS)
	target_compile_options(graph_drawing PRIVATE
		-fsanitize=address,undefined
		-fno-omit-frame-pointer
	)
	target_link_options(graph_drawing PRIVATE
		-fsanitize=address,undefined
	)
endif()
