cmake_minimum_required(VERSION 3.16)
project(graph_drawing VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
	nlohmann_json
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(graph_drawing ${SOURCES})
target_link_libraries(graph_drawing PRIVATE nlohmann_json::nlohmann_json)

target_compile_options(graph_drawing PRIVATE
    -fsanitize=address,undefined
    -g
    -O2
    -D_GLIBCXX_DEBUG
)

target_link_options(graph_drawing PRIVATE
    -fsanitize=address,undefined
)
