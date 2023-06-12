/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QDataStream>
#include <deconz/green_power_controller.h>
#include <deconz/dbg_trace.h>
#include <deconz/util.h>

static deCONZ::GreenPowerController *_gpCtrl = nullptr;
static const char *gpdLqi[] = { "poor", "moderate", "high", "excellent" };

static struct GP_Frame
{
    quint32 gpdSrcId;
    quint32 frameCounter;
    quint8 gpdCommandId;
} lastReceivedGP;

namespace deCONZ {

class GreenPowerControllerPrivate
{
public:
    bool testZgpProxy = false;
};

GreenPowerController::GreenPowerController(QObject *parent) :
    QObject(parent),
    d_ptr(new GreenPowerControllerPrivate)
{
    Q_ASSERT(_gpCtrl == 0);
    _gpCtrl = this;

    d_ptr->testZgpProxy = appArgumentNumeric("--zgp-proxy-test", 0) > 0;
}

GreenPowerController::~GreenPowerController()
{
    delete d_ptr;
    d_ptr = nullptr;
    _gpCtrl = nullptr;
}

GreenPowerController *GreenPowerController::instance()
{
    DBG_Assert(_gpCtrl != nullptr);
    return _gpCtrl;
}

void GreenPowerController::processIncomingData(const QByteArray &data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    deCONZ::GpDataIndication ind;

    if (ind.readFromStream(stream))
    {
        // frames are received 3 times, forward only one
        if (!(ind.frameCounter() == lastReceivedGP.frameCounter &&
              ind.gpdSrcId()     == lastReceivedGP.gpdSrcId &&
              ind.gpdCommandId() == lastReceivedGP.gpdCommandId))
        {

            DBG_Printf(DBG_ZGP, "ZGP srcId: 0x%08X cmd: 0x%02X frameCounter: %u\n", ind.gpdSrcId(), ind.gpdCommandId(), ind.frameCounter());

            if (d_ptr->testZgpProxy)
            {
                DBG_Printf(DBG_ZGP, "ZGP ignore message with frameCounter: %u (test proxy)\n", ind.frameCounter());
            }
            else
            {
                lastReceivedGP.gpdSrcId = ind.gpdSrcId();
                lastReceivedGP.gpdCommandId = ind.gpdCommandId();
                lastReceivedGP.frameCounter = ind.frameCounter();
                emit gpDataIndication(ind);
            }
        }
    }
}

void GreenPowerController::processIncomingProxyNotification(const QByteArray &data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    deCONZ::GpDataIndication ind;

    if (ind.readFromStreamGpNotification(stream))
    {
        // frames are received 3 times, forward only one
        if (!(ind.frameCounter() == lastReceivedGP.frameCounter &&
              ind.gpdSrcId()     == lastReceivedGP.gpdSrcId &&
              ind.gpdCommandId() == lastReceivedGP.gpdCommandId) ||
              (ind.gpdCommandId() == GppCommandIdCommissioningNotification)) // forward all GPP comissioning notifications
        {
            lastReceivedGP.gpdSrcId = ind.gpdSrcId();
            lastReceivedGP.gpdCommandId = ind.gpdCommandId();
            lastReceivedGP.frameCounter = ind.frameCounter();

            DBG_Printf(DBG_ZGP, "ZGP via GPP proxy 0x%04X for GPD srcId: 0x%08X cmd: 0x%02X frameCounter: %u, GPD lqi: %s, rssi: %d\n", ind.gppShortAddress(), ind.gpdSrcId(), ind.gpdCommandId(), ind.frameCounter(), gpdLqi[ind.gppLqi()], ind.gppRssi());
            emit gpDataIndication(ind);
        }
    }
}

} // namespace deCONZ
