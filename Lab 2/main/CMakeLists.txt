# CMakeList.txt: проект CMake для CMake_Lab2; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (CMake_Lab2 "main.cpp" "CMake_Lab2.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMake_Lab2 PROPERTY CXX_STANDARD 17)
endif()

target_link_libraries(CMake_Lab2 PRIVATE arrayfunctions)

target_include_directories(CMake_Lab2 PRIVATE ${CMAKE_SOURCE_DIR}/lib)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
