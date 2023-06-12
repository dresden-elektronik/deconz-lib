#ifndef DECONZ_H
#define DECONZ_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

/*! deCONZ library version as numeric value of the form:
    0xMMNNPP (MM = major, NN = minor, PP = patch).
 */
#define DECONZ_LIB_VERSION 0x011104

#include <deconz/declspec.h>
#include <deconz/types.h>
#include <deconz/aps.h>
#include <deconz/aps_controller.h>
#include <deconz/binding_table.h>
#include <deconz/dbg_trace.h>
#include <deconz/device_enumerator.h>
#include <deconz/green_power.h>
#include <deconz/green_power_controller.h>
#include <deconz/http_client_handler.h>
#include <deconz/node.h>
#include <deconz/node_event.h>
#include <deconz/node_interface.h>
#include <deconz/touchlink.h>
#include <deconz/touchlink_controller.h>
#include <deconz/timeref.h>
#include <deconz/util.h>
#include <deconz/u_rand32.h>
#include <deconz/zcl.h>
#include <deconz/zdp_profile.h>
#include <deconz/zdp_descriptors.h>

#if QT_VERSION >= 0x050000
  #include <deconz/qhttprequest_compat.h>
#endif

#endif // DECONZ_H
