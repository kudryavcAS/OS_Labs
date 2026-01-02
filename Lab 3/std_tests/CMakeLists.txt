add_executable(std_tests 
    test.cpp
    ../OS_Lab3/std_thread.cpp
)

target_link_libraries(std_tests PRIVATE Catch2::Catch2WithMain)

target_include_directories(std_tests PRIVATE ../OS_Lab3)

add_test(NAME std_tests COMMAND std_tests)