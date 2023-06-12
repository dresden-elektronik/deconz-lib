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

SOURCES += \
          aps.cpp \
          aps_controller.cpp \
          binding_table.cpp \
          buffer_helper.c \
          dbg_trace.cpp \
          device_enumerator.cpp \
          green_power.cpp \
          green_power_controller.cpp \
          util.cpp \
          qhttprequest_compat.cpp \
          zcl.cpp \
          zdp_descriptors.cpp \
          node.cpp \
          node_event.cpp \
          http_client_handler.cpp \
          timeref.cpp \
          touchlink.cpp \
          touchlink_controller.cpp \
          u_rand32.c

HEADERS+= \
          deconz/aps.h \
          deconz/aps_controller.h \
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
          deconz/util.h \
          deconz/u_rand32.h \
          deconz/qhttprequest_compat.h \
          deconz/zcl.h \
          deconz/zdp_descriptors.h \
          deconz/zdp_profile.h \
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
           DECONZ_DLLSPEC=DECONZ_DECL_EXPORT
