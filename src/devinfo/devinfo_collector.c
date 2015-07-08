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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <servicesync/moat.h>
#include <servicesync/sse/sseversion.h> // Workaround
#include <sseutils.h>
#include <devinfo/devinfo.h>

#define TAG "Devinfo"
#define LOG_ERROR(format, ...) MOAT_LOG_ERROR(TAG, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  MOAT_LOG_WARN(TAG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  MOAT_LOG_INFO(TAG, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) MOAT_LOG_DEBUG(TAG, format, ##__VA_ARGS__)
#define LOG_TRACE(format, ...) MOAT_LOG_TRACE(TAG, format, ##__VA_ARGS__)
#include <stdlib.h>
#define ASSERT(cond) if(!(cond)) { LOG_ERROR("ASSERTION FAILED:" #cond); abort(); }
#define LOG_DEBUG_SSESTRING(label, str) {				\
    sse_char buff[32];							\
    sse_uint len = (sizeof(buff) - 1 < sse_string_get_length(str) ? sizeof(buff) - 1 : sse_string_get_length(str)); \
    sse_strncpy(buff, sse_string_get_cstr(str), len);			\
    buff[len] = '\0';							\
    LOG_DEBUG(#label " = [%s]", buff);					\
  }


static void
DEVINFOCollector_FreeListedSSEString(SSESList *in_list)
{
  SSESList *i;
  while (in_list) {
    i = in_list;
    in_list = sse_slist_unlink(in_list, i);
    sse_string_free(sse_slist_data(i), sse_true);
    sse_slist_free(i);
  }
}

sse_int
TDEVINFOCollector_Initialize(TDEVINFOCollector* self, Moat in_moat)
{
  ASSERT(self);
  ASSERT(in_moat);

  self->fMoat = in_moat;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_INITIALIZED;
  self->fOnGetCallback = NULL;
  self->fUserData = NULL;
  return SSE_E_OK;
}

void
TDEVINFOCollector_Finalize(TDEVINFOCollector* self)
{
  ASSERT(self);
  self->fMoat = NULL;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_FINALIZED;
  self->fOnGetCallback = NULL;
  self->fUserData = NULL;
}

DEVINFOCollectorStatus
TDEVINFOCollector_GetStatus(TDEVINFOCollector* self)
{
  ASSERT(self);
  return self->fStatus;
}

static sse_int
TDEVINFOCollector_ReturnDefaultValue(TDEVINFOCollector* self,
				     DEVINFOCollector_OnGetCallback in_callback,
				     sse_pointer in_user_data,
				     const sse_char* in_value)
{
  MoatValue *value;

  ASSERT(self);
  ASSERT(in_value);

  /* Create a MoatValue */
  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;
  value = moat_value_new_string((sse_char*)in_value, 0, sse_true);
  ASSERT(value);

  /* Call callback */
  if (self->fOnGetCallback) {
    self->fOnGetCallback(value, self->fUserData, SSE_E_OK);
  }

  /* Cleanup */
  moat_value_free(value);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;

  return SSE_E_OK;

  /* Abnormal end */
}

static sse_int
TDEVINFOCollector_ReturnNoEntry(TDEVINFOCollector* self,
				DEVINFOCollector_OnGetCallback in_callback,
				sse_pointer in_user_data)
{
  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;
  if (self->fOnGetCallback) {
    self->fOnGetCallback(NULL, self->fUserData, SSE_E_NOENT);
  }
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;
}

sse_int
TDEVINFOCollector_GetHardwarePlatformVendor(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnNoEntry(self, in_callback, in_user_data);
}

sse_int
TDEVINFOCollector_GetHardwarePlatformProduct(TDEVINFOCollector* self,
					     DEVINFOCollector_OnGetCallback in_callback,
					     sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnNoEntry(self, in_callback, in_user_data);
}

sse_int
TDEVINFOCollector_GetHardwarePlatformModel(TDEVINFOCollector* self,
					   DEVINFOCollector_OnGetCallback in_callback,
					   sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnNoEntry(self, in_callback, in_user_data);
}

