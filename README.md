ServiceSync Embedded Device Info App
===

This application, which works on ServiceSync collects a basic information of the gateway device.

This application has the following components:
* __Server Package:__ https://github.com/inventit/ssjs-devinfo
* __Gateway Package:__ https://github.com/inventit/ssegw-devinfo

# System Requirements

If you want to run this application on your ServiceSync, here's what it takes:
* __ServiceSync Server:__ `v2.2.3` or later
* __ServiceSync Clinet:__ `v1.1.0` or later

Supported gateway devices are as follows:

* __Generic Intel-based PCs:__ Debian `7.x` (Wheezy)
* __Amradillo-IoT / G2:__  Kernel `v3.14-at3` + AtmarkDist `v20150826` or later
* __OpenBlocks:__ Firmware `1.0.4` or later
* __Raspberry Pi:__ Raspbian `May 2015`

# Item List

| Items    |           |               |             | Description                             | Policy   | Generic                                     | Armadillo-IoT                  | OpenBlocks-IoT                           | Raspberry Pi                             |
| -------- | --------- | ------------- | ----------- | --------------------------------------- | -------- | ------------------------------------------- | ------------------------------ | ---------------------------------------- | ---------------------------------------- |
| hardware | platform  | vendor        |             | Name of manufacture/vendor              | MUST     | `/sys/devices/virtual/dmi/id/board_vendor`  | "Atmark Techno, Inc." (FIXED)  | "Plat'Home Co., Ltd. " (FIXED)           | "Raspberry Pi Foundation" (FIXED)        |
|          |           | product       |             | Name of product                      	 | MUST     | `/sys/devices/virtual/dmi/id/board_name`    | "Armadillo-IoT" (FIXED)        | "OpenBlocks IoT" (FIXED)                 | "Raspberry Pi" (FIXED)                   |
|          |           | model         |             | Product code (e.g. AG-40)               | MUST     | `/sys/devices/virtual/dmi/id/board_name`    | "Hardware" in `/proc/cpuinfo`  | `/sys/devices/virtual/dmi/id/board_name` | "Hardware" in `/proc/cpuinfo`            |
|          |           | serial        |             | Serial number                           | MUST     | `/sys/devices/virtual/dmi/id/board_serial`  | "Serial" in `/proc/cpuinfo`    |                                          | "Serial" in `/proc/cpuinfo`              |
|          |           | hwVersion     |             | Hardware version                        | WANT     | `/sys/devices/virtual/dmi/id/board_version` | "Revision" in `/proc/cpuinfo`  | No entry                                 | "Revision" in `/proc/cpuinfo`            |
|          |           | fwVersion     |             | Firmware version                        | MUST     | No entry                                    | `/proc/sys/kernel/version`     | Package version of `kernel-image-obsbx1` | No entry                                 |
|          |           | deviceId      |             | Device id of ServiceSync                | MUST     | `moat_get_device_id()`                      | `moat_get_device_id()`         | `moat_get_device_id()`                   | `moat_get_device_id()`                   |
|          |           | category      |             | Category (e.g. gateway, smart phone)    | WANT     | "Gateway" (FIXED)                           | "Gateway" (FIXED)              | "Gateway" (FIXED)                        | "Gateway" (FIXED)                        |
|          | processor | type          |             | Processor type (e.g. armv5tejl, x86_64) | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | clock         |             | Processor clock                         | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | numProcessor  |             | Number of processors                    | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | numCore       |             | Number of cores                         | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | cacheSize     |             | Cache size                              | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | memory        |             | Size                                    | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          | power     | type		       |             | Power supply (e.g. AC, battery)	       | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          | modem     | type          |             | Modem type                              | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | hwVersion		 |             | Hardware version of modem               | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | fwVersion		 |             | Firmware version of modem               | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | imei          |             |                                         | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | meid          |             |                                         | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          | network   | interface []  | name        | Interface name (e.g. eth0, wlan0, usb0) | WANT     |                                             |                                |                                          |                                          |
|          |           |               | hwAddress   | MAC Address                             | WANT     |                                             |                                |                                          |                                          |
|          |           |               | ipv4Address | IPv4 Address                            | WANT     |                                             |                                |                                          |                                          |
|          |           |               | ipv6Address | IPv6 Address                            | WANT     |                                             |                                |                                          |                                          |
|          |           |               | gateway     | Default gateway                         | RESERVED |                                             |                                |                                          |                                          |
|          |           | nameserver [] |             | DNS Server                              | WANT     | `/etc/resolve.conf`                         | `/etc/config/resolve.conf`     | `/etc/resolve.conf`                      | `/etc/resolve.conf`                      |
|          | sim []    | carrier       |             | Carrier (e.g. NTTDocomo, Vodafone)      | RESERVED | No entry                                    | No entry                       | No entry                                 | No entry                                 |
|          |           | iccid         |             |                                         | WANT     | No entry                                    | No entry                       | `/tmp/.flags/.sim_exist`                 | No entry                                 |
|          |           | msisdn        |             |                                         | WANT     | No entry                                    | `3g-phone-num` command         | `/tmp/.flags/.sim_exist`                 | No entry                                 |
|          |           | imsi          |             |                                         | WANT     | No entry                                    | No entry                       | `/tmp/.flags/.sim_exist`                 | No entry                                 |
| software | os        | type          |             | Type of OS (e.g. Linux, Android)	       | WANT     | `/proc/sys/kernel/ostype`                   | `/proc/sys/kernel/ostype`      | `/proc/sys/kernel/ostype`                | `/proc/sys/kernel/ostype`                |
|          |           | version       |             | Verson of OS                            | WANT     | `/proc/sys/kernel/osrelease`                | `/proc/sys/kernel/osrelease`   | `/proc/sys/kernel/osrelease`             | `/proc/sys/kernel/osrelease`             |
|          | ssclient  | type          |             | Type of ServiceSync Client              | WANT     | "SSEGW" (FIXED)                             | "SSEGW" (FIXED)                | "SSEGW" (FIXED)                          | "SSEGW" (FIXED)                          |
|          |           | version       |             | ServiceSync Client version              | WANT     | `sse_get_version()`                         | `sse_get_version()`            | `sse_get_version()`                      | `sse_get_version()`                      |  
|          |           | sdkVersion    |             | MOAT C SDK version                      | WANT     | `sse_get_sdk_version()`                     | `sse_get_sdk_version()`        | `sse_get_sdk_version()`                  | `sse_get_sdk_version()`                  |
|          |           | config	       |             | ServiceSync Client configuration        |RESERVED  |  No entry                                   | No entry                       | No entry                                 | No entry                                 |

