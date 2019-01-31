CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(HADAMER)

SET(CMAKE_BUILD_TYPE Debug)

SET(CMAKE_CXX_STANDARD 17)

SET(SOURCES
    src/Buffer.hpp
    src/Buffer.impl.hpp
    src/Control.hpp
    src/Control.impl.hpp
    src/Mesh.hpp
    src/Mesh.impl.hpp
    src/Types.hpp
    src/Util.hpp
    src/Util.impl.hpp)

SET(TEST_SOURCES
    test/BufferTest.cpp
    test/ControlTest.cpp
    test/MeshTest.cpp
    test/Main.cpp
    test/UtilTest.cpp)

SET(HEADERS
    src/Block.hpp
    src/Buffer.hpp
    src/Frac.hpp
    src/Types.hpp)

INCLUDE_DIRECTORIES(catch/single_include/catch2 src)

ADD_EXECUTABLE(hadamer_test ${TEST_SOURCES})

ADD_CUSTOM_TARGET(format
    COMMAND clang-format -style=file -i ${SOURCES} ${HEADERS} ${TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
