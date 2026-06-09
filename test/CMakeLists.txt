cmake_minimum_required(VERSION 3.14)
project(simppeliTCU_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest QUIET)
if(GTest_FOUND)
    set(GTEST_MAIN_TARGET GTest::gtest_main)
else()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
    set(GTEST_MAIN_TARGET gtest_main)
endif()

add_library(test_stubs STATIC
    test_canInterface_stubs.cpp
)

target_include_directories(test_stubs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(can_leaf_ze1_parsing_tests
    test_canLeafZE1_parsing.cpp
    ../simppeliTCU/canLeafZE1.cpp
    ../simppeliTCU/vehicleTypes.h
    ../simppeliTCU/canInterface.h
    ../simppeliTCU/canLeafZE1.h
)

target_include_directories(can_leaf_ze1_parsing_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../simppeliTCU
)

target_link_libraries(can_leaf_ze1_parsing_tests PRIVATE 
    ${GTEST_MAIN_TARGET}
    test_stubs
)

enable_testing()

add_test(NAME can_leaf_ze1_parsing_tests COMMAND can_leaf_ze1_parsing_tests)