sse_int
TDEVINFOCollector_GetHardwarePlatformSerial(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data)
{
  sse_int err;
  sse_char mac_addr[32];
  MoatValue *value = NULL;
  sse_int result = SSE_E_OK;

  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

  /* Use MAC address of primary network interface instead of HW serial number. */
  err = SseUtilNetInfo_GetHwAddressCstr("eth0", mac_addr, sizeof(mac_addr));
  if (err != SSE_E_OK) {
    LOG_WARN("SseUtilNetInfo_GetHwAddressCstr(if=[eth0], ...) ... failed with [%s].", sse_get_error_string(err));
    value = NULL;
    result = SSE_E_NOENT;
  } else {
    value = moat_value_new_string(mac_addr, 0, sse_true);
    ASSERT(value);
    result = SSE_E_OK;
  }
  self->fOnGetCallback(value, self->fUserData, result);

  if (value) {
    moat_value_free(value);
  }

  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;
}

sse_int
TDEVINFOCollector_GetHardwarePlatformHwVersion(TDEVINFOCollector* self,
					       DEVINFOCollector_OnGetCallback in_callback,
					       sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnNoEntry(self, in_callback, in_user_data);
}

sse_int
TDEVINFOCollector_GetHardwarePlatformFwVersion(TDEVINFOCollector* self,
					       DEVINFOCollector_OnGetCallback in_callback,
					       sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnNoEntry(self, in_callback, in_user_data);
}

sse_int
TDEVINFOCollector_GetHardwarePlatformDeviceId(TDEVINFOCollector* self,
					      DEVINFOCollector_OnGetCallback in_callback,
					      sse_pointer in_user_data)
{
  sse_int err;
  MoatValue *value = NULL;
  sse_char *device_id = NULL;
  sse_int result = SSE_E_OK;

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

  /* Get DeviceId */
  err = moat_get_device_id(self->fMoat, &device_id);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_get_device_id() ... failed with [%s].", sse_get_error_string(err));
    value = NULL;
    result = SSE_E_NOENT;
  } else {
    value = moat_value_new_string(device_id, 0, sse_true);
    ASSERT(value);
  }

  /* Call callback */
  if (self->fOnGetCallback) {
    self->fOnGetCallback(value, self->fUserData, result);
  }

  if (value) {
    moat_value_free(value);
  }
  if (device_id) {
    sse_free(device_id);
  }

  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;
}

sse_int
TDEVINFOCollector_GetHardwarePlatformCategory(TDEVINFOCollector* self,
					      DEVINFOCollector_OnGetCallback in_callback,
					      sse_pointer in_user_data)
{
  return TDEVINFOCollector_ReturnDefaultValue(self, in_callback, in_user_data, "Gateway");
}

sse_int
TDEVINFOCollector_GetHardwareModemType(TDEVINFOCollector* self,
				       DEVINFOCollector_OnGetCallback in_callback,
				       sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnNoEntry(self, in_callback, in_user_data);
}


sse_int
TDEVINFOCollector_GetHardwareModemHwVersion(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnNoEntry(self, in_callback, in_user_data);
}


sse_int
TDEVINFOCollector_GetHardwareModemFwVersion(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnNoEntry(self, in_callback, in_user_data);
}

