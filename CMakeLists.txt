cmake_minimum_required(VERSION 3.0)
project(unnamed)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

set(TARGET_NAME ${PROJECT_NAME})

set(lib_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/unnamed_ptr.h
  ${CMAKE_CURRENT_SOURCE_DIR}/make_deleter.h
)

add_library(${TARGET_NAME} INTERFACE)
target_sources(${TARGET_NAME} INTERFACE ${lib_headers})

target_include_directories(${TARGET_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	${Boost_INCLUDE_DIRS}
)

add_subdirectory(examples)

enable_testing()
add_executable(tests
    tests/simple.cpp
    tests/polymorphic.cpp
    tests/borrowed.cpp
    tests/alloc.cpp
)

target_compile_definitions(tests PRIVATE BOOST_TEST_DYN_LINK)
target_link_libraries(tests ${PROJECT_NAME} Boost::unit_test_framework)

add_test(NAME test COMMAND tests)
