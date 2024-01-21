#project(tests VERSION 0.1.0 LANGUAGES CXX)

find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
add_executable(01_file 01_file.cpp)

target_link_libraries(01_file PRIVATE Catch2::Catch2WithMain deCONZLib)
