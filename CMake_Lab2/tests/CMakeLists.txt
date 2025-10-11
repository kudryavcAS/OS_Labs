add_executable (tests "tests.cpp")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET tests PROPERTY CXX_STANDARD 17)
endif()

enable_testing()


#target_link_libraries(tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)

target_link_libraries(tests PRIVATE arrayfunctions PRIVATE GTest::gtest_main PRIVATE Kernel32.lib)

target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/lib)