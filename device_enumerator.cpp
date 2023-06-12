/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <QDirIterator>
#include <QTimer>
#include <vector>

#ifdef USE_QEXT_SERIAL
  #include <QSerialPortInfo>
#endif // USE_QEXT_SERIAL

#ifdef Q_OS_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "deconz/aps_controller.h"
#include "deconz/dbg_trace.h"
#include "deconz/device_enumerator.h"
#include "deconz/util.h"

// Firmware version related (32-bit field)
#define FW_PLATFORM_MASK          0x0000FF00UL
#define FW_PLATFORM_DERFUSB23E0X  0x00000300UL
#define FW_PLATFORM_AVR           0x00000500UL
#define FW_PLATFORM_R21           0x00000700UL

struct DE_Product
{
    uint16_t vendorId;
    uint16_t productId;
    const char *name;
};

static const DE_Product deProducts[] =
{
    { 0x1cf1, 0x0030, "ConBee II" },
    { 0x0403, 0x6015, "ConBee" }, // FTDI
//    { 0x1cf1, 0x0014, "deNRJ" },
    { 0x1cf1, 0x001d, "deRFnode" },
//    { 0x1cf1, 0x0018, "deRFusb-24E001" },
//    { 0x1cf1, 0x0019, "deRFusb-14E001" },
    { 0x1cf1, 0x001a, "deRFusb-23E00" },
//    { 0x1cf1, 0x001b, "deRFusb-13E00" },
    { 0x1cf1, 0x001c, "deRFnode" },
    { 0x1cf1, 0x0022, "deUSB level shifter" },
    { 0x1cf1, 0x0025, "deRFusb-23E06" },
//    { 0x1cf1, 0x0027, "deRFusb-13E06" },
//    { 0x1cf1, 0x0017, "deRFusb-Sniffer 2.4 GHz" },
//    { 0x1cf1, 0x0023, "deRFusb-Sniffer Sub-GHz" },
//    { 0x1cf1, 0x0001, "Sensor Terminal Board" },
//    { 0x03eb, 0x2125, "Sensor Terminal Board" },
    { 0x0000, 0x0000, nullptr }
};

static deCONZ::DeviceEnumerator *instance_ = nullptr;


