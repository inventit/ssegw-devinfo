/*
 * LEGAL NOTICE
 *
 * Copyright (C) 2012-2015 InventIt Inc. All rights reserved.
 *
 * This source code, product and/or document is protected under licenses
 * restricting its use, copying, distribution, and decompilation.
 * No part of this source code, product or document may be reproduced in
 * any form by any means without prior written authorization of InventIt Inc.
 * and its licensors, if any.
 *
 * InventIt Inc.
 * 9F KOJIMACHI CP BUILDING
 * 4-4-7 Kojimachi, Chiyoda-ku, Tokyo 102-0083
 * JAPAN
 * http://www.yourinventit.com/
 */

#ifndef __DEVINFO_H__
#define __DEVINFO_H__

SSE_BEGIN_C_DECLS

#define DEVINFO_KEY_VENDOR "hardware.platform.vendor"
#define DEVINFO_KEY_PRODUCT "hardware.platform.product"
#define DEVINFO_KEY_MODEL "hardware.platform.model"
#define DEVINFO_KEY_SERIAL "hardware.platform.serial"
#define DEVINFO_KEY_HW_VERSION "hardware.platform.hwVersion"
#define DEVINFO_KEY_FW_VERSION "hardware.platform.hwVersion"
#define DEVINFO_KEY_DEVICE_ID "hardware.platform.deviceId"
#define DEVINFO_KEY_CATEGORY "hardware.platform.category"
#define DEVINFO_KEY_MODEM_TYPE "hardware.modem.type"
#define DEVINFO_KEY_MODEM_HW_VERSION "hardware.modem.hwVersion"
#define DEVINFO_KEY_MODEM_FW_VERSION "hardware.modem.fwVersion"
#define DEVINFO_KEY_NET_INTERFACE "hardware.network.interface"
#define DEVINFO_KEY_NET_NAMESERVER "hardware.network.nameserver"
#define DEVINFO_KEY_SIM_ICCID "hardware.sim.iccid"
#define DEVINFO_KEY_SIM_IMSI "hardware.sim.imsi"
#define DEVINFO_KEY_SIM_MSISDN "hardware.sim.msisdn"
#define DEVINFO_KEY_OS_TYPE "software.os.type"
#define DEVINFO_KEY_OS_VERSION "software.os.version"

#include <devinfo/devinfo_repository.h>
#include <devinfo/devinfo_collector.h>

SSE_END_C_DECLS

#endif /* __DEVINFO_H__ */
