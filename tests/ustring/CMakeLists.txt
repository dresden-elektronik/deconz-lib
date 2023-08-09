
link_libraries(
    deCONZLib
    Qt${QT_VERSION_MAJOR}::Core)

add_executable(01_number_u64 01_number_u64.cpp)
add_executable(02_number_s64 02_number_s64.cpp)
add_executable(03_number_double 03_number_double.cpp)

add_test(NAME 01_number_u64 COMMAND 01_number_u64)
add_test(NAME 02_number_s64 COMMAND 02_number_s64)
add_test(NAME 03_number_double COMMAND 03_number_double)
