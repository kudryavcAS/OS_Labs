add_executable(tests 
    test.cpp
    ../OS_Lab3/winapi.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

target_include_directories(tests PRIVATE ../OS_Lab3)

add_test(NAME tests COMMAND tests)