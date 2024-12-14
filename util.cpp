/*
 * Copyright (c) 2012-2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QtGlobal>
#include <QCoreApplication>
#include <QDir>
#include <QDesktopServices>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <limits.h>
#include <stdlib.h>
#include <memory>
#include "deconz/dbg_trace.h"
#include "deconz/util.h"

#ifdef PL_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <limits.h>
#include <intrin.h>
typedef unsigned __int32  uint32_t;

#else
#include <stdint.h>
#endif

/*! \ingroup utils

    @{
 */

static QString resolvePath(const QString &origPath)
{
#ifdef PL_UNIX
    QString path = origPath;
    if (!path.isEmpty())
    {
        std::unique_ptr<char[]> realp(new char[PATH_MAX]);

        if (path.startsWith('~'))
        {
            path.replace('~', QDir::homePath());
        }

        const char *p = realpath(qPrintable(path), realp.get());

        if (p && strlen(p) > 0)
        {
            path = p;
        }
    }
    return path;
#else
    return origPath;
#endif
}

class CPUID {
  uint32_t regs[4] = {0};

public:
  explicit CPUID(unsigned i) {
#ifdef PL_WINDOWS
    __cpuid((int *)regs, (int)i);

#elif defined(Q_PROCESSOR_X86)
    asm volatile
      ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
       : "a" (i), "c" (0));
    // ECX is set to zero for CPUID function 4
#else
      (void)i;
#endif
  }

  const uint32_t &EAX() const {return regs[0];}
  const uint32_t &EBX() const {return regs[1];}
  const uint32_t &ECX() const {return regs[2];}
  const uint32_t &EDX() const {return regs[3];}
};

