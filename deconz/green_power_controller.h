#ifndef DECONZ_GREEN_POWER_CONTROLLER_H
#define DECONZ_GREEN_POWER_CONTROLLER_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QObject>
#include <deconz/types.h>
#include <deconz/green_power.h>

namespace deCONZ {

class GreenPowerControllerPrivate;

/*!
    \ingroup greenpower
    \class GreenPowerController
    \brief Provides services to access green power functionality.
 */
class DECONZ_DLLSPEC GreenPowerController : public QObject
{
    Q_OBJECT

public:
    /*! Constructor. */
    GreenPowerController(QObject *parent);
    /*! Deconstructor. */
    virtual ~GreenPowerController();
    /*! Get the singleton instance of the GreenPowerController. */
    static GreenPowerController *instance();

    void processIncomingData(const QByteArray &data);
    void processIncomingProxyNotification(const QByteArray &data);

Q_SIGNALS:
    /*! Is emitted on the reception of a green power frame.

        \param ind the green power data indication frame
     */
    void gpDataIndication(const deCONZ::GpDataIndication&);

private:
    GreenPowerControllerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(GreenPowerController)
};

} // namespace deCONZ

#endif // DECONZ_GREEN_POWER_CONTROLLER_H
