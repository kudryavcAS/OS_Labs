# CMakeList.txt: проект CMake для OS_Lab4; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

#Добавьте источник в исполняемый файл этого проекта.
add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_compile_definitions(NOMINMAX)
add_compile_definitions(BOOST_ALL_NO_LIB)
add_executable (receiver "receiver_func.cpp" "receiver.h" "receiver.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET receiver PROPERTY CXX_STANDARD 98)
endif()

if(WIN32)
    target_link_libraries(receiver ${SYSTEM_LIBS})
endif()

# Устанавливаем выходной каталог для ВСЕХ исполняемых файлов
set_target_properties(receiver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
# TODO: Добавьте тесты и целевые объекты, если это необходимо.