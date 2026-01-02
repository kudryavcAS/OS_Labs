include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(tests "tests.cpp")

target_sources(tests PRIVATE ../receiver/receiver_func.cpp)
target_sources(tests PRIVATE ../sender/sender_func.cpp)

set_property(TARGET tests PROPERTY CXX_STANDARD 98)
target_include_directories(tests PRIVATE ../receiver)
target_include_directories(tests PRIVATE ../sender)

find_package(Boost REQUIRED COMPONENTS interprocess system filesystem)
target_link_libraries(tests PRIVATE gtest_main 
    Boost::interprocess Boost::system Boost::filesystem)

enable_testing()
add_test(NAME tests COMMAND tests)