namespace deCONZ {
class DeviceEnumeratorPrivate
{
public:
    std::vector<DeviceEntry> devs;
#ifdef Q_OS_LINUX
    QString stableDevicePath; // /dev/serial/by-id/...
#endif
};

DeviceEnumerator::DeviceEnumerator(QObject *parent) :
    QObject(parent),
    d_ptr2(new DeviceEnumeratorPrivate)
{
    instance_ = this;
}

DeviceEnumerator::~DeviceEnumerator()
{
    delete d_ptr2;
    d_ptr2 = nullptr;
    instance_ = nullptr;
}

DeviceEnumerator *DeviceEnumerator::instance()
{
    return instance_;
}

const std::vector<DeviceEntry> &DeviceEnumerator::getList() const
{
    Q_ASSERT(d_ptr2 != nullptr);
    return d_ptr2->devs;
}

#ifdef Q_OS_LINUX
bool tryCreateDeviceFile(const QString &path)
{
    if (QFile::exists(path))
    {
        DBG_Printf(DBG_INFO_L2, "dev file exists: %s\n", qPrintable(path));
        return true;
    }

    int ret = -2; // mknod returns -1 on error
    bool ok = false;
    int minor = path.rightRef(1).toInt(&ok);
    const mode_t mode = S_IFCHR | 0660;

    QFile devicesList(QLatin1String("/sys/fs/cgroup/devices/devices.list"));

    bool allow166 = false;
    bool allow188 = false;

    if (devicesList.open(QIODevice::ReadOnly | QIODevice::Unbuffered | QIODevice::Text))
    {
        QByteArray arr = devicesList.readAll();
        QTextStream stream(arr);
        while (!stream.atEnd())
        {
            const auto line = stream.readLine();
            if (!allow166 && line.contains(QLatin1String("c 166:*")))
            {
                allow166 = true;
            }
            else if (!allow188 && line.contains(QLatin1String("c 188:*")))
            {
                allow188 = true;
            }
        }
        devicesList.close();
    }

    if (!allow166 && !allow188)
    {
        DBG_Printf(DBG_INFO, "dev can't create file: %s via mknod (missing permissions)\n", qPrintable(path));
    }
    else if (!ok)
    {
        DBG_Printf(DBG_ERROR, "error creating: %s,unsupported path\n", qPrintable(path));
    }
    else if (allow166 && path.contains(QLatin1String("ttyACM")))
    {
        ret = mknod(qPrintable(path), mode, makedev(166, minor));
    }
    else if (allow188 && path.contains(QLatin1String("ttyUSB")))
    {
        ret = mknod(qPrintable(path), mode, makedev(188, minor));
    }

    if (ret == 0)
    {
        DBG_Printf(DBG_INFO, "created: %s\n", qPrintable(path));
    }
    else if (ret == -1)
    {
        auto err = errno;
        DBG_Printf(DBG_ERROR, "error creating: %s, %s (%d)\n", qPrintable(path), strerror(err), err);
    }

    return ret == 0;
}
#endif

QString DEV_StableDevicePath(const QString &path)
{
#ifdef Q_OS_LINUX
    QDir devDir("/dev/serial/by-id");
    const auto entries = devDir.entryInfoList();

    QString path1 = path;

    {
        const QFileInfo fi(path1);
        if (fi.isSymLink())
        {
            path1 = fi.canonicalFilePath();
        }
    }

    for (const auto &fi : entries)
    {
        if (fi.isSymLink() && fi.symLinkTarget() == path1)
        {
            return fi.absoluteFilePath();
        }
    }
#endif
    return path;
}

#ifdef USE_QEXT_SERIAL
bool DeviceEnumerator::listSerialPorts()
{
    Q_D(DeviceEnumerator);

    Q_ASSERT(d_ptr != nullptr);
    d->devs.clear();

    const QString comPort = deCONZ::appArgumentString(QLatin1String("--dev"), QString());

#ifdef Q_OS_LINUX
    // ConBee II
    // /dev/serial/by-id/usb-dresden_elektronik_ingenieurtechnik_GmbH_ConBee_II_DE1948474-if00
    if (d->stableDevicePath.isEmpty() && !comPort.isEmpty())
    {
        d->stableDevicePath = DEV_StableDevicePath(comPort);

        if (!d->stableDevicePath.startsWith(QLatin1String("/dev/serial/by-id")))
        {
            d->stableDevicePath.clear(); // only interested in those
        }

        if (!d->stableDevicePath.isEmpty())
        {
            DBG_Printf(DBG_INFO, "COM: use stable device path %s\n", qPrintable(d->stableDevicePath));
        }
    }
    QFileInfo fiComPort(comPort);
#else
    QFileInfo fiComPort;
#endif


    const auto availPorts = QSerialPortInfo::availablePorts();
    auto i = availPorts.begin();
    const auto end = availPorts.end();

    for (; i != end; ++i)
    {
        DBG_Printf(DBG_INFO_L2, "COM: %s : %s (0x%04X/0x%04X)\n", qPrintable(i->systemLocation()), qPrintable(i->description()), i->vendorIdentifier(), i->productIdentifier());

        if ((i->vendorIdentifier() == 0x1cf1) ||  // dresden elektronik
            (i->vendorIdentifier() == 0x0403) ||  // FTDI
             i->systemLocation().contains(QLatin1String("ttyAMA")) ||
             i->systemLocation().contains(QLatin1String("ttyUSB")) ||
             i->systemLocation().contains(QLatin1String("ttyACM")) ||
             i->systemLocation().contains(QLatin1String("ttyS")))
        {
            DeviceEntry dev;
            bool found = false;

#ifdef Q_OS_LINUX
            if (i->vendorIdentifier() == 0x1cf1 || i->vendorIdentifier() == 0x0403)
            {
                // inside Docker check if we are allowed to create via cgroups
                if (!QFile::exists(i->systemLocation()) && tryCreateDeviceFile(i->systemLocation()))
                {

                }
            }
#endif
            if ((i->vendorIdentifier() == 0x1cf1) || // dresden elektronik
                (i->vendorIdentifier() == 0x0403 && i->productIdentifier() == 0x6015)) // FTDI
            {
                dev.idVendor = i->vendorIdentifier();
                dev.idProduct = i->productIdentifier();
                dev.serialNumber = i->serialNumber();
            }
            else if (i->description() == QLatin1String("FT230X Basic UART"))
            {
                dev.idVendor = 0x0403;
                dev.idProduct = 0x6015;
                dev.serialNumber = i->serialNumber();
            }
            // ConBee in Docker environment
            else if (comPort.contains(QLatin1String("ttyUSB")) && i->systemLocation().contains(QLatin1String("ttyUSB")))
            {
                dev.idVendor = 0x0403;
                dev.idProduct = 0x6015;
                dev.serialNumber = i->serialNumber();
#ifdef Q_OS_LINUX
                dev.path = d->stableDevicePath;
#endif
            }
            // ConBee II in Docker environment
            else if (comPort.contains(QLatin1String("ttyACM")) && i->systemLocation().contains(QLatin1String("ttyACM")))
            {
                dev.idVendor = 0x1cf1;
                dev.idProduct = 0x0030;
                dev.serialNumber = i->serialNumber();
                dev.friendlyName = QLatin1String("ConBee II");
#ifdef Q_OS_LINUX
                dev.path = d->stableDevicePath;
#endif
            }

            // Raspberry Pi UART configuration
            // https://www.raspberrypi.org/documentation/configuration/uart.md

            if (i->systemLocation().contains(QLatin1String("ttyAMA")) ||
                i->systemLocation().contains(QLatin1String("ttyS")))
            {
                // Raspbian default UART
                QFileInfo fi("/dev/serial0");
                if (fiComPort.exists() && (i->systemLocation() == comPort || (fiComPort.isSymLink() && fiComPort.symLinkTarget() == i->systemLocation())))
                {
                    DBG_Printf(DBG_INFO, "dev %s\n", qPrintable(i->systemLocation()));
                    dev.friendlyName = QLatin1String("RaspBee");
                    dev.path = comPort;
                    found = true;
                }
                else if (fi.exists() && fi.isSymLink())
                {
                    if (fi.symLinkTarget() == i->systemLocation())
                    {
                        DBG_Printf(DBG_INFO, "dev %s (%s)\n", qPrintable(i->systemLocation()), qPrintable(fi.symLinkTarget()));
                        dev.friendlyName = QLatin1String("RaspBee");
                        found = true;
                    }
                }
                else if (i->systemLocation().contains(QLatin1String("ttyAMA")))
                {
                    DBG_Printf(DBG_INFO, "dev %s\n", qPrintable(i->systemLocation()));
                    dev.friendlyName = QLatin1String("RaspBee");
                    found = true;
                }

                if (deCONZ::ApsController::instance())
                {
                    const auto fwVersion = deCONZ::ApsController::instance()->getParameter(ParamFirmwareVersion);
                    if ((fwVersion & FW_PLATFORM_MASK) == FW_PLATFORM_R21)
                    {
                        dev.friendlyName = QLatin1String("RaspBee II");
                    }
                }
            }
            else if (dev.friendlyName.isEmpty())
            {
                dev.friendlyName = i->portName();
            }

            if (dev.path.isEmpty())
            {
                dev.path = DEV_StableDevicePath(i->systemLocation());
            }

#ifdef Q_OS_LINUX
            // after re-enumeration the stable device path might not be there
            if (dev.path == d->stableDevicePath && !QFile::exists(dev.path))
            {
                dev.path = i->systemLocation();
            }
#endif

            if (!found)
            {
                const DE_Product *descr = deProducts;
                for (; descr->name != nullptr; descr++)
                {
                    // only list supported devices
                    if (dev.idVendor == descr->vendorId &&
                        dev.idProduct == descr->productId)
                    {
                        found = true;
                        dev.friendlyName = QLatin1String(descr->name);
                        break;
                    }
                }
            }

            if (found)
            {
#ifdef Q_OS_LINUX
                if (!comPort.isEmpty() && d->stableDevicePath == dev.path)
                {
                    d->devs.clear();
                    d->devs.push_back(dev);
                    break;
                }
#endif
                d->devs.push_back(dev);
            }
        }
    }

#ifdef Q_OS_LINUX
    // QSerialPortInfo::availablePorts() might fail to detect a device.
    // If a specific device is given via --dev and exists, return it anyway.
    if (d->devs.empty() && !comPort.isEmpty() && QFile::exists(comPort))
    {
        DeviceEntry dev;
        dev.path = comPort;
        dev.friendlyName = QLatin1String("RaspBee"); // could be wrong
        d->devs.push_back(dev);
    }
#endif

    if (d->devs.size() > 1)
    {
        std::stable_sort(d->devs.begin(), d->devs.end(), [](const DeviceEntry &a, const DeviceEntry &b) {
            if (a.idProduct && b.idProduct)
            {
                return a.idVendor == 0x1cf1; // DDEL first
            }

            return a.idProduct != 0; // place USB devices first
        });
    }

    return true;
}
#endif // USE_QEXT_SERIAL
} // namespace deCONZ