static MoatObject*
DEVINFOCollector_GetNetworkConfiguration(SSEString* in_ifname)
{
  sse_int err;
  MoatObject *object = NULL;
  SSEString *hw_addr = NULL;
  SSEString *ipv4_addr = NULL;
  SSEString *netmask = NULL;
  SSEString *ipv6_addr = NULL;

  ASSERT(in_ifname);

  object = moat_object_new();
  ASSERT(object);

  /* Set interface name */
  err = moat_object_add_string_value(object, DEVINFO_KEY_NET_INTERFACE_NAME, sse_string_get_cstr(in_ifname), sse_string_get_length(in_ifname), sse_true, sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", err);
    goto error_exit;
  }
  LOG_DEBUG_SSESTRING("I/F name", in_ifname);

  /* Set MAC Address */
  err = SseUtilNetInfo_GetHwAddress(in_ifname, &hw_addr);
  if (err != SSE_E_OK) {
    LOG_ERROR("SseUtilNetInfo_GetHwAddress() ... failed with [%s].", err);
    goto error_exit;
  }
  LOG_DEBUG_SSESTRING("MAC address", hw_addr);

  err = moat_object_add_string_value(object, DEVINFO_KEY_NET_INTERFACE_HW_ADDR, sse_string_get_cstr(hw_addr), sse_string_get_length(hw_addr), sse_true, sse_false);
  sse_string_free(hw_addr, sse_true);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", err);
    goto error_exit;
  }

  /* Set IPv4 Address */
  err = SseUtilNetInfo_GetIPv4Address(in_ifname, &ipv4_addr);
  if (err == SSE_E_OK) {
    LOG_DEBUG_SSESTRING("IPv4 address", ipv4_addr);
    err = moat_object_add_string_value(object, DEVINFO_KEY_NET_INTERFACE_IPV4_ADDR, sse_string_get_cstr(ipv4_addr), sse_string_get_length(ipv4_addr), sse_true, sse_false);
    sse_string_free(ipv4_addr, sse_true);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", err);
      goto error_exit;
    }
  } else {
    LOG_WARN("SseUtilNetInfo_GetIpv4Address() ... failed with [%s].", err);
  }

  /* Set netmask */
  err = SseUtilNetInfo_GetIPv4Netmask(in_ifname, &netmask);
  if (err == SSE_E_OK) {
    LOG_DEBUG_SSESTRING("Netmask", netmask);
    err = moat_object_add_string_value(object, DEVINFO_KEY_NET_INTERFACE_NETMASK, sse_string_get_cstr(netmask), sse_string_get_length(netmask), sse_true, sse_false);
    sse_string_free(netmask, sse_true);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", err);
      goto error_exit;
    }
  } else {
    LOG_WARN("SseUtilNetInfo_GetIpv4Address() ... failed with [%s].", err);
  }

  /* Set IPv6 Address */
  err = SseUtilNetInfo_GetIPv6Address(in_ifname, &ipv6_addr);
  if (err == SSE_E_OK) {
    LOG_DEBUG_SSESTRING("IPv6 address", ipv6_addr);
    err = moat_object_add_string_value(object, DEVINFO_KEY_NET_INTERFACE_IPV6_ADDR, sse_string_get_cstr(ipv6_addr), sse_string_get_length(ipv6_addr), sse_true, sse_false);
    sse_string_free(ipv6_addr, sse_true);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", err);
      goto error_exit;
    }
  } else {
    LOG_WARN("SseUtilNetInfo_GetIpv6Address() ... failed with [%s].", err);
  }

  /* Add object to the list */
  return object;

 error_exit:
  if (object) {
    moat_object_free(object);
  }
  return NULL;
}

sse_int
TDEVINFOCollector_GetHardwareNetworkInterface(TDEVINFOCollector* self,
					      DEVINFOCollector_OnGetCallback in_callback,
					      sse_pointer in_user_data)
{
  sse_int err;
  MoatObject *object = NULL;
  MoatValue *value = NULL;
  SSESList *if_list = NULL;
  SSESList *i = NULL;

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

  /* Get network interface information */
  err = SseUtilNetInfo_GetInterfaceList(&if_list);
  if ((err != SSE_E_OK) || (sse_slist_length(if_list) == 0)) {
    if (err == SSE_E_OK) {
      LOG_WARN("No network interface was found.");
    } else {
      LOG_ERROR("SseUtilNetInfo_GetInterfaceList() ... failed with [%s].", sse_get_error_string(err));
    }
    if (self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, SSE_E_NOENT);
    }
    self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
    return SSE_E_OK;
  }

  /* Get detailed interface configuration. */
  for (i = if_list; i != NULL; i = sse_slist_next(i)) {
    object = DEVINFOCollector_GetNetworkConfiguration(sse_slist_data(i));
    if (object == NULL) {
      LOG_ERROR("DEVINFOCollector_GetNetworkConfiguration() ... failed.");
      continue;
    }
    value = moat_value_new_object(object, sse_true);
    ASSERT(value);
    if (self->fOnGetCallback) {
      self->fOnGetCallback(value, self->fUserData, SSE_E_OK);
    }
    moat_value_free(value);
    moat_object_free(object);
  }

  /* Cleanup */
  DEVINFOCollector_FreeListedSSEString(if_list);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;

  return SSE_E_OK;
}