namespace deCONZ {

/*! @addtogroup utils

    @{
 */


/*! Get the value of a commandline argument

    \param arg the commandline argument
    \param defaultValue
    \return number found or default value
 */
int appArgumentNumeric(const QString &arg, int defaultValue)
{
    QStringList args = QCoreApplication::arguments();
    QStringList::iterator i = args.begin();
    QStringList::iterator end = args.end();

    for (int n = 0; i != end; ++i, n++)
    {
        if (i->startsWith(arg))
        {
            QStringList ls = i->split('=');

            if (!ls.isEmpty() && (ls[0] != arg))
            {
                continue;
            }

            if (ls.size() == 2 && !ls[1].isEmpty())
            {
                bool ok;
                int num = ls[1].toInt(&ok);

                if (ok)
                {
                    return num;
                }
                else
                {
                    DBG_Printf(DBG_INFO, "Invalid numeric app argument %s\n", qPrintable(ls[1]));
                }
            }
            else
            {
                DBG_Printf(DBG_INFO, "Invalid app argument %s\n", qPrintable(*i));
            }

            break;
        }
    }

    return defaultValue;
}

/*! Get the value of a commandline argument

    \param arg the commandline argument
    \param defaultValue
    \return number found or default value
 */
QString DECONZ_DLLSPEC appArgumentString(const QString &arg, const QString &defaultValue)
{
    QStringList args = QCoreApplication::arguments();
    QStringList::iterator i = args.begin();
    QStringList::iterator end = args.end();

    for (int n = 0; i != end; ++i, n++)
    {
        if (i->startsWith(arg))
        {
            QStringList ls = i->split('=');

            if (!ls.isEmpty() && (ls[0] != arg))
            {
                continue;
            }

            if (ls.size() == 2 && !ls[1].isEmpty())
            {
                return ls[1];
            }
            else
            {
                DBG_Printf(DBG_INFO, "Invalid app argument %s\n", qPrintable(*i));
            }

            break;
        }
    }

    return defaultValue;
}

QString getStorageLocation(StorageLocation location)
{
    QString path;

    switch (location)
    {
    case HomeLocation:
    {
        path = QDir::homePath();
    }
        break;

    case ApplicationsLocation:
    {
        QDir dir(QCoreApplication::applicationDirPath()); // leave /bin
        dir.cdUp();
        path = dir.absolutePath();
        //path = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation);
        path.replace("//", "/");
    }
        break;

    case ApplicationsDataLocation:
    {
        path = deCONZ::appArgumentString(QLatin1String("--appdata"), QLatin1String(""));
        if (!path.isEmpty())
        {
            return resolvePath(path);
        }

        bool found = false;
        static const char *paths[] = {
#ifdef PL_LINUX
            "/.local/share/data/dresden-elektronik/deCONZ",
            "/.local/share/dresden-elektronik/deCONZ",
            "/.local/share/deCONZ",
#endif
#ifdef PL_WINDOWS
            "/AppData/Local/dresden-elektronik/deCONZ",
#endif
            0
        };

        auto loc =  QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (!loc.isEmpty())
        {
            path = loc.first();
        }

        for (int i = 0; paths[i]; i++)
        {
            if (QFile::exists(path + QLatin1String(paths[i])))
            {
                path += QLatin1String(paths[i]);
                found = true;
                break;
            }
        }

        if (!found)
        {
            loc = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
            if (!loc.isEmpty())
            {
                path = loc.first();
            }
        }

        path.replace("//", "/");
    }
        break;

    case DdfLocation:
    {
#ifdef PL_LINUX
        path = deCONZ::appArgumentString(QLatin1String("--ddf-root"),
                                         QLatin1String("/usr/share/deCONZ/devices"));
#elif defined (PL_WINDOWS)
        path = deCONZ::appArgumentString(QLatin1String("--ddf-root"),
                                         deCONZ::getStorageLocation(deCONZ::ApplicationsLocation) + QLatin1String("/devices"));
#elif defined (PL_MACOS)
        path = deCONZ::appArgumentString(QLatin1String("--ddf-root"),
                                         deCONZ::getStorageLocation(deCONZ::ApplicationsLocation) + QLatin1String("/Resources/devices"));
#else
        path = deCONZ::appArgumentString(QLatin1String("--ddf-root"),
                                         deCONZ::getStorageLocation(deCONZ::ApplicationsDataLocation) + QLatin1String("/devices"));
#endif
    }
        break;

    case DdfBundleLocation:
    {
#ifdef PL_LINUX
        path = QLatin1String("/usr/share/deCONZ/bundles");
#elif defined (PL_WINDOWS)
        path = deCONZ::getStorageLocation(deCONZ::ApplicationsLocation) + QLatin1String("/bundles");
#elif defined (PL_MACOS)
        path = deCONZ::getStorageLocation(deCONZ::ApplicationsLocation) + QLatin1String("/Resources/bundles");
#else
        path = deCONZ::getStorageLocation(deCONZ::ApplicationsDataLocation) + QLatin1String("/bundles");
#endif
    }
        break;

    case DdfUserLocation:
    {
        path = deCONZ::getStorageLocation(ApplicationsDataLocation) + QLatin1String("/devices");
    }
        break;

    case DdfBundleUserLocation:
    {
        path = deCONZ::getStorageLocation(ApplicationsDataLocation) + QLatin1String("/bundles");
    }
        break;

    case DocumentsLocation:
    {
        auto loc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (!loc.isEmpty())
        {
            path = loc.first();
        }
    }
        break;

    case ZcldbLocation:
        path = deCONZ::getStorageLocation(ApplicationsDataLocation) + "/zcldb.txt";
        break;

    case ConfigLocation:
        path = deCONZ::getStorageLocation(ApplicationsDataLocation) + "/config.ini";
        break;

    case NodeCacheLocation:
        path = deCONZ::getStorageLocation(ApplicationsDataLocation) + "/session.default";
        break;

    case RuntimeLocation:
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        if (env.contains(QLatin1String("XDG_RUNTIME_DIR")))
        {
            path = env.value("XDG_RUNTIME_DIR") + QLatin1String("/deconz");
        }
    }
    break;


    default:
        break;
    }

    return resolvePath(path);
}

bool isVirtualMachine()
{
    CPUID cpuid(1);

    // https://kb.vmware.com/s/article/1009458
    return cpuid.ECX() & (1 << 31); // hypervisor bit set
}

//! @} end of group utils
} // namespace deCONZ

/*! \brief Convert utf-8 to unicode code point.

    Returns pointer to remainder of text. 'codepoint' is a valid codepoint
    or set to U_INVALID_UNICODE_CODEPOINT for invalid utf8.
 */
const char * U_utf8_codepoint(const char *text, unsigned *codepoint)
{
    unsigned cp;

    cp = (unsigned)*text & 0xFF;
    text++;

    if ((cp & 0x80) == 0)
    {
        // 1-byte ASCII
    }
    else if ((cp & 0xE0) == 0xC0 && text[0] != 0) /*  110 prefix 2-byte char */
    {
        /* 110x xxxx 10xx xxxx */
        cp &= 0x1F;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
    }
    else if ((cp & 0xF0) == 0xE0 && text[0] != 0 && text[1] != 0) /*  1110 prefix 3-byte char */
    {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        cp &= 0x0F;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
    }
    else if ((cp & 0xF8) == 0xF0 && text[0] != 0 && text[1] != 0 && text[2] != 0) /*  11110 prefix 4-byte char */
    {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        cp &= 0x07;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
        cp <<= 6;
        cp |= (unsigned)*text & 0x3F;
        text++;
    }
    else
    {
        cp = U_INVALID_UNICODE_CODEPOINT;
    }

    *codepoint = cp;

    return text;
}
