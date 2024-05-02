cmake_minimum_required(VERSION 3.11)

project(deCONZLib VERSION 1.2.0 LANGUAGES C;CXX)

option(BUILD_TESTS "Build tests" OFF)
option(USE_MICRO_ECC "Use micro-ecc library" ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Gui SerialPort REQUIRED)

if (UNIX)
    include(FindPkgConfig)
    pkg_check_modules (OPENSSL openssl)
else()
    find_package(OpenSSL REQUIRED)
endif()

include(FetchContent)

# --- fetch micro-ecc from GitHub --------------------------------
# Technically OpenSSL can do everything micro-ecc does but we already
# have an tested implementation, so use it for now.

if (USE_MICRO_ECC)
    FetchContent_Declare(
      libuecc
      GIT_REPOSITORY https://github.com/kmackay/micro-ecc.git
      GIT_TAG        v1.1
    )

    # FetchContent_MakeAvailable(uecc)
    # FetchContent_MakeAvailable requires CMake 3.14, but Debian Buster has only 3.13
    FetchContent_GetProperties(libuecc)
    if (NOT libuecc_POPULATED)
        FetchContent_Populate(libuecc)
    endif()

    add_library(uECC STATIC ${libuecc_SOURCE_DIR}/uECC.c)
    target_include_directories(uECC PUBLIC ${libuecc_SOURCE_DIR})

endif()

# -------------------------------------------------------------------------

set(PUB_INCLUDE_FILES
    deconz/aps.h
    deconz/aps_controller.h
    deconz/atom.h
    deconz/atom_table.h
    deconz/binding_table.h
    deconz/buffer_helper.h
    deconz/buffer_pool.h
    deconz/dbg_trace.h
    deconz/device_enumerator.h
    deconz/file.h
    deconz/green_power.h
    deconz/green_power_controller.h
    deconz/nanbox.h
    deconz/touchlink.h
    deconz/touchlink_controller.h
    deconz/timeref.h
    deconz/types.h
    deconz/util.h
    deconz/u_arena.h
    deconz/u_assert.h
    deconz/u_bstream.h
    deconz/u_ecc.h
    deconz/u_hmac_sha256.h
    deconz/u_random.h
    deconz/u_rand32.h
    deconz/u_library.h
    deconz/u_library_ex.h
    deconz/u_memory.h
    deconz/u_sha1.h
    deconz/u_sha256.h
    deconz/u_sstream.h
    deconz/u_sstream_ex.h
    deconz/u_threads.h
    deconz/u_time.h
    deconz/ustring.h
    deconz/zcl.h
    deconz/zdp_descriptors.h
    deconz/zdp_profile.h
    deconz/node.h
    deconz/node_event.h
    deconz/node_interface.h
    deconz/http_client_handler.h
    deconz/qhttprequest_compat.h
)

add_library(${PROJECT_NAME} SHARED
    ${PUB_INCLUDE_FILES}

    aps.cpp
    aps_controller.cpp
    atom_table.c
    binding_table.cpp
    buffer_helper.c
    buffer_pool.c
    dbg_trace.cpp
    device_enumerator.cpp
    green_power.cpp
    green_power_controller.cpp
    nanbox.c
    util.cpp
    u_arena.c
    u_bstream.c
    u_hmac_sha256.c
    u_library_ex.c
    u_memory.c
    u_random.c
    u_rand32.c
    u_sha1.c
    u_sha256.c
    u_sstream.c
    u_sstream_ex.c
    u_time.c
    ustring.cpp
    qhttprequest_compat.cpp
    zcl.cpp
    zdp_descriptors.cpp
    node.cpp
    node_event.cpp
    http_client_handler.cpp
    timeref.cpp
    touchlink.cpp
    touchlink_controller.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE U_LIBAPI_PRIVATE)

if (APPLE)
    target_include_directories(${PROJECT_NAME} PRIVATE PL_MACOS PL_UNIX)
endif()

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PL_WINDOWS)
endif()

if (LINUX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PL_LINUX PL_UNIX)
endif()

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE u_library_win32.c)
    target_sources(${PROJECT_NAME} PRIVATE u_threads_win32.c)
    target_sources(${PROJECT_NAME} PRIVATE file_win32.c)
    target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt)
else()
    target_sources(${PROJECT_NAME} PRIVATE u_threads_pthreads.c)
    target_sources(${PROJECT_NAME} PRIVATE u_library_unix.c)
    target_sources(${PROJECT_NAME} PRIVATE u_random_getentropy.c)
    target_sources(${PROJECT_NAME} PRIVATE file_posix.c)
endif()

if (USE_MICRO_ECC)
    target_sources(${PROJECT_NAME} PRIVATE u_micro_ecc.c)
    target_link_libraries(${PROJECT_NAME} PRIVATE uECC)
endif()

if (OPENSSL_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_OPENSSL=1)
    if (APPLE)
        target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})
    else()
        target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
    endif()
    target_sources(${PROJECT_NAME} PRIVATE u_random_openssl.c)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME deCONZ)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:DECONZ_DEBUG_BUILD>")

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    BUILD_ULIB_SHARED
    USE_QEXT_SERIAL
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Qt${QT_VERSION_MAJOR}::Gui
    PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/deconz>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#include(GNUInstallDirs)


#install(TARGETS ${PROJECT_NAME}
#        EXPORT ${PROJECT_NAME}Targets
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#)

#install(TARGETS ${PROJECT_NAME}
#    LIBRARY DESTINATION lib
#    PUBLIC_HEADER DESTINATION include/deconz
#)

#install(FILES ${PUB_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/deconz)
#install(FILES deconz.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#install(EXPORT ${PROJECT_NAME}Targets
#        FILE ${PROJECT_NAME}Targets.cmake
#        NAMESPACE deCONZ::
#        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
#)

#set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Manuel Pietschmann <mpi@dresden-elektronik.de>")
#include(CPack)
