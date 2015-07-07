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

sse_int
TDEVINFOManager_Initialize(TDEVINFOManager *self, Moat in_moat)
{
  ASSERT(self);
  ASSERT(in_moat);

  LOG_DEBUG("self=[%p]", self);

  self->fMoat = in_moat;
  self->fState = DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED;
  TDEVINFOCollector_Initialize(&self->fCollector, in_moat);
  TDEVINFORepository_Initialize(&self->fRepository, in_moat);
  self->fStateMonitor = moat_timer_new();
  ASSERT(self->fStateMonitor);
  self->fStateMonitorTimerId = 0;
  self->fStateMonitorInterval = 0;
  return SSE_E_OK;
}

void
TDEVINFOManager_Finalize(TDEVINFOManager *self)
{
  ASSERT(self);
  self->fMoat = NULL;
  TDEVINFOCollector_Finalize(&self->fCollector);
  TDEVINFORepository_Finalize(&self->fRepository);
  self->fState = DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED;
  if (self->fStateMonitor) {
    moat_timer_free(self->fStateMonitor);
  }
  self->fStateMonitorTimerId = 0;
  self->fStateMonitorInterval = 0;
}

static void
DEVINFOManager_GetVendorCallback(MoatValue* in_vendor, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_vendor);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwarePlatformVendor(&self->fRepository, in_vendor);
  return;
}

static void
DEVINFOManager_GetProductCallback(MoatValue* in_product, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_product);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwarePlatformProduct(&self->fRepository, in_product);
  return;
}

static void
DEVINFOManager_GetModelCallback(MoatValue* in_model, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_model);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwarePlatformModel(&self->fRepository, in_model);
  return;
}

static void
DEVINFOManager_GetSerialCallback(MoatValue* in_serial, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_serial);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwarePlatformSerial(&self->fRepository, in_serial);
  return;
}

static void
DEVINFOManager_GetHwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_version);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwarePlatformHwVersion(&self->fRepository, in_version);
  return;
}

static void
DEVINFOManager_GetFwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_version);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwarePlatformFwVersion(&self->fRepository, in_version);
  return;
}

static void
DEVINFOManager_GetDeviceIdCallback(MoatValue* in_device_id, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_device_id);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwarePlatformDeviceId(&self->fRepository, in_device_id);
  return;
}

static void
DEVINFOManager_GetCategoryCallback(MoatValue* in_category, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_category);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwarePlatformCategory(&self->fRepository, in_category);
  return;
}

static void
DEVINFOManager_GetModemTypeCallback(MoatValue* in_type, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_type);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwareModemType(&self->fRepository, in_type);
  return;
}

static void
DEVINFOManager_GetModemHwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_version);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwareModemHwVersion(&self->fRepository, in_version);
  return;
}

static void
DEVINFOManager_GetModemFwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_version);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_SetHardwareModemFwVersion(&self->fRepository, in_version);
  return;
}

static void
DEVINFOManager_GetNetworkInterfaceCallback(MoatValue* in_if, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self;
  sse_int err;
  MoatObject *object;
  MoatValue *name;
  MoatValue *hw_addr;
  MoatValue *ipv4_addr;
  MoatValue *netmask;
  MoatValue *ipv6_addr;

  ASSERT(in_if);
  ASSERT(in_user_data);
  
  self = (TDEVINFOManager *)in_user_data;

  err = moat_value_get_object(in_if, &object);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_value_get_object() ... failed with [%s].", sse_get_error_string(err));
    return;
  }

  name = moat_object_get_value(object, DEVINFO_KEY_NET_INTERFACE_NAME);
  if (name == NULL) {
    LOG_ERROR("[%s] was not found in the object.", DEVINFO_KEY_NET_INTERFACE_NAME);
    return;
  }

  hw_addr = moat_object_get_value(object, DEVINFO_KEY_NET_INTERFACE_HW_ADDR);
  if (hw_addr == NULL) {
    LOG_ERROR("[%s] was not found in the object.", DEVINFO_KEY_NET_INTERFACE_HW_ADDR);
    return;
  }

  /* Following informatin is not mandatory. NULL is acceptable. */
  ipv4_addr = moat_object_get_value(object, DEVINFO_KEY_NET_INTERFACE_IPV4_ADDR);
  netmask = moat_object_get_value(object, DEVINFO_KEY_NET_INTERFACE_NETMASK);
  ipv6_addr = moat_object_get_value(object, DEVINFO_KEY_NET_INTERFACE_IPV6_ADDR);

  TDEVINFORepository_AddHardwareNetworkInterface(&self->fRepository, name, hw_addr, ipv4_addr, netmask, ipv6_addr);
  return;
}

static void
DEVINFOManager_GetNetworkNameserverCallback(MoatValue* in_nameserver, sse_pointer in_user_data, sse_int in_error_code)
{
  ASSERT(in_nameserver);
  ASSERT(in_user_data);
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  TDEVINFORepository_AddHardwareNetworkNameserver(&self->fRepository, in_nameserver);
  return;
}