# Setup

To collect a basic gateway information, you need to build the Server Package and the Gateway Package. Then deploy the Server Package to ServiceSync and install the Gateway Package to each gateway device via ServiceSync.

## Server Package

### Preparing Development Environment

To develop a Server Package, `Node.js`, `Grunt` and `Java`(1.6 or later) are required. If these tools even work, you are able to use any Operating System what you like.

In case of using Mac or Linux, you can install `Node.js` with [nodebrew](https://github.com/hokaccha/nodebrew).
The blow is a sample to install Node.js `v0.12.5`.
```
$ nodebrew install-binary v0.12.5
$ nodebrew use v0.12.5
$ node -v
v0.12.5
```

[Grunt](http://gruntjs.com/) is required to build the Server Package. Install Grunt as follows.
```
$ npm install -g grunt-cli
```
Java Runtime is required by Grunt task. Install Java Runtime.

Linux:
```
$ sudo apt-get install openjdk-7-jre
```

Mac or Windows:
* Refer to https://www.java.com/en/download/help/download_options.xml

### Build

Get the source code from GitHub.
```
$ git clone https://github.com/inventit/ssjs-devinfo.git
$ cd ssjs-devinfo
```

Execute the following commands to build the Server Package. The Server Package named `devinfo-${VERSION}.zip` will be generated.
```
$ npm install
$ grunt pack
```

If you want to clean artifacts, execute the following command.
```
$ grunt clean
```

### Deploy

1. Login to ServiceSync Web Console with PP's (Platform Provicer) account.
2. Select [Packages] from the side menu.
3. Open [Server Packages] tab.
4. Click [Add] Button. Pop-up window will appear.
5. Select the Server Package and click [submit] button.
6. Push [Deploy] button in [Detail Info].

## Gateway Package

### Preparing Development Environment

To build a Gateway Packages, Setup Linux environment. The below are required Linux distribution for each product.

* __Intel 64 based PCs:__ Debian 7.x (64-bit)
* __Intel IA-32 based PCs:__ Debian 7.x (32-bit)
* __Armadillo-IoT /G2:__ [ATDE5](http://armadillo.atmark-techno.com/armadillo-iot/downloads)
* __OpenBlocks IoT:__ Debian 7.x (32-bit) (or Self-Development)
* __Raspberry Pi:__ [Raspbian](https://www.raspberrypi.org/downloads/raspbian/) (Self-Development)

Install additional packages to develop a Gateway Package as follows.
```
$ sudo apt-get update
$ sudo apt-get install -y --force-yes \
	git \
	mercurial \
	build-essential \
	gyp \
	zip \
	libssl-dev \
	libz-dev \
	libxml2-dev \
	libev-dev \
	uuid \
	uuid-dev \
	libreadline-dev \
	libudev-dev \
	openjdk-7-jdk
```

In case of using Armadillo-IoT, install the additional packages for cross compile.
```
atde5$ curl -O http://ftp.jp.debian.org/debian/pool/main/libe/libev/libev4_4.11-1_armel.deb
atde5$ curl -O http://ftp.jp.debian.org/debian/pool/main/libe/libev/libev-dev_4.11-1_armel.deb
atde5$ sudo dpkg-cross --build  --arch armel libev4_4.11-1_armel.deb
atde5$ sudo dpkg-cross --build  --arch armel libev-dev_4.11-1_armel.deb
atde5$ sudo dpkg -i libev4-armel-cross_4.11-1_all.deb libev-dev-armel-cross_4.11-1_all.deb
```

### Build

#### Getting source code

Get the source code from GitHub.
```
$ git clone https://github.com/inventit/ssegw-devinfo.git
$ cd ssegw-devinfo
$ git checkout <branch name> # if necessary
$ git submodule init
$ git submodule update
```

#### Getting Platform Certificate (moat.pem)

To sign the Gateway Package, the platform certicate (moat.pem) is required. ServiceSync Server (DMS) has their own certificate, so get it from your SA (System Administrator) in advance.

Copy `moat.pem` into `ssegw-devinfo/certs`.
```
$ cp /path/to/moat.pem ./certs
```

####  Getting token.bin

Generate the token for signing as follows.
```
$ cd package
$ export SSDMS_PREFIX="YOUR SERVICESYNC DMS URL"
$ export APP_ID="YOUR PP's APPLICATION ID"
$ export CLIENT_ID="YOUR PP's CLIENT ID"
$ export CLIENT_SECRET="YOUR PP's PASSWORD"
$ export PACKAGE_ID="devinfo"
$ export TOKEN=`curl -v "${SSDMS_PREFIX}/moat/v1/sys/auth?a=${APP_ID}&u=${CLIENT_ID}&c=${CLIENT_SECRET}" | sed 's/\\\\\//\//g' | sed 's/[{}]//g' | awk -v k="text" '{n=split($0,a,","); for (i=1; i<=n; i++) print a[i]}' | sed 's/\"\:\"/\|/g' | sed 's/[\,]/ /g' | sed 's/\"//g' | grep -w 'accessToken' | cut -d"|" -f2 | sed -e 's/^ *//g' -e 's/ *$//g'`
$ curl -v -o token.bin -L "${SSDMS_PREFIX}/moat/v1/sys/package/${PACKAGE_ID}?token=${TOKEN}&secureToken=true"
$ cd ..
```

#### Packaging

Build a Gateway package as follows. You will be able to get the Gateway Package named  `devinfo_${VERSION}_${ARCH}_${PRODUCT}.zip`.

__Generic Intel-based PCs:__
```
debian$ ./configure
debian$ make package
```

__Armadillo-IoT:__
```
atde5$ export CROSS=arm-linux-gnueabi-
atde5$ export CC=${CROSS}gcc
atde5$ export CXX=${CROSS}g++
atde5$ export AR=${CROSS}ar
atde5$ export LD=${CROSS}ld
atde5$ export RANLIB=${CROSS}ranlib
atde5$ export STRIP=${CROSS}strip
atde5$ ./configure --dest-cpu=arm --product=Armadillo-IoT
atde5$ make package
```

__OpenBlocks IoT:__
```
debian$ ./configure --dest-cpu=ia32 --product=OpenBlocks-IoT
debian$ make package
```

__Raspberry Pi:__
```
pi$ ./configure --dest-cpu=arm --product=RaspberryPi
pi$ make package
```

### Deploy

1. Login to ServiceSync Web Console with PP's (Platform Provicer) account.
2. Select [Packages] from the side menu.
3. Open [Gateway Packages] tab.
4. Click [Add] Button. Pop-up window will appear.
5. Select the server package and push [submit] button.
6. Click [Deploy] button in [Detail Info].

### Install

1. Login to ServiceSync Web Console with PU's (Platform User) account.
2. Select [Packages] from the side menu.
3. Open [Gateway Packages] tab.
5. Select the Gateway Package and click [Install] button in [Detail Info].
6. Select the devices and click [Install] button. Pop-up window will appear.
7. Click [OK] to start installation.

# Usage

1. Login to ServiceSync Web Console with PU's (Platform User) account.
2. Select [Devices] from the side menu.
3. Select the device which you want to fetch a file from [Device List]
4. Click [Sync] in [Detail info].
5. Pop-up window will appear. Click [OK] to start collectiong.
6. Device information will be displayed in the below box.


# Limitation

## Armadillo-IoT

* A serial number always will be "0000000000000000", because Armadillo-IoT does not have a serial number in the device.


# Known Bugs

* No bug as of now

# Change History

## Server Package

### `2.0.0` September 25, 2015

* ServiceSync 2.x support
* Armadillo-IoT support
* OpenBlocks IoT support
* Raspberry Pi support

### `1.0.0` April 9, 2015

* Initial Release

## Gateway Package

### `2.0.0` September 25, 2015

* ServiceSync 2.x support
* Armadillo-IoT support
* OpenBlocks IoT support
* Raspberry Pi support

### `0.0.1` April 1, 2015

* Initial Release
