#ifndef AM_GUI_H
#define AM_GUI_H

/*
 * Copyright (c) 2012-2025 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

 #include "actor/plugin.h"

/*! GUI node actor to communicate with core node.
    Main purpose is to decouple GUI code from core.
 */
#define AM_ACTOR_ID_GUI_NODE        3001
#define AM_ACTOR_ID_GUI_MAINWINDOW  3002

#define M_ID_GUI_NODE_SELECTED    AM_MESSAGE_ID_SPECIFIC_NOTIFY(0)
#define M_ID_GUI_NODE_DESELECTED  AM_MESSAGE_ID_SPECIFIC_NOTIFY(1)
#define M_ID_GUI_NODE_MOVED       AM_MESSAGE_ID_SPECIFIC_NOTIFY(2) // new position: (i32,i32) floating point position x 1000
#define M_ID_GUI_NODE_KEY_PRESSED AM_MESSAGE_ID_SPECIFIC_NOTIFY(3) // i32 key
#define M_ID_GUI_NODE_CONTEXT_MENU AM_MESSAGE_ID_SPECIFIC_NOTIFY(4)

struct am_api_functions;

// Wrapper for AM_ApiFunctions() so that gui actors don't depend on actor/service.h
am_api_functions *GUI_GetActorModelApi(void);

#endif // AM_GUI_H
