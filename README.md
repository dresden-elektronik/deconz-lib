# deCONZ Library

This library is used by deCONZ core and plugins.
The compiled library is part of the deCONZ package.

Doxgen generated documentation: [https://phoscon.de/deconz-cpp](https://phoscon.de/deconz-cpp/index.html)

**Work in Progress**

Previously the library was a fixed part of deCONZ and build via QMake. The goal of this repository is making the library code Open Source and to replace the deconz-dev.deb package. This repository will be referenced in the REST-API plugin CMake file to automatically fetch the C/C++ headers.

## Building on Linux (Debian / Ubuntu)

1. The following packages need to be installed:

```
apt-get install --no-install-recommends \
    build-essential \
    qt5-default \
    libqt5serialport5-dev \
    libqt5websockets5-dev \
    qtdeclarative5-dev  \
    sqlite3 \
    libsqlite3-dev \
    libssl-dev \
    pkg-config \
    cmake \
    ninja-build
```

2. Configure and compile

```
cmake -G Ninja -S . -B build
cmake --build build

```

## Building on Windows

1. Qt5 for MSVC and Visual Studio 2019 needs to be installed

2. Configure and compile

```
cmake -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019\lib\cmake\Qt5 -A Win32 -S . -B build
```
