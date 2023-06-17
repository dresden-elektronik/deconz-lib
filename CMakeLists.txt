cmake_minimum_required(VERSION 3.14)

project(deCONZLib VERSION 1.2.0 LANGUAGES C;CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Gui SerialPort REQUIRED)

set(PUB_INCLUDE_FILES
    deconz/aps.h
    deconz/aps_controller.h
    deconz/binding_table.h
    deconz/buffer_helper.h
    deconz/dbg_trace.h
    deconz/device_enumerator.h
    deconz/green_power.h
    deconz/green_power_controller.h
    deconz/touchlink.h
    deconz/touchlink_controller.h
    deconz/timeref.h
    deconz/types.h
    deconz/util.h
    deconz/u_rand32.h
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
    binding_table.cpp
    buffer_helper.c
    dbg_trace.cpp
    device_enumerator.cpp
    green_power.cpp
    green_power_controller.cpp
    util.cpp
    u_rand32.c
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

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME deCONZ)

#add_library(deCONZ::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    DECONZ_DLLSPEC=DECONZ_DECL_EXPORT
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
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

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
