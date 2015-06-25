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

#ifndef __DEVINFO_REPOSITORY_H__
#define __DEVINFO_REPOSITORY_H__

SSE_BEGIN_C_DECLS

struct TDEVINFORepository_ {
  Moat fMoat;
  MoatObject* fDevinfo;
};
typedef struct TDEVINFORepository_ TDEVINFORepository;

sse_int
TDEVINFORepository_Initialize(TDEVINFORepository* self,
			      Moat in_moat);

void
TDEVINFORepository_Finalize(TDEVINFORepository* self);

sse_int
TDEVINFORepository_LoadDevinfo(TDEVINFORepository* self,
			       SSEString *in_path);

sse_int
TDEVINFORepository_GetDevinfoWithJson(TDEVINFORepository* self,
				      SSEString *in_key,
				      SSEString **out_devinfo);

sse_int
TDEVINFORepository_GetDevinfo(TDEVINFORepository* self,
			      SSEString* in_key,
			      MoatValue** out_value);

sse_int
TDEVINFORepository_SetDevinfo(TDEVINFORepository* self,
			      SSEString* in_key,
			      MoatValue* in_value);

sse_int
TDEVINFORepository_SetHardwarePlatformVendor(TDEVINFORepository* self,
					     SSEString* in_vendor);

sse_int
TDEVINFORepository_SetHardwarePlatformProduct(TDEVINFORepository* self,
					      SSEString* in_product);

sse_int
TDEVINFORepository_SetHardwarePlatformModel(TDEVINFORepository* self,
					    SSEString* in_model);

sse_int
TDEVINFORepository_SetHardwarePlatformSerial(TDEVINFORepository* self,
					     SSEString* in_serial);

sse_int
TDEVINFORepository_SetHardwarePlatformHwVersion(TDEVINFORepository* self,
						SSEString* in_hw_version);

sse_int
TDEVINFORepository_SetHardwarePlatformFwVersion(TDEVINFORepository* self,
						SSEString* in_fw_version);

sse_int
TDEVINFORepository_SetHardwarePlatformDeviceId(TDEVINFORepository* self,
					       SSEString* in_device_id);

sse_int
TDEVINFORepository_SetHardwarePlatformCategory(TDEVINFORepository* self,
					       SSEString* in_category);

sse_int
TDEVINFORepository_SetHardwareModemType(TDEVINFORepository* self,
					SSEString* in_type);

sse_int
TDEVINFORepository_SetHardwareModemHwVersion(TDEVINFORepository* self,
					     SSEString* in_hw_version);

sse_int
TDEVINFORepository_SetHardwareModemfwVersion(TDEVINFORepository* self,
					     SSEString* in_fw_version);

sse_int
TDEVINFORepository_AddHadwareNetworkInterface(TDEVINFORepository* self,
					      SSEString* in_name,
					      SSEString* in_hw_address,
					      SSEString* in_ipv4_address,
					      SSEString* in_netmask,
					      SSEString* in_ipv6_address);

sse_int
TDEVINFORepository_RemoveHardwareNetworkInterface(TDEVINFORepository* self,
						  SSEString* in_name);

sse_int
TDEVINFORepository_AddHardwareNetworkNameserver(TDEVINFORepository* self,
						SSEString* in_nameserver);

sse_int
TDEVINFORepository_RemoveHardwareNetworkNameserver(TDEVINFORepository* self,
						   SSEString* in_nameserver);

sse_int
TDEVINFORepository_AddHardwareSim(TDEVINFORepository* self,
				  SSEString* in_iccid,
				  SSEString* in_imsi,
				  SSEString* in_msisdn);

sse_int
TDEVINFORepository_RemoveHardwareSim(TDEVINFORepository* self,
				     SSEString* in_iccid,
				     SSEString* in_imsi,
				     SSEString* in_msisdn);

sse_int
TDEVINFORepository_SetSoftwareOS(TDEVINFORepository* self,
				 SSEString* in_type,
				 SSEString* in_version);

sse_int
TDEVINFORepository_SetSoftwareSscl(TDEVINFORepository* self,
				   SSEString* in_type,
				   SSEString* in_version,
				   SSEString* in_sdk_version);

SSE_END_C_DECLS

#endif /* __DEVINFO_REPOSITORY_H__ */
