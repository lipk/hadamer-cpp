CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(HADAMER)

SET(CMAKE_CXX_STANDARD 11)

SET(SOURCES
    src/Buffer.cpp)

SET(TEST_SOURCES
    src/Main.cpp)

SET(HEADERS
    src/Buffer.hpp
    src/Frac.hpp
    src/Types.hpp)

ADD_LIBRARY(hadamer ${SOURCES})

ADD_CUSTOM_TARGET(format
    COMMAND clang-format -style=file -i ${SOURCES} ${HEADERS} ${TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
