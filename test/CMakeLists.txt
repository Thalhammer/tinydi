enable_testing()
include(GoogleTest)
find_package(GTest REQUIRED)

set(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/InjectorTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DefaultInjectorTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LazyHandleTest.cpp
)

add_executable(tinydi-test ${TEST_SOURCES})

# NOTE: Don't use space inside a generator expression here, because the function prematurely breaks the expression into
# multiple lines. https://cmake.org/pipermail/cmake/2018-April/067422.html
set(TINYDI_TESTER_GCC_FLAGS -Wall -Wextra -Wpedantic)
set(TINYDI_TESTER_CLANG_FLAGS -Weverything -Wno-c++98-compat -Wno-global-constructors -Wno-weak-vtables)
target_compile_options(
  tinydi-test PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4> $<$<CXX_COMPILER_ID:GNU>:${TINYDI_TESTER_GCC_FLAGS}>
                       $<$<CXX_COMPILER_ID:Clang>:${TINYDI_TESTER_CLANG_FLAGS}>)
target_link_libraries(tinydi-test PRIVATE tinydi GTest::GTest GTest::Main
                                           $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${CMAKE_DL_LIBS}>)

gtest_add_tests(TARGET tinydi-test)