static void
DEVINFOCollector_GetHardwareNetworkNameserverOnComplateCallback(TSseUtilShellCommand* self,
								sse_pointer in_user_data,
								sse_int in_result)
{
  TDEVINFOCollector *collector = (TDEVINFOCollector *)in_user_data;
  ASSERT(collector);

  LOG_INFO("collector=[%p], command=[%s] has been completed.", collector, self->fShellCommand);

  /* Cleanup */
  collector->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  TSseUtilShellCommand_Finalize(self);
}

static void
DEVINFOCollector_GetHardwareNetworkNameserverOnReadCallback(TSseUtilShellCommand* self,
							    sse_pointer in_user_data)
{
  sse_int err;
  sse_char *buff;
  MoatValue *value;

  TDEVINFOCollector *collector = (TDEVINFOCollector *)in_user_data;
  ASSERT(collector);

  LOG_DEBUG("collector=[%p], command=[%s] is readable.", collector, self->fShellCommand);
  while ((err = TSseUtilShellCommand_ReadLine(self, &buff, sse_true)) == SSE_E_OK) {
    LOG_DEBUG("nameserver=[%s]", buff);
    if (sse_strncmp("nameserver", buff, sse_strlen("nameserver")) == 0) {
      sse_char *p;
      p = sse_strchr(buff, ' ');
      if ((p != NULL) && (*(++p) != '\0')) {
	value = moat_value_new_string(p, 0, sse_true);
	ASSERT(value);

	if(collector->fOnGetCallback) {
	  collector->fOnGetCallback(value, collector->fUserData, err);
	}
	moat_value_free(value);
      }
    }
    sse_free(buff);
  }
  if (err != SSE_E_NOENT && err != SSE_E_AGAIN) {
    LOG_ERROR("TSseUtilShellCommand_ReadLine() ... failed with [%s]", sse_get_error_string(err));
    if (self->fOnErrorCallback) {
      self->fOnErrorCallback(self, self->fOnErrorCallbackUserData, err, sse_get_error_string(err));
    }
  }

  return;
}

static void
TDEVINFOCollector_GetHardwareNetworkNameserverOnErrorCallback(TSseUtilShellCommand* self,
							      sse_pointer in_user_data,
							      sse_int in_error_code,
							      const sse_char* in_message)
{
  TDEVINFOCollector *collector = (TDEVINFOCollector *)in_user_data;
  ASSERT(collector);

  LOG_ERROR("collector=[%p], command=[%s] failed with code=[%d], message=[%s]", collector, self->fShellCommand, in_error_code, in_message);
  if(collector->fOnGetCallback) {
      collector->fOnGetCallback(NULL, collector->fUserData, in_error_code);
  }
  collector->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
}

sse_int
TDEVINFOCollector_GetHardwareNetworkNameserver(TDEVINFOCollector* self,
					       DEVINFOCollector_OnGetCallback in_callback,
					       sse_pointer in_user_data)
{
  sse_int err;

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

  /* Create command to read /etc/resolv.conf */
  err = TSseUtilShellCommand_Initialize(&self->fCommand);
  ASSERT(err == SSE_E_OK);
  err = TSseUtilShellCommand_SetShellCommand(&self->fCommand, "cat");
  ASSERT(err == SSE_E_OK);
  err = TSseUtilShellCommand_AddArgument(&self->fCommand, "/etc/resolv.conf"); //TODO fix me.
  ASSERT(err == SSE_E_OK);

  /* Set callbacks */
  err = TSseUtilShellCommand_SetOnComplatedCallback(&self->fCommand, DEVINFOCollector_GetHardwareNetworkNameserverOnComplateCallback, self);
  ASSERT(err == SSE_E_OK);
  err = TSseUtilShellCommand_SetOnReadCallback(&self->fCommand, DEVINFOCollector_GetHardwareNetworkNameserverOnReadCallback, self);
  ASSERT(err == SSE_E_OK);
  err=  TSseUtilShellCommand_SetOnErrorCallback(&self->fCommand,TDEVINFOCollector_GetHardwareNetworkNameserverOnErrorCallback, self);
  ASSERT(err == SSE_E_OK);

  err = TSseUtilShellCommand_Execute(&self->fCommand);
  if (err != SSE_E_OK) {
    LOG_ERROR("TSseUtilShellCommand_Execute() ... failed with [%s].", sse_get_error_string(err));
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
    }
    self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  }

  return SSE_E_OK;
}

