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

static void
DEVINFOCollector_FreeListedMoatObject(SSESList *in_list)
{
  SSESList *i;
  while (in_list) {
    i = in_list;
    in_list = sse_slist_unlink(in_list, i);
    LOG_DEBUG("list=[%p], data=[%p]", i, sse_slist_data(i));
    moat_object_free(sse_slist_data(i));
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

sse_int
TDEVINFOCollector_GetHardwarePlatformVendor(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data)
{
  sse_int err;
  MoatObject *object;

  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;

  /* Create vendor info "Unknown" in stead of collectiong. */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;
  object = moat_object_new();
  ASSERT(object);

  err = moat_object_add_string_value(object,
				     DEVINFO_KEY_VENDOR,
				     "Unknown",
				     sse_strlen("Unknown"),
				     sse_true,
				     sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }

  /* Call callback */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_PUBLISHING;
  if (self->fOnGetCallback) {
    LOG_DEBUG("Call callback = [%p]", self->fOnGetCallback);
    self->fOnGetCallback(object, self->fUserData, SSE_E_OK);
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
}

static sse_int
TDEVINFOCollector_ReturnDefaultValue(TDEVINFOCollector* self,
				     DEVINFOCollector_OnGetCallback in_callback,
				     sse_pointer in_user_data,
				     const sse_char* in_key,
				     const sse_char* in_value)
{
  sse_int err;
  MoatObject *object;

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);
  ASSERT(in_key);
  ASSERT(in_value);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;

  /* Create vendor info "Unknown" in stead of collectiong. */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;
  object = moat_object_new();
  ASSERT(object);

  err = moat_object_add_string_value(object, (sse_char*)in_key, (sse_char*)in_value, sse_strlen(in_value), sse_true, sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }

  /* Call callback */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_PUBLISHING;
  if (self->fOnGetCallback) {
    LOG_DEBUG("Call callback = [%p]", self->fOnGetCallback);
    self->fOnGetCallback(object, self->fUserData, SSE_E_OK);
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
}


sse_int
TDEVINFOCollector_GetHardwarePlatformProduct(TDEVINFOCollector* self,
					     DEVINFOCollector_OnGetCallback in_callback,
					     sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnDefaultValue(self, in_callback, in_user_data, DEVINFO_KEY_PRODUCT, "Unknown");
}

sse_int
TDEVINFOCollector_GetHardwarePlatformModel(TDEVINFOCollector* self,
					   DEVINFOCollector_OnGetCallback in_callback,
					   sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnDefaultValue(self, in_callback, in_user_data, DEVINFO_KEY_MODEL, "Unknown");
}

sse_int
TDEVINFOCollector_GetHardwarePlatformSerial(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data)
{
  sse_int err;
  sse_char mac_addr[32];
  MoatObject *object = NULL;

  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;

  /* Use MAC address of primary network interface instead of HW serial number. */
  err = SseUtilNetInfo_GetHwAddressCstr("eth0", mac_addr, sizeof(mac_addr));
  if (err != SSE_E_OK) {
    /* In case of error, pass "Unknown" */
    LOG_WARN("SseUtilNetInfo_GetHwAddressCstr(if=[eth0], ...) ... failed with [%s].", sse_get_error_string(err));
    sse_strcpy(mac_addr, "Unknown");
  }

  object = moat_object_new();
  err = moat_object_add_string_value(object, DEVINFO_KEY_SERIAL, mac_addr, sse_strlen(mac_addr), sse_true, sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_string_value(key=[%s], value=[%s], ...) ... failed with [%s].", DEVINFO_KEY_SERIAL, mac_addr, sse_get_error_string(err));
    goto error_exit;
  }

  /* Call callback */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_PUBLISHING;
  if (self->fOnGetCallback) {
    LOG_DEBUG("Call callback = [%p]", self->fOnGetCallback);
    self->fOnGetCallback(object, self->fUserData, SSE_E_OK);
  }

  /* Cleanup */
  moat_object_free(object);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;

  /* Abnormal end */
 error_exit:
  self->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
  if (object) moat_object_free(object);
  return err;				     
}

sse_int
TDEVINFOCollector_GetHardwarePlatformHwVersion(TDEVINFOCollector* self,
					       DEVINFOCollector_OnGetCallback in_callback,
					       sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnDefaultValue(self, in_callback, in_user_data, DEVINFO_KEY_HW_VERSION, "Unknown");
}

sse_int
TDEVINFOCollector_GetHardwarePlatformFwVersion(TDEVINFOCollector* self,
					       DEVINFOCollector_OnGetCallback in_callback,
					       sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnDefaultValue(self, in_callback, in_user_data, DEVINFO_KEY_FW_VERSION, "Unknown");
}

sse_int
TDEVINFOCollector_GetHardwarePlatformDeviceId(TDEVINFOCollector* self,
					      DEVINFOCollector_OnGetCallback in_callback,
					      sse_pointer in_user_data)
{
  sse_int err;
  MoatObject *object = NULL;
  sse_char *device_id = NULL;

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;

  /* Get DeviceId */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;
  object = moat_object_new();
  ASSERT(object);

  err = moat_get_device_id(self->fMoat, &device_id);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_get_device_id() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }

  err = moat_object_add_string_value(object, DEVINFO_KEY_DEVICE_ID, device_id, sse_strlen(device_id), sse_true, sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }

  /* Call callback */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_PUBLISHING;
  if (self->fOnGetCallback) {
    LOG_DEBUG("Call callback = [%p]", self->fOnGetCallback);
    self->fOnGetCallback(object, self->fUserData, SSE_E_OK);
  }

  /* Cleanup */
  moat_object_free(object);
  sse_free(device_id);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;

  return SSE_E_OK;

  /* Abnormal end */
 error_exit:
  self->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
  if (object) moat_object_free(object);
  if (device_id) sse_free(device_id);
  return err;
}

sse_int
TDEVINFOCollector_GetHardwarePlatformCategory(TDEVINFOCollector* self,
					      DEVINFOCollector_OnGetCallback in_callback,
					      sse_pointer in_user_data)
{
  return TDEVINFOCollector_ReturnDefaultValue(self, in_callback, in_user_data, DEVINFO_KEY_CATEGORY, "Gateway");
}

sse_int
TDEVINFOCollector_GetHardwareModemType(TDEVINFOCollector* self,
				       DEVINFOCollector_OnGetCallback in_callback,
				       sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnDefaultValue(self, in_callback, in_user_data, DEVINFO_KEY_MODEM_TYPE, "Unknown");
}


sse_int
TDEVINFOCollector_GetHardwareModemHwVersion(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnDefaultValue(self, in_callback, in_user_data, DEVINFO_KEY_MODEM_HW_VERSION, "Unknown");
}


sse_int
TDEVINFOCollector_GetHardwareModemFwVersion(TDEVINFOCollector* self,
					    DEVINFOCollector_OnGetCallback in_callback,
					    sse_pointer in_user_data)
{
  LOG_WARN("This function should not be called. Concrete function for each gateways should be called.");
  return TDEVINFOCollector_ReturnDefaultValue(self, in_callback, in_user_data, DEVINFO_KEY_MODEM_FW_VERSION, "Unknown");
}

static SSESList*
DEVINFOCollector_AddHadwareNetworkInterface(SSESList* in_list,
					    SSEString* in_ifname)
{
  sse_int err;
  MoatObject *object = NULL;
  SSEString *hw_addr = NULL;
  SSEString *ipv4_addr = NULL;
  SSEString *netmask = NULL;
  SSEString *ipv6_addr = NULL;
  SSESList *list = NULL;

  ASSERT(in_ifname);

  object = moat_object_new();
  ASSERT(object);

  /* Set interface name */
  err = moat_object_add_string_value(object, "name", sse_string_get_cstr(in_ifname), sse_string_get_length(in_ifname), sse_true, sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", err);
    goto error_exit;
  }
  { /* DEBUG */
    sse_char buff[32];
    sse_uint len = sizeof(buff) - 1 < sse_string_get_length(in_ifname) ? sizeof(buff) - 1 : sse_string_get_length(in_ifname);
    sse_strncpy(buff, sse_string_get_cstr(in_ifname), len);
    buff[len] = '\0';
    LOG_DEBUG("I/F name = [%s]", buff);
  }

  /* Set MAC Address */
  err = SseUtilNetInfo_GetHwAddress(in_ifname, &hw_addr);
  if (err != SSE_E_OK) {
    LOG_ERROR("SseUtilNetInfo_GetHwAddress() ... failed with [%s].", err);
    goto error_exit;
  }
  { /* DEBUG */
    sse_char buff[32];
    sse_uint len = sizeof(buff) - 1 < sse_string_get_length(hw_addr) ? sizeof(buff) - 1 : sse_string_get_length(hw_addr);
    sse_strncpy(buff, sse_string_get_cstr(hw_addr), len);
    buff[len] = '\0';
    LOG_DEBUG("MAC Address = [%s]", buff);
  }
  err = moat_object_add_string_value(object, "hwAddress", sse_string_get_cstr(hw_addr), sse_string_get_length(hw_addr), sse_true, sse_false);
  sse_string_free(hw_addr, sse_true);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", err);
    goto error_exit;
  }

  /* Set IPv4 Address */
  err = SseUtilNetInfo_GetIPv4Address(in_ifname, &ipv4_addr);
  if (err == SSE_E_OK) {
    { /* DEBUG */
      sse_char buff[32];
      sse_uint len = sizeof(buff) - 1 < sse_string_get_length(ipv4_addr) ? sizeof(buff) - 1 : sse_string_get_length(ipv4_addr);
      sse_strncpy(buff, sse_string_get_cstr(ipv4_addr), len);
      buff[len] = '\0';
      LOG_DEBUG("IPv4 Address = [%s]", buff);
    }
    err = moat_object_add_string_value(object, "ipv4Address", sse_string_get_cstr(ipv4_addr), sse_string_get_length(ipv4_addr), sse_true, sse_false);
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
    { /* DEBUG */
      sse_char buff[32];
      sse_uint len = sizeof(buff) - 1 < sse_string_get_length(netmask) ? sizeof(buff) - 1 : sse_string_get_length(netmask);
      sse_strncpy(buff, sse_string_get_cstr(netmask), len);
      buff[len] = '\0';
      LOG_DEBUG("Netmask = [%s]", buff);
    }
    err = moat_object_add_string_value(object, "netmask", sse_string_get_cstr(netmask), sse_string_get_length(netmask), sse_true, sse_false);
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
    { /* DEBUG */
      sse_char buff[32];
      sse_uint len = sizeof(buff) - 1 < sse_string_get_length(ipv6_addr) ? sizeof(buff) - 1 : sse_string_get_length(ipv6_addr);
      sse_strncpy(buff, sse_string_get_cstr(ipv6_addr), len);
      buff[len] = '\0';
      LOG_DEBUG("IPv6 Address = [%s]", buff);
    }
    err = moat_object_add_string_value(object, "ipv6Address", sse_string_get_cstr(ipv6_addr), sse_string_get_length(ipv6_addr), sse_true, sse_false);
    sse_string_free(ipv6_addr, sse_true);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", err);
      goto error_exit;
    }
  } else {
    LOG_WARN("SseUtilNetInfo_GetIpv6Address() ... failed with [%s].", err);
  }

  /* Add object to the list */
  list = sse_slist_add(in_list, object);
  ASSERT(list);
  return list;

 error_exit:
  if (object) {
    moat_object_free(object);
  }
  return NULL;
}

