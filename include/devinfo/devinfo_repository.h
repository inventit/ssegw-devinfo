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

void
TDEVINFORepository_Reset(TDEVINFORepository* self);

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

/**
 * @brief Set a vendor (manufacture) name.
 *
 * Set a vendor (manufacture name) name of M2M gatway device to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_vendor vendor name
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwarePlatformVendor(TDEVINFORepository* self,
					     MoatValue* in_vendor);

/**
 * @brief Set a product name.
 *
 * Set a product name of M2M gatway device to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_product product name
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwarePlatformProduct(TDEVINFORepository* self,
					      MoatValue* in_product);

/**
 * @brief Set a model name.
 *
 * Set a model name of M2M gatway device to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_model model name
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwarePlatformModel(TDEVINFORepository* self,
					    MoatValue* in_model);
/**
 * @brief Set a serial number.
 *
 * Set a serial number of M2M gatway device to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_serial serial number
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwarePlatformSerial(TDEVINFORepository* self,
					     MoatValue* in_serial);

/**
 * @brief Set a hardware version
 *
 * Set a hardware version of M2M gatway device to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_hw_version hardware version
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwarePlatformHwVersion(TDEVINFORepository* self,
						MoatValue* in_hw_version);

/**
 * @brief Set a firmware version
 *
 * Set a firmware version of M2M gatway device to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_fw_version firmware version
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwarePlatformFwVersion(TDEVINFORepository* self,
						MoatValue* in_fw_version);

/**
 * @brief Set a devince id
 *
 * Set a device id of M2M gatway device assigned for ServiceSync to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_device_id device id
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwarePlatformDeviceId(TDEVINFORepository* self,
					       MoatValue* in_device_id);

/**
 * @brief Set a category of device
 *
 * Set a category of device to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_hw_version hardware version
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwarePlatformCategory(TDEVINFORepository* self,
					       MoatValue* in_category);

/**
 * @brief Set a modem type
 *
 * Set a modem type to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_type modem type
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwareModemType(TDEVINFORepository* self,
					MoatValue* in_type);

/**
 * @brief Set a hardware version of modem
 *
 * Set a hardware version of modem to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_hw_version modem h/w version
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwareModemHwVersion(TDEVINFORepository* self,
					     MoatValue* in_hw_version);

/**
 * @brief Set a firmware version of modem
 *
 * Set a firmware version of modem to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_fw_version modem f/w version
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetHardwareModemFwVersion(TDEVINFORepository* self,
					     MoatValue* in_fw_version);

/**
 * @brief Add a network interface
 *
 * Add a network interface configuration to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_name interface name
 * @param [in] in_hw_addess MAC address
 * @param [in] in_ipv4_addess IPv4 address
 * @param [in] in_netmask Subnetmask
 * @param [in] in_ipv6_addess IPv6 address
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_AddHardwareNetworkInterface(TDEVINFORepository* self,
					       MoatValue* in_name,
					       MoatValue* in_hw_address,
					       MoatValue* in_ipv4_address,
					       MoatValue* in_netmask,
					       MoatValue* in_ipv6_address);

/**
 * @brief Remove a network interface
 *
 * Remove a network interface from the repository
 *
 * @param [in] self pointer to instance
 * @param [in] in_name interface name should be removed
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_RemoveHardwareNetworkInterface(TDEVINFORepository* self,
						  MoatValue* in_name);

/**
 * @brief Add a name server
 *
 * Add a name server to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_nameserver name server address
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_AddHardwareNetworkNameserver(TDEVINFORepository* self,
						MoatValue* in_nameserver);

/**
 * @brief Remove a name server
 *
 * Remove a name server to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_nameserver name server address should be removed
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_RemoveHardwareNetworkNameserver(TDEVINFORepository* self,
						   MoatValue* in_nameserver);

/**
 * @brief Add a sim information
 *
 * Add a sim information to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_iccid ICCID
 * @param [in] in_imsi IMSI
 * @param [in] in_msisdn MSISDN
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_AddHardwareSim(TDEVINFORepository* self,
				  MoatValue* in_iccid,
				  MoatValue* in_imsi,
				  MoatValue* in_msisdn);

/**
 * @brief Remove a sim information
 *
 * Remove a sim information to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_iccid ICCID
 * @param [in] in_imsi IMSI
 * @param [in] in_msisdn MSISDN
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_RemoveHardwareSim(TDEVINFORepository* self,
				     MoatValue* in_iccid,
				     MoatValue* in_imsi,
				     MoatValue* in_msisdn);

/**
 * @brief Set a OS information
 *
 * Set a OS information to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_type OS type (e.g. Linux)
 * @param [in] in_version OS version (e.g. 3.2.0-4-amd64)
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetSoftwareOS(TDEVINFORepository* self,
				 MoatValue* in_type,
				 MoatValue* in_version);

/**
 * @brief Set a ServiceSync Client version
 *
 * Set a ServiceSync Client version to the repository.
 *
 * @param [in] self pointer to instance
 * @param [in] in_type ServiceSync Client type (FIXED "SSEGW")
 * @param [in] in_version ServiceSync Client version
 * @param [in] in_sdk_version MOAT C SDK version
 *
 * @retval SSE_E_OK Success
 * @retval others   Failuer
 */
sse_int
TDEVINFORepository_SetSoftwareSscl(TDEVINFORepository* self,
				   MoatValue* in_type,
				   MoatValue* in_version,
				   MoatValue* in_sdk_version);

SSE_END_C_DECLS

#endif /* __DEVINFO_REPOSITORY_H__ */
