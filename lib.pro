TEMPLATE = lib
TARGET = deCONZ
VERSION = 1.2.0

# -Wno-attributes

QMAKE_CXXFLAGS += -Wall

CONFIG += c++14

CONFIG(debug, debug|release) {
    DESTDIR  = ..
#    QMAKE_CXXFLAGS += -Wconversion
}

CONFIG(release, debug|release) {
    DESTDIR  = ..
    DEFINES     += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT NDEBUG
}

QT += gui widgets serialport

INCLUDEPATH += deconz

SOURCES += \
          aps.cpp \
          aps_controller.cpp \
          atom_table.c \
          binding_table.cpp \
          buffer_helper.c \
          dbg_trace.cpp \
          device_enumerator.cpp \
          green_power.cpp \
          green_power_controller.cpp \
          qhttprequest_compat.cpp \
          zcl.cpp \
          zdp_descriptors.cpp \
          nanbox.c \
          node.cpp \
          node_event.cpp \
          http_client_handler.cpp \
          timeref.cpp \
          touchlink.cpp \
          touchlink_controller.cpp \
          ustring.cpp \
          util.cpp \
          u_rand32.c \
          u_sstream.c


win32:SOURCES += u_library_win32.c
unix:SOURCES += u_library_unix.c

HEADERS+= \
          deconz/aps.h \
          deconz/aps_controller.h \
          deconz/atom.h \
          deconz/atom_table.h \
          deconz/binding_table.h \
          deconz/buffer_helper.h \
          deconz/dbg_trace.h \
          deconz/declspec.h \
          deconz/device_enumerator.h \
          deconz/green_power.h \
          deconz/green_power_controller.h \
          deconz/timeref.h \
          deconz/touchlink.h \
          deconz/touchlink_controller.h \
          deconz/types.h \
          deconz/ustring.h \
          deconz/util.h \
          deconz/u_rand32.h \
          deconz/u_library.h \
          deconz/u_sstream.h \
          deconz/qhttprequest_compat.h \
          deconz/zcl.h \
          deconz/zdp_descriptors.h \
          deconz/zdp_profile.h \
          deconz/nanbox.h \
          deconz/node.h \
          deconz/node_event.h \
          deconz/node_interface.h \
          deconz/http_client_handler.h \
          deconz/mem_pool.h \
          aps_private.h \
          zcl_private.h \
          node_private.h \
          deconz.h

DEFINES += USE_QEXT_SERIAL \
           __STDC_FORMAT_MACROS \
           BUILD_ULIB_SHARED
