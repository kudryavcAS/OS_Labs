
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(std_tests "test.cpp")


target_sources(std_tests PRIVATE ../OS_Lab3/std_thread.cpp)


set_property(TARGET std_tests PROPERTY CXX_STANDARD 11)
target_include_directories(std_tests PRIVATE ../OS_Lab3)


target_link_libraries(std_tests PRIVATE gtest_main)


enable_testing()
add_test(NAME std_tests COMMAND tests)