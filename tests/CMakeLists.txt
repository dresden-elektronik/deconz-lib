
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_subdirectory(ustring)
add_subdirectory(file)
add_subdirectory(buffer_pool)
add_subdirectory(sha256)
