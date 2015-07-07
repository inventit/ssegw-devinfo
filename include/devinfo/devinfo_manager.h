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

#ifndef __DEVINFO_MANAGER_H__
#define __DEVINFO_MANAGER_H__

SSE_BEGIN_C_DECLS

enum DEVINFOManagerState_ {
  DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED = 0,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_VENDOR,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_PRODUCT,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_MODEL,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_SERIAL,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_HW_VERSION,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_FW_VERSION,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_DEVICE_ID,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_CATEGORY,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_MODEM_TYPE,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_MODEM_HW_VERSION,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_MODEM_FW_VERSION,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_NETWORK_INTERFACE,
  DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_NETWORK_NAMESERVER,
  DEVINFO_MANAGER_STATE_COLLECTION_DONE,
  DEVINFO_MANAGER_STATEs /* terminator */
};
typedef enum DEVINFOManagerState_ DEVINFOManagerState;

struct TDEVINFOMagager_ {
  Moat fMoat;
  DEVINFOManagerState fState;
  TDEVINFOCollector fCollector;
  TDEVINFORepository fRepository;
  MoatTimer *fStateMonitor;
  sse_int fStateMonitorTimerId;
  sse_int fStateMonitorInterval;
};
typedef struct TDEVINFOMagager_ TDEVINFOManager;

sse_int
TDEVINFOManager_Initialize(TDEVINFOManager *self,
			   Moat in_moat);

void
TDEVINFOMamager_Finalize(TDEVINFOManager *self);

sse_int
TDEVINFOManager_Collect(TDEVINFOManager *self);

sse_int
TDEVINFOManager_Progress(TDEVINFOManager *self);


SSE_END_C_DECLS

#endif  /* __DEVINFO_MANAGER_H__ */