sse_int
TDEVINFOCollector_GetHardwareSim(TDEVINFOCollector* self,
				 DEVINFOCollector_OnGetCallback in_callback,
				 sse_pointer in_user_data)
{
  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

#if 0
  if(self->fOnGetCallback) {
    self->fOnGetCallback(NULL, self->fUserData, SSE_E_NOENT);
  }
  return SSE_E_OK;
#else
  /*
   * Following code is just a referrence implementation.
   */

  sse_int err;
  MoatObject *object = NULL;
  MoatValue *value = NULL;

  /* Create MoatObject */
  object = moat_object_new();
  ASSERT(object);
  err = moat_object_add_string_value(object, DEVINFO_KEY_SIM_ICCID, "Unkonwn", sse_strlen("Unkonwn"), sse_true, sse_false); 
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
    }
    goto error_exit;
  }
  err = moat_object_add_string_value(object, DEVINFO_KEY_SIM_IMSI, "Unkonwn", sse_strlen("Unkonwn"), sse_true, sse_false); 
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
    }
    goto error_exit;
  }
  err = moat_object_add_string_value(object, DEVINFO_KEY_SIM_MSISDN, "Unkonwn", sse_strlen("Unkonwn"), sse_true, sse_false); 
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
    }
    goto error_exit;
  }

  /* Call callback */
  if(self->fOnGetCallback) {
    value = moat_value_new_object(object, sse_true);
    ASSERT(value);
    self->fOnGetCallback(value, self->fUserData, err);
    moat_value_free(value);
  }

  /* Cleanup */
  moat_object_free(object);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;

  /* Abnormal end */
 error_exit:
  self->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
  if (object) moat_object_free(object);
  return err;
#endif
}

static sse_int
DEVINFOCollector_GetSoftwareOSType(MoatValue **out_os_type)
{
  FILE *fd;
  sse_char buff[64];
  MoatValue *os_type = NULL;

  fd = fopen(DEVINFO_COLLECTOR_PROCFS_OS_TYPE, "r");
  if (fd == NULL) {
    LOG_ERROR("fopen(" DEVINFO_COLLECTOR_PROCFS_OS_TYPE ") ... failed with [%s].", strerror(errno));
    return SSE_E_ACCES;
  }
  if (fgets(buff, sizeof(buff), fd) == NULL) {
    LOG_ERROR("fgets() ... failed.");
    fclose(fd);
    return SSE_E_ACCES;
  }
  fclose(fd);

  /* Strip CRLF */
  {
    sse_char *p = buff;
    while (*p++ != '\0') {
      if (*p == 0x0d || *p == 0x0a) {
	*p ='\0';
      }
    }
  }

  LOG_DEBUG("OS Type = [%s]", buff);
  os_type = moat_value_new_string(buff, 0, sse_true);
  ASSERT(os_type);
  *out_os_type = os_type;
  return SSE_E_OK;
}

static sse_int
DEVINFOCollector_GetSoftwareOSVersion(MoatValue **out_os_version)
{
  FILE *fd;
  sse_char buff[64];
  MoatValue *os_version = NULL;

  fd = fopen(DEVINFO_COLLECTOR_PROCFS_OS_VERSION, "r");
  if (fd == NULL) {
    LOG_ERROR("fopen(" DEVINFO_COLLECTOR_PROCFS_OS_TYPE ") ... failed with [%s].", strerror(errno));
    return SSE_E_ACCES;
  }
  if (fgets(buff, sizeof(buff), fd) == NULL) {
    LOG_ERROR("fgets() ... failed.");
    fclose(fd);
    return SSE_E_ACCES;
  }
  fclose(fd);

  /* Strip CRLF */
  {
    sse_char *p = buff;
    while (*p++ != '\0') {
      if (*p == 0x0d || *p == 0x0a) {
        *p ='\0';
      }
    }
  }

  LOG_DEBUG("OS Version = [%s]", buff);
  os_version = moat_value_new_string(buff, 0, sse_true);
  ASSERT(os_version);
  *out_os_version = os_version;
  return SSE_E_OK;
}

