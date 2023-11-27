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
#include <QTextStream>
#include <QTimer>
#include <vector>

#ifdef __linux__
  #define PL_LINUX 1
#endif

#ifdef USE_QEXT_SERIAL
  #include <QSerialPortInfo>
#endif // USE_QEXT_SERIAL

#ifdef PL_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <unistd.h>
/* for popen to check udevadm */
#include <sys/stat.h>
#include <dirent.h>
#endif

#include "deconz/aps_controller.h"
#include "deconz/dbg_trace.h"
#include "deconz/device_enumerator.h"
#include "deconz/util.h"
#include "deconz/u_sstream.h"

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
#ifdef PL_LINUX
    QString stableDevicePath; // /dev/serial/by-id/...
#endif
};

#ifdef PL_LINUX
static bool hasUdevadm = false;

static bool initHasUdevadm(void)
{
    FILE *f;
    size_t n;
    U_SStream ss;

    unsigned i;
    char buf[128];
    long udevadm_version;

    udevadm_version = 0;
    /* check if udevadm is available */
    f = popen("udevadm --version", "r");
    if (f)
    {
        n = fread(&buf[0], 1, sizeof(buf) - 1, f);
        if (n > 0 && n < 10)
        {
            buf[n] = '\0';
            U_sstream_init(&ss, &buf[0], (unsigned)n);
            udevadm_version = U_sstream_get_long(&ss);
            if (ss.status != U_SSTREAM_OK)
                udevadm_version = 0;
        }
        pclose(f);
    }

    if (udevadm_version == 0)
        return false;

    return true;
}

/*  Returns /dev/ttyUSBx | /dev/ttyACMx for a serial number
    This works also when /dev/by-id .. is symlinks aren't available

    udevadm info --name=/dev/ttyACM0

    query: /dev/by-id/.. path or simply any string containing the serial number
    ss_out: here the device path like  /dev/ttyACMx path will be placed if matched

*/
static int query_udevadm_dev_path_of_serial(const char *query, U_SStream *ss_out)
{
    if (!hasUdevadm)
        return 0;

    FILE *f;
    size_t n;
    U_SStream ss;
    U_SStream ss_query;
    char ch;
    char buf[4096 * 4];
    char path[256];
    char query_path[256];
    char serial[64];
    unsigned i;
    unsigned dev_major;

    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    U_sstream_init(&ss_query, query_path, sizeof(query_path));
    U_sstream_put_str(&ss_query, query);

    dir = opendir("/dev");

    if (!dir)
        return 0;

    serial[0] = '\0';

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_CHR)
            continue;

        U_sstream_init(&ss, &path[0], sizeof(path));
        U_sstream_put_str(&ss, "/dev/");
        U_sstream_put_str(&ss, &entry->d_name[0]);

        if (stat(ss.str, &statbuf) != 0)
            continue;

        dev_major = statbuf.st_rdev >> 8;
        if (dev_major != 166 && dev_major != 188) /* ConBee I / II / III */
            continue;

        U_sstream_init(&ss, &buf[0], sizeof(buf));
        U_sstream_put_str(&ss, "udevadm info --name=");
        U_sstream_put_str(&ss, "/dev/");
        U_sstream_put_str(&ss, &entry->d_name[0]);

        f = popen(ss.str, "r");
        if (f)
        {
            n = fread(&buf[0], 1, sizeof(buf) - 1, f);
            pclose(f);

            if (n > 0 && n < sizeof(buf) - 1)
            {
                buf[n] = '\0';

                DBG_WriteString(DBG_INFO_L2, &buf[0]);

                U_sstream_init(&ss, &buf[0], (unsigned)n);
                while (U_sstream_find(&ss, "E: ") && U_sstream_remaining(&ss) > 8)
                {
                    ss.pos += 3;

                    if (U_sstream_starts_with(&ss, "ID_USB_SERIAL_SHORT=") && U_sstream_find(&ss, "="))
                    {
                        i = 0;
                        ss.pos += 1;
                        serial[0] = '\0';

                        for (;ss.pos < ss.len && i + 1 < sizeof(serial); ss.pos++, i++)
                        {
                            ch = U_sstream_peek_char(&ss);
                            if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))
                            {
                                serial[i] = ch;
                                serial[i + 1] = '\0';
                            }
                            else
                            {
                                break;
                            }
                        }

                        ss_query.pos = 0;
                        if (serial[0] && U_sstream_find(&ss_query, &serial[0]))
                        {
                            break;
                        }

                        serial[0] = '\0';
                    }
                }
            }
        }

        if (serial[0])
            break;
    }

    closedir(dir);

    if (serial[0] && path[0])
    {
        U_sstream_put_str(ss_out, &path[0]);
    }

    return ss_out->pos != 0 ? 1 : 0;
}

