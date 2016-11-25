project(ABM)
cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE SRC_LIST RELATIVE ${PROJECT_SOURCE_DIR} "src/*.cpp")
file(GLOB_RECURSE HPP_LIST RELATIVE ${PROJECT_SOURCE_DIR} "include/*.hpp" "ThirdParty/*.hpp")

add_definitions(-std=c++14)
add_definitions(-Wall -Wpedantic -Wextra -Werror)

set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} "-O3")

find_package(SFML REQUIRED system window graphics)

include_directories(include)
include_directories(ThirdParty)
include_directories(${SFML_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${HPP_LIST} ${SRC_LIST} main.cpp)
target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
