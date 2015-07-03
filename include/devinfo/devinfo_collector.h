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

#ifndef __DEVINFO_COLLECTOR_H__
#define __DEVINFO_COLLECTOR_H__

SSE_BEGIN_C_DECLS

#define DEVINFO_COLLECTOR_PROCFS_OS_TYPE     "/proc/sys/kernel/ostype"
#define DEVINFO_COLLECTOR_PROCFS_OS_VERSION  "/proc/sys/kernel/osrelease"


enum DEVINFOCollectorStatus_ {
  DEVINFO_COLLECTOR_STATUS_INITIALIZED = 0,
  DEVINFO_COLLECTOR_STATUS_COLLECTING,
  DEVINFO_COLLECTOR_STATUS_PUBLISHING,
  DEVINFO_COLLECTOR_STATUS_COMPLETED,
  DEVINFO_COLLECTOR_STATUS_FINALIZED,
  DEVINFO_COLLECTOR_STATUS_ABEND,
  DEVINFO_COLLECTOR_STATUSs /* terminator */
};
typedef enum DEVINFOCollectorStatus_ DEVINFOCollectorStatus;

typedef void(*DEVINFOCollector_OnGetCallback)(MoatObject* in_collected,
					      sse_pointer in_user_data,
					      sse_int in_err_code);

struct TDEVINFOCollector_ {
  Moat fMoat;
  DEVINFOCollectorStatus fStatus;
  DEVINFOCollector_OnGetCallback fOnGetCallback;
  sse_pointer fUserData;
};
typedef struct TDEVINFOCollector_ TDEVINFOCollector;

sse_int
TDEVINFOCollector_Initialize(TDEVINFOCollector* self,
			     Moat in_moat);

void
TDEVINFOCollector_Finalize(TDEVINFOCollector* self);

DEVINFOCollectorStatus
TDEVINFOCollector_GetStatus(TDEVINFOCollector* self);

sse_int
TDEVINFOCollector_GetHardwarePlatformVendor(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwarePlatformProduct(TDEVINFOCollector* self,
					     DEVINFOCollector_OnGetCallback in_callback,
					     sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwarePlatformModel(TDEVINFOCollector* self,
					   DEVINFOCollector_OnGetCallback in_callback,
					   sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwarePlatformSerial(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwarePlatformHwVersion(TDEVINFOCollector* self,
					       DEVINFOCollector_OnGetCallback in_callback,
					       sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwarePlatformFwVersion(TDEVINFOCollector* self,
					       DEVINFOCollector_OnGetCallback in_callback,
					       sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwarePlatformDeviceId(TDEVINFOCollector* self,
					      DEVINFOCollector_OnGetCallback in_callback,
					      sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwarePlatformCategory(TDEVINFOCollector* self,
					      DEVINFOCollector_OnGetCallback in_callback,
					      sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwareModemType(TDEVINFOCollector* self,
				       DEVINFOCollector_OnGetCallback in_callback,
				       sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwareModemHwVersion(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHardwareModemFwVersion(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetHadwareNetworkInterface(TDEVINFOCollector* self,
					     DEVINFOCollector_OnGetCallback in_callback,
					     sse_pointer in_user_data);

sse_int
TDEVINFOCollector_GetSoftwareOS(TDEVINFOCollector* self,
				DEVINFOCollector_OnGetCallback in_callback,
				sse_pointer in_user_data);

SSE_END_C_DECLS

#endif /* __DEVINFO_COLLECTOR_H__ */

