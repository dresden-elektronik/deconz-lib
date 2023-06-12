/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/aps_controller.h"
#include "deconz/dbg_trace.h"
#include "deconz/node_event.h"

static deCONZ::ApsController *_apsCtrl = nullptr;

namespace deCONZ {

ApsController::ApsController(QObject *parent) :
    QObject(parent)
{
    _apsCtrl = this;

    qRegisterMetaType<NodeEvent>( "NodeEvent" );
}

ApsController::~ApsController()
{
    _apsCtrl = nullptr;
}

ApsController * ApsController::instance()
{
    DBG_Assert(_apsCtrl != nullptr);
    return _apsCtrl;
}

} // namespace deCONZ

uint8_t DECONZ_DLLSPEC APS_NextApsRequestId()
{
    if (_apsCtrl)
    {
        return _apsCtrl->nextRequestId();
    }

    return 0;
}