/*  Fills in all USB device info from udev
    This replaces QSerialPortInfo on Linux.

    udevadm info --name=/dev/ttyACM0
*/
static int query_udevadm_dev_info(const char *dev_path, deCONZ::DeviceEntry *dev)
{
    if (!hasUdevadm)
        return 0;

    FILE *f;
    size_t n;
    U_SStream ss;
    U_SStream ss_query;
    char ch;
    char buf[4096 * 4];
    char serial[64];
    unsigned i;
    unsigned dev_major;
    struct stat statbuf;

    int matches = 0;

    if (stat(dev_path, &statbuf) != 0)
        return 0;

    dev_major = statbuf.st_rdev >> 8;
    if (dev_major != 166 && dev_major != 188) /* ConBee I / II / III */
        return 0;

    U_sstream_init(&ss, &buf[0], sizeof(buf));
    U_sstream_put_str(&ss, "udevadm info --name=");
    U_sstream_put_str(&ss, dev_path);

    f = popen(ss.str, "r");
    if (f)
    {
        n = fread(&buf[0], 1, sizeof(buf) - 1, f);
        pclose(f);

        if (n > 0 && n < sizeof(buf) - 1)
        {
            buf[n] = '\0';

            dev->path = QString::fromLatin1(dev_path);

            // DBG_WriteString(DBG_INFO_L2, &buf[0]);

            U_sstream_init(&ss, &buf[0], (unsigned)n);
            while (U_sstream_find(&ss, "E: ") && U_sstream_remaining(&ss) > 8)
            {
                ss.pos += 3;

                if (U_sstream_starts_with(&ss, "ID_USB_MODEL_ID=") && U_sstream_find(&ss, "="))
                {
                    ss.pos += 1;
                    dev->idProduct = strtoull(&ss.str[ss.pos], 0, 16);
                }
                else if (U_sstream_starts_with(&ss, "ID_USB_VENDOR_ID=") && U_sstream_find(&ss, "="))
                {
                    ss.pos += 1;
                    dev->idVendor = strtoull(&ss.str[ss.pos], 0, 16);
                }
                else if (U_sstream_starts_with(&ss, "ID_USB_MODEL=") && U_sstream_find(&ss, "="))
                {
                    ss.pos += 1;
                    unsigned start = ss.pos;

                    if (U_sstream_find(&ss, "\n") && ss.pos > start && (ss.pos - start) < 32)
                    {
                        for (int k = start; k < ss.pos; k++) // replace underscores with spaces
                        {
                            if (ss.str[k] == '_')
                                ss.str[k] = ' ';
                        }

                        dev->friendlyName = QString::fromLatin1(&ss.str[start], ss.pos - start);
                    }
                }
                else if (U_sstream_starts_with(&ss, "ID_USB_SERIAL_SHORT=") && U_sstream_find(&ss, "="))
                {
                    i = 0;
                    ss.pos += 1;
                    serial[0] = '\0';

                    for (;ss.pos < ss.len && i + 1 < sizeof(serial); ss.pos++, i++)
                    {
                        ch = U_sstream_peek_char(&ss);
                        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')
                        {
                            serial[i] = ch;
                            serial[i + 1] = '\0';
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (serial[0])
                    {
                        // break;
                        dev->serialNumber = QString::fromLatin1(serial);
                    }

                    serial[0] = '\0';
                }
            }
        }
    }

    if (dev->idVendor == 0x1cf1 && dev->idProduct == 0x0030) // ConBee II
    {
        dev->baudrate = 115200;
        return 1;
    }

    if (dev->friendlyName == QLatin1String("ConBee III"))
    {
        dev->baudrate = 115200;
        return 1;
    }

    if (dev->idVendor == 0x0403 && dev->idProduct == 0x6015)
    {
        if (dev->friendlyName == QLatin1String("FT230X Basic UART"))
        {
            dev->friendlyName = QLatin1String("ConBee");
            dev->baudrate = 38400; // ConBee I
            return 1;
        }
    }

    *dev = {};
    return 0;
}
#endif // PL_LINUX

DeviceEnumerator::DeviceEnumerator(QObject *parent) :
    QObject(parent),
    d_ptr2(new DeviceEnumeratorPrivate)
{
    instance_ = this;

#ifdef PL_LINUX
    hasUdevadm = initHasUdevadm();
#endif
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

#ifdef PL_LINUX
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
        DBG_Printf(DBG_INFO_L2, "dev can't create file: %s via mknod (missing permissions)\n", qPrintable(path));
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

// Fallback when the /dev/serial/by-id/... path doesn't exist or points into the void.
// Lookup the serial number in /dev/serial/by-id/usb-dresden_elektronik_ingenieurtechnik_GmbH_ConBee_II_DE1948474-if00
// via udevadm
QString DEV_ResolvedDevicePath(const QString &path)
{
#ifdef PL_LINUX
    if (path.startsWith(QLatin1String("/dev/serial/by-id")))
    {
        QFileInfo fi(path);
        if (fi.exists() && fi.isSymLink() && QFile::exists(fi.symLinkTarget()))
        {
            return fi.canonicalFilePath();
        }

        // fallback udevadm
        U_SStream ssOut;
        char resolvedPath[256] = {0};
        U_sstream_init(&ssOut, &resolvedPath[0], sizeof(resolvedPath));
        if (query_udevadm_dev_path_of_serial(qPrintable(path), &ssOut))
        {
            return QString(&resolvedPath[0]);
        }

    }
#endif
    return QString();
}

// /dev/ttyACM0
// /dev/serial/by-id/usb-dresden_elektronik_ingenieurtechnik_GmbH_ConBee_II_DE1948474-if00
QString DEV_StableDevicePath(const QString &path)
{
#ifdef PL_LINUX
    if (path.startsWith(QLatin1String("/dev/serial/by-id"))) // todo this could be any symlink or string containing serial number
    {
        QString path1;
        path1 = DEV_ResolvedDevicePath(path);
        if (!path1.isEmpty())
        {
            // symlink might not exist but can be resolved
            return path;
        }
    }

    // reverse: try do find symlink for /dev/ttyAMCx devices
    QDir devDir("/dev/serial/by-id");
    const auto entries = devDir.entryInfoList();

    for (const auto &fi : entries)
    {
        if (fi.isSymLink() && fi.symLinkTarget() == path && QFile::exists(path))
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

#ifdef PL_LINUX
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
#endif

    const auto availPorts = QSerialPortInfo::availablePorts();
    auto i = availPorts.begin();
    const auto end = availPorts.end();

    for (; i != end; ++i)
    {
        if (DBG_IsEnabled(DBG_INFO_L2))
        {
            DBG_Printf(DBG_INFO_L2, "COM: %s : %s : %s (0x%04X/0x%04X)\n",
                       qPrintable(i->systemLocation()), qPrintable(i->description()),
                       qPrintable(i->manufacturer()), i->vendorIdentifier(), i->productIdentifier());
        }

        if ((i->vendorIdentifier() == 0x1cf1) ||  // dresden elektronik
            (i->vendorIdentifier() == 0x0403) ||  // FTDI
             i->systemLocation().contains(QLatin1String("ttyAMA")) ||
             i->systemLocation().contains(QLatin1String("ttyUSB")) ||
             i->systemLocation().contains(QLatin1String("ttyACM")) ||
             i->systemLocation().contains(QLatin1String("ttyS")))
        {
            DeviceEntry dev {};
            bool found = false;

#ifdef PL_LINUX
//            if (i->vendorIdentifier() == 0x1cf1 || i->vendorIdentifier() == 0x0403)
//            {
//                // inside Docker check if we are allowed to create via cgroups
//                if (!QFile::exists(i->systemLocation()) && tryCreateDeviceFile(i->systemLocation()))
//                {

//                }
//            }

            if (query_udevadm_dev_info(qPrintable(i->systemLocation()), &dev))
            {
                found = true;
            }
            else
#endif
            if ((i->vendorIdentifier() == 0x1cf1) || // dresden elektronik
                (i->vendorIdentifier() == 0x0403 && i->productIdentifier() == 0x6015)) // FTDI
            {
                dev.idVendor = i->vendorIdentifier();
                dev.idProduct = i->productIdentifier();
                dev.serialNumber = i->serialNumber();

                if (i->description() == QLatin1String("ConBee III"))
                {
                    dev.friendlyName = i->description();
                    dev.baudrate = 115200;
                    found = true;
                }
#ifdef _WIN32
                // TODO use code from GCFFlasher4, this is here is only a workaround
                if (i->description() == QLatin1String("USB Serial Port"))
                {
                    if (i->serialNumber().startsWith(QLatin1String("DE")))
                    {
                        dev.friendlyName = QLatin1String("ConBee III");
                        dev.baudrate = 115200;
                    }
                    else
                    {
                        dev.friendlyName = QLatin1String("ConBee");
                        dev.baudrate = 38400;
                    }

                    found = true;
                }
#endif
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
#ifdef PL_LINUX
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
                dev.baudrate = 115200;
#ifdef PL_LINUX
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
                QFileInfo fiComPort(comPort);

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

#ifdef PL_LINUX
            if (dev.path.isEmpty() && !d->stableDevicePath.isEmpty())
            {
                dev.path = d->stableDevicePath;
            }
            else
#endif
            {
                dev.path = DEV_StableDevicePath(i->systemLocation());
            }

#ifdef PL_LINUX
            // after re-enumeration the stable device path might not be there
            if (dev.path == d->stableDevicePath)
            {
                QString path1 = DEV_ResolvedDevicePath(d->stableDevicePath);
                if (path1.isEmpty() || !QFile::exists(path1))
                {
                    dev.path = i->systemLocation();
                }
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
#ifdef PL_LINUX
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

#ifdef PL_LINUX
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