sse_int
TDEVINFOCollector_GetHadwareNetworkInterface(TDEVINFOCollector* self,
					     DEVINFOCollector_OnGetCallback in_callback,
					     sse_pointer in_user_data)
{
  sse_int err;
  MoatObject *object = NULL;
  SSESList *if_list = NULL;
  SSESList *i = NULL;
  SSESList *info_list = NULL;

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;

  /* Get network interface information */
  err = SseUtilNetInfo_GetInterfaceList(&if_list);
  if (err != SSE_E_OK) {
    LOG_ERROR("SseUtilNetInfo_GetInterfaceList() ... failed with [%s].", err);
    goto error_exit;
  }
  LOG_DEBUG("Network I/F num = [%d]", sse_slist_length(if_list));

  for (i = if_list; i != NULL; i = sse_slist_next(i)) {
    SSEString *ifname;
    SSESList *tmp;

    ifname = sse_slist_data(i);
    ASSERT(ifname);
    tmp = DEVINFOCollector_AddHadwareNetworkInterface(info_list, ifname);
    if (tmp == NULL) {
      LOG_ERROR("DEVINFOCollector_AddHadwareNetworkInterface() ... failed.");
      goto error_exit;
    }
    info_list = tmp;
  }

  object = moat_object_new();
  ASSERT(object);
  
  /* NOTE: In case of using moat_object_add_list_value() with in_dup=FALSE, Valgrind detects
   *       a invalid memory write, so I use it with in_dup=TRUE as workaround.
   *       It might be a bug in MOAT C SDK.
   */
  err = moat_object_add_list_value(object, DEVINFO_KEY_NET_INTERFACE, info_list, sse_true, sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_list_value() ... failed with [%s].", err);
    goto error_exit;
  }
  
  /* Call callback */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_PUBLISHING;
  if (self->fOnGetCallback) {
    LOG_DEBUG("Call callback = [%p]", self->fOnGetCallback);
    self->fOnGetCallback(object, self->fUserData, SSE_E_OK);
  }

  /* Cleanup */
  moat_object_free(object);
  DEVINFOCollector_FreeListedSSEString(if_list);
  DEVINFOCollector_FreeListedMoatObject(info_list);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;

  return SSE_E_OK;

  /* Abnormal end */
 error_exit:
  self->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
  if (object) moat_object_free(object);
  DEVINFOCollector_FreeListedSSEString(if_list);
  DEVINFOCollector_FreeListedMoatObject(info_list);
  return err;
}

