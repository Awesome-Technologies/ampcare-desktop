# AMPcare Desktop Client

Travis OSX build: [![Build Status](https://travis-ci.org/Awesome-Technologies/ampcare-desktop.svg?branch=master)](https://travis-ci.org/Awesome-Technologies/ampcare-desktop)

The :computer: AMPcare Desktop Client is a tool for nursing staff to get medical assistance from a doctor.

![Desktop Client on Mac OS](https://cloud.awesome-technologies.de/s/KeSWLNbeLocqc8D/preview)

## :hammer_and_wrench: How to compile the desktop client

:building_construction: [System requirements](https://github.com/nextcloud/desktop/wiki/System-requirements-for-compiling-the-desktop-client) includes OpenSSL 1.1.x, QtKeychain, Qt 5.x.x and zlib.

### :memo: Step by step instructions

#### Clone the repo and create build directory
```
$ git clone https://github.com/awesome-technologies/ampcare-desktop.git
$ cd ampcare-desktop
$ mkdir build
$ cd build
```
#### Compile and install

For development reasons it is better to install the client on user space
instead on the global system. For example you could use in the next
instructions `path-to-install-folder/` as `~/.local/` in a linux system. If
you want to install system wide you could use `/usr/local` or `/opt/ampcare/`.

##### Linux

```
$ cmake .. -DOEM_THEME_DIR=../theme/amp -DCMAKE_INSTALL_PREFIX=path-to-install-folder/ -DCMAKE_BUILD_TYPE=Debug -DNO_SHIBBOLETH=1 -DQTKEYCHAIN_LIBRARY=/path-to-qt5keychain-folder/lib64/libqt5keychain.so -DQTKEYCHAIN_INCLUDE_DIR=/path-to-qt5keychain-folder/include/qt5keychain/ -DOPENSSL_ROOT_DIR=/path-to-openssl-folder/ -DOPENSSL_INCLUDE_DIR=path-to-openssl-folder/include -DOPENSSL_LIBRARIES=path-to-openssl-folder/lib
$ make install
```

##### Windows

```
$ cmake -G "Visual Studio 15 2017 Win64" .. -DOEM_THEME_DIR=../theme/amp -DCMAKE_INSTALL_PREFIX=path-to-install-folder/ -DCMAKE_BUILD_TYPE=Debug -DNO_SHIBBOLETH=1 -DPng2Ico_EXECUTABLE=/path-to-install-png2ico/png2ico.exe  -DQTKEYCHAIN_LIBRARY=/path-to-qt5keychain-folder/lib/qt5keychain.lib -DQTKEYCHAIN_INCLUDE_DIR=/path-to-qt5keychain-folder/include/qt5keychain/ -DOPENSSL_ROOT_DIR=/path-to-openssl-folder/ -DOPENSSL_INCLUDE_DIR=path-to-openssl-folder/include -DOPENSSL_LIBRARIES=path-to-openssl-folder/lib
$ cmake --build . --config Debug --target install
```

##### Mac OS

```
$ brew install qt5
$ brew install openssl@1.1
$ brew install qtkeychain
$ cmake .. -DOEM_THEME_DIR=../theme/amp -DCMAKE_INSTALL_PREFIX=path-to-install-folder/ -DCMAKE_BUILD_TYPE=Debug -DNO_SHIBBOLETH=1 -DQTKEYCHAIN_LIBRARY=/usr/local/opt/qtkeychain/lib/libqt5keychain.dylib -DQTKEYCHAIN_INCLUDE_DIR=/usr/local/opt/qtkeychain/include/qt5keychain -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl@1.1
$ make install
```

## :inbox_tray: Where to find binaries to download

### Releases

- Refer to the [release page https://github.com/Awesome-Technologies/ampcare-desktop/releases](https://github.com/Awesome-Technologies/ampcare-desktop/releases)

## Reporting issues

- If you find any bugs or have any suggestion for improvement, please
file an issue at https://github.com/Awesome-Technologies/ampcare-desktop/issues.

## :v: Code of conduct

The Nextcloud community has core values that are shared between all members during conferences, hackweeks and on all interactions in online platforms including [Github](https://github.com/nextcloud) and [Forums](https://help.nextcloud.com). If you contribute, participate or interact with this community, please respect [our shared values](https://nextcloud.com/code-of-conduct/). :relieved:

## :scroll: License

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
    for more details.
