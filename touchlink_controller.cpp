/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <deconz/touchlink_controller.h>
#include <deconz/dbg_trace.h>
#include <deconz/u_rand32.h>

static deCONZ::TouchlinkController *_touchlinkCtrl = 0;

namespace deCONZ {

class TouchlinkControllerPrivate
{
public:
};

TouchlinkController::TouchlinkController(QObject *parent) :
    QObject(parent),
    d_ptr(new TouchlinkControllerPrivate)
{
    DBG_Assert(_touchlinkCtrl == 0);
    _touchlinkCtrl = this;
}

TouchlinkController::~TouchlinkController()
{
    delete d_ptr;
    d_ptr = 0;
    _touchlinkCtrl = 0;
}

TouchlinkController *TouchlinkController::instance()
{
    DBG_Assert(_touchlinkCtrl != 0);
    return _touchlinkCtrl;
}

uint32_t TouchlinkController::generateTransactionId() const
{
    uint32_t id = 0;
    for (;id == 0;) // must be non zero
    {
        id = U_rand32();
    }
    return id;
}

} // namespace deCONZ