static sse_int
TDEVINFOManager_GetDevinfo(TDEVINFOManager *self,
			   sse_int (*in_proc)(TDEVINFOCollector*, DEVINFOCollector_OnGetCallback, sse_pointer),
			   void (*in_callback)(MoatValue*, sse_pointer, sse_int))
{
  sse_int err;

  ASSERT(self);
  ASSERT(in_proc);
  ASSERT(in_callback);

  switch (TDEVINFOCollector_GetStatus(&self->fCollector)) {
  case DEVINFO_COLLECTOR_STATUS_INITIALIZED:
    err = in_proc(&self->fCollector, in_callback, self);
    if (err != SSE_E_OK) {
      LOG_ERROR("TDEVINFOCollector_GetXxx() ... failed with [%s].", sse_get_error_string(err));
    }
    break;
  case DEVINFO_COLLECTOR_STATUS_COLLECTING:
    LOG_DEBUG("Collecting ...");
    return SSE_E_INPROGRESS;
  case DEVINFO_COLLECTOR_STATUS_COMPLETED:
    LOG_DEBUG("Complete with success.");
    TDEVINFOCollector_Finalize(&self->fCollector);
    TDEVINFOCollector_Initialize(&self->fCollector, self->fMoat);
    self->fState++;
    break;
  case DEVINFO_COLLECTOR_STATUS_ABEND:
    LOG_DEBUG("Complete with failure. Ignore and go to next.");
    TDEVINFOCollector_Finalize(&self->fCollector);
    TDEVINFOCollector_Initialize(&self->fCollector, self->fMoat);
    self->fState++;
    break;
  default:
    LOG_WARN("Unexpected status = [%d].", TDEVINFOCollector_GetStatus(&self->fCollector));
    TDEVINFOCollector_Finalize(&self->fCollector);
    TDEVINFOCollector_Initialize(&self->fCollector, self->fMoat);
    self->fState++;
    break;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFOManager_Progress(TDEVINFOManager *self)
{
  sse_int err;
  SSEString *json_string;

  ASSERT(self);

  switch(self->fState) {
  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_VENDOR:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwarePlatformVendor, DEVINFOManager_GetVendorCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_PRODUCT:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwarePlatformProduct, DEVINFOManager_GetProductCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_MODEL:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwarePlatformModel, DEVINFOManager_GetModelCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_SERIAL:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwarePlatformSerial, DEVINFOManager_GetSerialCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_HW_VERSION:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwarePlatformHwVersion, DEVINFOManager_GetHwVersionCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_FW_VERSION:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwarePlatformFwVersion, DEVINFOManager_GetFwVersionCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_DEVICE_ID:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwarePlatformDeviceId, DEVINFOManager_GetDeviceIdCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_CATEGORY:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwarePlatformCategory, DEVINFOManager_GetCategoryCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_MODEM_TYPE:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwareModemType, DEVINFOManager_GetModemTypeCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_MODEM_HW_VERSION:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwareModemHwVersion, DEVINFOManager_GetModemHwVersionCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case   DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_MODEM_FW_VERSION:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwareModemFwVersion, DEVINFOManager_GetModemFwVersionCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_NETWORK_INTERFACE:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwareNetworkInterface, DEVINFOManager_GetNetworkInterfaceCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_NETWORK_NAMESERVER:
    err = TDEVINFOManager_GetDevinfo(self, TDEVINFOCollector_GetHardwareNetworkNameserver, DEVINFOManager_GetNetworkNameserverCallback);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
    break;

  case DEVINFO_MANAGER_STATE_COLLECTION_DONE:
    err = TDEVINFORepository_GetDevinfoWithJson(&self->fRepository, NULL, &json_string);
    if (err != SSE_E_OK) {
      LOG_ERROR("TDEVINFORepository_GetDevinfoWithJson() ... failed with [%d].", sse_get_error_string(err));
      return err;
    }
    {
      sse_char *devinfo = sse_strndup(sse_string_get_cstr(json_string), sse_string_get_length(json_string));
      ASSERT(devinfo);
      LOG_INFO("devinfo=[%s]", devinfo);
      sse_free(devinfo);
    }
    sse_free(json_string);
    return SSE_E_OK;

  default:
    LOG_WARN("Unexpected status = [%d].", &self->fCollector);
    return SSE_E_GENERIC;
  }

  err = TDEVINFOManager_Progress(self);
  return err;
}

static sse_bool
DEVINFOManager_Progress(sse_int in_timer_id, sse_pointer in_user_data)
{
  sse_int err;
  TDEVINFOManager *self;

  self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  err = TDEVINFOManager_Progress(self);
  LOG_DEBUG("TDEVINFOManager_Progress() returns [%s].", sse_get_error_string(err));
  if (err == SSE_E_INPROGRESS) {
    return sse_true;
  }
  return sse_false;
}

sse_int
TDEVINFOManager_Collect(TDEVINFOManager *self)
{
  sse_int timer_id;

  self->fState++;
  timer_id = moat_timer_set(self->fStateMonitor, self->fStateMonitorInterval, DEVINFOManager_Progress, self);
  if (timer_id < 0) {
    LOG_ERROR("moat_timer_set() ... failed with [%s].", timer_id);
    return SSE_E_GENERIC;
  }
  self->fStateMonitorTimerId = timer_id;
  return SSE_E_OK;
}
