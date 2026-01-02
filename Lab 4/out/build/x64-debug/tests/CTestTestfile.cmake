# CMake generated Testfile for 
# Source directory: D:/Y/OS_labs/Lab 4/tests
# Build directory: D:/Y/OS_labs/Lab 4/out/build/x64-debug/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(tests "D:/Y/OS_labs/Lab 4/out/build/x64-debug/tests/tests.exe")
set_tests_properties(tests PROPERTIES  _BACKTRACE_TRIPLES "D:/Y/OS_labs/Lab 4/tests/CMakeLists.txt;21;add_test;D:/Y/OS_labs/Lab 4/tests/CMakeLists.txt;0;")
subdirs("../_deps/googletest-build")
