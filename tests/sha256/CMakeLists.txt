#project(tests VERSION 0.1.0 LANGUAGES CXX)

# These tests can use the Catch2-provided main
add_executable(01_sha256 01_sha256.cpp)

target_link_libraries(01_sha256 PRIVATE Catch2::Catch2WithMain deCONZLib)
