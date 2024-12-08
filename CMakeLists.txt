project(testTask)
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NLOHMANN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann)

include_directories(${INCLUDE_DIR} ${NLOHMANN_DIR})
include_directories(CMAKE_CURRENT_SOURCE_DIR)

set(SRC
        ${SRC_DIR}/Tape.cpp
        ${SRC_DIR}/factory/TapeFactory.cpp
        ${SRC_DIR}/factory/TemporaryTapeFactory.cpp
        ${SRC_DIR}/Sort.cpp
)

add_executable(${PROJECT_NAME} main.cpp ${SRC})


ADD_SUBDIRECTORY (googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(TEST_SRC
        ${TEST_DIR}/tests.cpp
        ${SRC}
)
add_executable(runTests ${TEST_SRC})
target_link_libraries(runTests gtest gtest_main)
add_test(runTests runTests)

add_executable(generateInputData generateInputData.cpp)