static sse_int
DEVINFOCollector_GetSoftwareOSType(SSEString **out_os_type)
{
  FILE *fd;
  sse_char buff[64];
  SSEString *os_type = NULL;

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

  LOG_DEBUG("OS Type = [%s]", buff);
  os_type = sse_string_new(buff);
  ASSERT(os_type);
  *out_os_type = os_type;
  return SSE_E_OK;
}

static sse_int
DEVINFOCollector_GetSoftwareOSVersion(SSEString **out_os_version)
{
  FILE *fd;
  sse_char buff[64];
  SSEString *os_version = NULL;

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

  LOG_DEBUG("OS Version = [%s]", buff);
  os_version = sse_string_new(buff);
  ASSERT(os_version);
  *out_os_version = os_version;
  return SSE_E_OK;
}

static void
DEVINFOCollector_GetHadwareNetworkNameserverOnComplateCallback(TSseUtilShellCommand* self,
							       sse_pointer in_user_data,
							       sse_int in_result)
{
  ASSERT(in_user_data);
  TDEVINFOCollector *collector = (TDEVINFOCollector *)in_user_data;
  LOG_INFO("collector=[%p], command=[%s] has been completed.", collector, self->fShellCommand);

  /* Cleanup */
  collector->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  TSseUtilShellCommand_Finalize(self);
}

