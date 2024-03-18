#project(tests VERSION 0.1.0 LANGUAGES CXX)

find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
add_executable(01_file 01_file.cpp)
add_executable(02_dir 02_dir.cpp)

target_link_libraries(01_file PRIVATE Catch2::Catch2WithMain deCONZLib)
target_link_libraries(02_dir PRIVATE Catch2::Catch2WithMain deCONZLib)