sse_int
TDEVINFOCollector_GetSoftwareOS(TDEVINFOCollector* self,
				DEVINFOCollector_OnGetCallback in_callback,
				sse_pointer in_user_data)
{
  sse_int err;
  MoatObject *object = NULL;
  MoatValue *os_type = NULL;
  MoatValue *os_version = NULL;
  MoatValue *value = NULL;

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

  /* Get OS Type (e.g. Linux) */
  err = DEVINFOCollector_GetSoftwareOSType(&os_type);
  if (err != SSE_E_OK) {
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
    }
    goto error_exit;
  }

  /* Get OS Version (e.g. 3.2.0-4-amd64) */
  err = DEVINFOCollector_GetSoftwareOSVersion(&os_version);
  if (err != SSE_E_OK) {
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
    }
    goto error_exit;
  }

  /* Create MoatObject */
  object = moat_object_new();
  ASSERT(object);
  /* err = moat_object_add_value(object, DEVINFO_KEY_OS_TYPE, os_type, 0, sse_true); */
  err = moat_object_add_value(object, DEVINFO_KEY_OS_TYPE, os_type, 0, sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }
  /* err = moat_object_add_value(object, DEVINFO_KEY_OS_VERSION, os_version, 0, sse_true); */
  err = moat_object_add_value(object, DEVINFO_KEY_OS_VERSION, os_version, 0, sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }

  /* Call callback */
  if(self->fOnGetCallback) {
    value = moat_value_new_object(object, sse_true);
    ASSERT(value);
    self->fOnGetCallback(value, self->fUserData, err);
    moat_value_free(value);
  }

  /* Cleanup */
  /* os_type and os_version are pointing a heap area allocated by moat_value_new_string().
   * When these MoatValue were added to the MoatObject wiht in_dup=TRUE, invalid free are
   * deteced in moat_object_free().
   * As workaround, add os_type and os_version into the MoatObject with in_dup=FALSE and
   * ommit moat_value_free for os_type and os_version.
   *
   * moat_value_free(os_type);
   * moat_value_free(os_version);
   */
  moat_object_free(object);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;

  /* Abnormal end */
 error_exit:
  if(self->fOnGetCallback) {
    self->fOnGetCallback(NULL, self->fUserData, err);
  }
  self->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
  if (os_type) moat_value_free(os_type);
  if (os_version) moat_value_free(os_version);
  if (object) moat_object_free(object);
  return err;
}

sse_int
TDEVINFOCollector_GetSoftwareSscl(TDEVINFOCollector* self,
				  DEVINFOCollector_OnGetCallback in_callback,
				  sse_pointer in_user_data)
{
  sse_int err;
  const sse_char *sscl_version = NULL;
  const sse_char *moat_sdk_version = NULL;
  MoatObject *object = NULL;
  MoatValue *value = NULL;

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

  sscl_version = sse_get_version();
  ASSERT(sscl_version);
  moat_sdk_version = sse_get_sdk_version();
  ASSERT(moat_sdk_version);

  /* Create MoatObject */
  object = moat_object_new();
  ASSERT(object);
  err = moat_object_add_string_value(object, DEVINFO_KEY_SSCL_TYPE, "SSEGW", sse_strlen("SSEGE"), sse_true, sse_false); 
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
    }
    goto error_exit;
  }
  err = moat_object_add_string_value(object, DEVINFO_KEY_SSCL_VERSION, (sse_char*)sscl_version, sse_strlen(sscl_version), sse_true, sse_false); 
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
    }
    goto error_exit;
  }
  err = moat_object_add_string_value(object, DEVINFO_KEY_SSCL_SDK_VERSION, (sse_char*)moat_sdk_version, sse_strlen(moat_sdk_version), sse_true, sse_false); 
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }

  /* Call callback */
  if(self->fOnGetCallback) {
    value = moat_value_new_object(object, sse_true);
    ASSERT(value);
    self->fOnGetCallback(value, self->fUserData, err);
    moat_value_free(value);
  }

  /* Cleanup */
  moat_object_free(object);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;

  /* Abnormal end */
 error_exit:
  if(self->fOnGetCallback) {
    self->fOnGetCallback(NULL, self->fUserData, err);
  }
  self->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
  if (object) moat_object_free(object);
  return err;
}