static void
DEVINFOCollector_GetHadwareNetworkNameserverOnReadCallback(TSseUtilShellCommand* self,
							    sse_pointer in_user_data)
{
  sse_int err;
  sse_char *buff;
  MoatObject *object;

  ASSERT(in_user_data);
  TDEVINFOCollector *collector = (TDEVINFOCollector *)in_user_data;
  LOG_DEBUG("collector=[%p], command=[%s] is readable.", collector, self->fShellCommand);

  while ((err = TSseUtilShellCommand_ReadLine(self, &buff, sse_true)) == SSE_E_OK) {
    LOG_DEBUG("nameserver=[%s]", buff);
    if (sse_strncmp("nameserver", buff, sse_strlen("nameserver")) == 0) {
      sse_char *p;
      p = sse_strchr(buff, ' ');
      if ((p != NULL) && (*(++p) != '\0')) {
	object = moat_object_new();
	ASSERT(object);

	err = moat_object_add_string_value(object, DEVINFO_KEY_NET_NAMESERVER, p, sse_strlen(p), sse_true, sse_false);
	ASSERT(err == SSE_E_OK);

	if(collector->fOnGetCallback) {
	  collector->fOnGetCallback(object, self->fOnCompletedCallbackUserData, err);
	}
	moat_object_free(object);
      }
    }
    sse_free(buff);
  }
  if (err != SSE_E_NOENT && err != SSE_E_AGAIN) {
    LOG_ERROR("TSseUtilShellCommand_ReadLine() ... failed with [%s]", sse_get_error_string(err));
    if (self->fOnErrorCallback) {
      self->fOnErrorCallback(self, self->fOnErrorCallbackUserData, err, sse_get_error_string(err));
    }
    return;
  }

  return;
}

