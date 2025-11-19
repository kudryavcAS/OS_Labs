add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_compile_definitions(NOMINMAX)
add_compile_definitions(BOOST_ALL_NO_LIB)

add_executable (sender "sender.cpp" "sender.h" "sender_func.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
      set_property(TARGET sender PROPERTY CXX_STANDARD 98)
endif()

if(WIN32)
    target_link_libraries(sender ${SYSTEM_LIBS})
endif()

target_include_directories(sender PRIVATE ../receiver)

set_target_properties(sender PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)