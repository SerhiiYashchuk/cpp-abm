project(ABM)
cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE SRC_LIST RELATIVE ${PROJECT_SOURCE_DIR} "Src/*.cpp")
file(GLOB_RECURSE HPP_LIST RELATIVE ${PROJECT_SOURCE_DIR} "Include/*.hpp" "ThirdParty/*.hpp")
file(GLOB_RECURSE TESTS_LIST RELATIVE ${PROJECT_SOURCE_DIR} "Tests/*.cpp")

add_definitions(-std=c++14)
add_definitions(-Wall -Wpedantic -Wextra -Werror)

set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} "-O3")

find_package(SFML REQUIRED system window graphics)

include_directories(Include)
include_directories(ThirdParty)
include_directories(${SFML_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${HPP_LIST} ${SRC_LIST} main.cpp)
target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})

add_executable(Tests ${HPP_LIST} ${SRC_LIST} ${TESTS_LIST})
target_link_libraries(Tests ${SFML_LIBRARIES})