static void
TDEVINFOCollector_GetHadwareNetworkNameserverOnErrorCallback(TSseUtilShellCommand* self,
							      sse_pointer in_user_data,
							      sse_int in_error_code,
							      const sse_char* in_message)
{
  ASSERT(in_user_data);
  TDEVINFOCollector *collector = (TDEVINFOCollector *)in_user_data;

  LOG_ERROR("collector=[%p], command=[%s] failed with code=[%d], message=[%s]", collector, self->fShellCommand, in_error_code, in_message);
  collector->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
}

sse_int
TDEVINFOCollector_GetHadwareNetworkNameserver(TDEVINFOCollector* self,
					      DEVINFOCollector_OnGetCallback in_callback,
					      sse_pointer in_user_data)
{
  sse_int err;

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;

  /* Create command to read /etc/resolv.conf */
  err = TSseUtilShellCommand_Initialize(&self->fCommand);
  ASSERT(err == SSE_E_OK);
  err = TSseUtilShellCommand_SetShellCommand(&self->fCommand, "cat");
  ASSERT(err == SSE_E_OK);
  err = TSseUtilShellCommand_AddArgument(&self->fCommand, "/etc/resolv.conf");
  ASSERT(err == SSE_E_OK);

  /* Set callbacks */
  err = TSseUtilShellCommand_SetOnComplatedCallback(&self->fCommand, DEVINFOCollector_GetHadwareNetworkNameserverOnComplateCallback, self);
  ASSERT(err == SSE_E_OK);
  err = TSseUtilShellCommand_SetOnReadCallback(&self->fCommand, DEVINFOCollector_GetHadwareNetworkNameserverOnReadCallback, self);
  ASSERT(err == SSE_E_OK);
  err=  TSseUtilShellCommand_SetOnErrorCallback(&self->fCommand,TDEVINFOCollector_GetHadwareNetworkNameserverOnErrorCallback, self);
  ASSERT(err == SSE_E_OK);

  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;
  err = TSseUtilShellCommand_Execute(&self->fCommand);
  ASSERT(err == SSE_E_OK); //TODO fix me.

  return SSE_E_OK;
}

sse_int
TDEVINFOCollector_GetSoftwareOS(TDEVINFOCollector* self,
				DEVINFOCollector_OnGetCallback in_callback,
				sse_pointer in_user_data)
{
  sse_int err;
  MoatObject *object = NULL;
  SSEString *os_type = NULL;
  SSEString *os_version = NULL;

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;

  /* Get OS Type (e.g. Linux) */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;
  err = DEVINFOCollector_GetSoftwareOSType(&os_type);
  if (err != SSE_E_OK) {
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
      goto error_exit;
    }
  }

  /* Get OS Version (e.g. 3.2.0-4-amd64) */
  err = DEVINFOCollector_GetSoftwareOSVersion(&os_version);
  if (err != SSE_E_OK) {
    if(self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, err);
      goto error_exit;
    }
  }

  /* Create MoatObject */
  object = moat_object_new();
  ASSERT(object);
  err = moat_object_add_string_value(object,
				     DEVINFO_KEY_OS_TYPE,
				     sse_string_get_cstr(os_type),
				     sse_string_get_length(os_type),
				     sse_true,
				     sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }
  err = moat_object_add_string_value(object,
				     DEVINFO_KEY_OS_VERSION,
				     sse_string_get_cstr(os_version),
				     sse_string_get_length(os_version),
				     sse_true,
				     sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }

  /* Call callback */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_PUBLISHING;
  if(self->fOnGetCallback) {
    self->fOnGetCallback(object, self->fUserData, err);
  }

  /* Cleanup */
  sse_string_free(os_type, sse_true);
  sse_string_free(os_version, sse_true);
  moat_object_free(object);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;

  /* Abnormal end */
 error_exit:
  self->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
  if (os_type) sse_string_free(os_type, sse_true);
  if (os_version) sse_string_free(os_version, sse_true);
  if (object) moat_object_free(object);
  return err;
}


