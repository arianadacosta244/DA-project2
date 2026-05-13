cmake_minimum_required(VERSION 3.16)
project(RegAlloc CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(regalloc
    src/main.cpp
    src/core/Web.cpp
    src/io/Parser.cpp
    src/io/OutputWriter.cpp
    src/algo/InterferenceBuilder.cpp
    src/algo/Allocator.cpp
    src/ui/Menu.cpp
)

target_include_directories(regalloc PRIVATE src)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXY_IN  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_IN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()
