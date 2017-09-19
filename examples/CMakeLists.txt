
include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(c-example c-example.c ../include/acutest.h)
add_executable(cpp-example cpp-example.cc ../include/acutest.h)
