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

static void DEVINFOManager_GetVendorCallback(MoatValue* in_vendor, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetVendorCallback(MoatValue* in_vendor, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetProductCallback(MoatValue* in_product, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetModelCallback(MoatValue* in_model, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetSerialCallback(MoatValue* in_serial, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetHwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetFwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetDeviceIdCallback(MoatValue* in_device_id, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetCategoryCallback(MoatValue* in_category, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetModemTypeCallback(MoatValue* in_type, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetModemHwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetModemFwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetNetworkInterfaceCallback(MoatValue* in_if, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetNetworkNameserverCallback(MoatValue* in_nameserver, sse_pointer in_user_data, sse_int in_error_code);

static sse_int TDEVINFOManager_Execute(TDEVINFOManager *self);
static sse_int TDEVINFOManager_EnterNextState(TDEVINFOManager *self);
static sse_int TDEVINFOManager_LeaveState(TDEVINFOManager *self);
static sse_int TDEVINFOManager_ProgressSubState(TDEVINFOManager *self);
static sse_int TDEVINFOManager_Progress(TDEVINFOManager *self);
static sse_bool DEVINFOManager_Progress(sse_int in_timer_id, sse_pointer in_user_data);


const static TDEVINFOManagerGetDevinfoProc DEVINFO_MANAGER_GET_DEVINFO_PROC_TABLE[DEVINFO_MANAGER_STATEs] = {
  { NULL, NULL },
  { TDEVINFOCollector_GetHardwarePlatformVendor,    DEVINFOManager_GetVendorCallback },
  { TDEVINFOCollector_GetHardwarePlatformProduct,   DEVINFOManager_GetProductCallback },
  { TDEVINFOCollector_GetHardwarePlatformModel,     DEVINFOManager_GetModelCallback },
  { TDEVINFOCollector_GetHardwarePlatformSerial,    DEVINFOManager_GetSerialCallback },
  { TDEVINFOCollector_GetHardwarePlatformHwVersion, DEVINFOManager_GetHwVersionCallback },
  { TDEVINFOCollector_GetHardwarePlatformFwVersion, DEVINFOManager_GetFwVersionCallback },
  { TDEVINFOCollector_GetHardwarePlatformDeviceId,  DEVINFOManager_GetDeviceIdCallback },
  { TDEVINFOCollector_GetHardwarePlatformCategory, DEVINFOManager_GetCategoryCallback },
  { TDEVINFOCollector_GetHardwareModemType,         DEVINFOManager_GetModemTypeCallback },
  { TDEVINFOCollector_GetHardwareModemHwVersion,    DEVINFOManager_GetModemHwVersionCallback },
  { TDEVINFOCollector_GetHardwareModemFwVersion,    DEVINFOManager_GetModemFwVersionCallback },
  { TDEVINFOCollector_GetHardwareNetworkInterface,  DEVINFOManager_GetNetworkInterfaceCallback },
  { TDEVINFOCollector_GetHardwareNetworkNameserver, DEVINFOManager_GetNetworkNameserverCallback },
  { NULL, NULL }
};

sse_int
TDEVINFOManager_Initialize(TDEVINFOManager *self, Moat in_moat)
{
  ASSERT(self);
  ASSERT(in_moat);

  LOG_DEBUG("self=[%p]", self);

  self->fMoat = in_moat;
  self->fState = DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED;
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
  TDEVINFORepository_Finalize(&self->fRepository);
  self->fState = DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED;
  if (self->fStateMonitor) {
    moat_timer_free(self->fStateMonitor);
  }
  self->fStateMonitorTimerId = 0;
  self->fStateMonitorInterval = 0;
}

DEVINFOManagerState
TDEVINFOManager_GetState(TDEVINFOManager *self)
{
  ASSERT(self);
  return self->fState;
}

#define RETURN_STRING_IF_MATCH(def, code) { if (def == code) return #def; }
const sse_char*
TDEVINFOManager_GetStateWithCstr(TDEVINFOManager *self)
{
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED, TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_VENDOR,     TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_PRODUCT,    TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_MODEL,      TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_SERIAL,     TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_HW_VERSION, TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_FW_VERSION, TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_DEVICE_ID,  TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_PLATFORM_CATEGORY,   TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_MODEM_TYPE,          TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_MODEM_HW_VERSION,    TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_MODEM_FW_VERSION,    TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_NETWORK_INTERFACE,   TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_NETWORK_NAMESERVER,  TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTION_DONE, TDEVINFOManager_GetState(self));
  return "Non-defined state";
}

sse_int
TDEVINFOManager_Collect(TDEVINFOManager *self)
{
  sse_int timer_id;

  TDEVINFOManager_EnterNextState(self);
  timer_id = moat_timer_set(self->fStateMonitor, self->fStateMonitorInterval, DEVINFOManager_Progress, self);
  if (timer_id < 0) {
    LOG_ERROR("moat_timer_set() ... failed with [%s].", timer_id);
    return SSE_E_GENERIC;
  }
  self->fStateMonitorTimerId = timer_id;
  return SSE_E_OK;
}

static sse_int
TDEVINFOManager_Execute(TDEVINFOManager *self)
{
  sse_int err = SSE_E_GENERIC;
  TDEVINFOCollector_GetDevinfoProc proc;
  DEVINFOCollector_OnGetCallback callback;

  ASSERT(self);
  //ASSERT(self->fState >= DEVINFO_MANAGER_STATEs);
  if (self->fState < DEVINFO_MANAGER_STATEs) {
    proc = DEVINFO_MANAGER_GET_DEVINFO_PROC_TABLE[self->fState].fGetDevinfoProc;
    callback = DEVINFO_MANAGER_GET_DEVINFO_PROC_TABLE[self->fState].fGetDevinfoCallback;
    ASSERT(proc);
    err = proc(&self->fCollector, callback, self);
  }
  return err;
}

static sse_int
TDEVINFOManager_EnterNextState(TDEVINFOManager *self)
{
  sse_int err;

  ASSERT(self);
  self->fState++;
  LOG_DEBUG("Entering [%s (%d)].", TDEVINFOManager_GetStateWithCstr(self), TDEVINFOManager_GetState(self));
  err = TDEVINFOCollector_Initialize(&self->fCollector, self->fMoat);
  if (err != SSE_E_OK) {
    LOG_ERROR("TDEVINFOCollector_Initialize() ... failed with [%s].", sse_get_error_string(err));
    return err;
  }
  return SSE_E_OK;
}

static sse_int
TDEVINFOManager_LeaveState(TDEVINFOManager *self)
{
  ASSERT(self);
  TDEVINFOCollector_Finalize(&self->fCollector);
  return SSE_E_OK;

}

/* Wapper function to be called from timer.
 */
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

static sse_int
TDEVINFOManager_Progress(TDEVINFOManager *self)
{
  sse_int err;
  SSEString *json_string;

  ASSERT(self);
  if (TDEVINFOManager_GetState(self) == DEVINFO_MANAGER_STATE_COLLECTION_DONE) {
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
    sse_string_free(json_string, sse_true);
    return SSE_E_OK;
  } else if (TDEVINFOManager_GetState(self) == DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED) {
    LOG_WARN("Unexpected status = [%d].", &self->fCollector);
    return SSE_E_GENERIC;
  } else {
    err = TDEVINFOManager_ProgressSubState(self);
    if (err == SSE_E_INPROGRESS) {
      return err;
    }
  }
  err = TDEVINFOManager_Progress(self);
  return err;
}

static sse_int
TDEVINFOManager_ProgressSubState(TDEVINFOManager *self)
{
  sse_int err;

  ASSERT(self);

  switch (TDEVINFOCollector_GetStatus(&self->fCollector)) {
  case DEVINFO_COLLECTOR_STATUS_INITIALIZED:
    err = TDEVINFOManager_Execute(self);
    if (err != SSE_E_OK) {
      LOG_ERROR("TDEVINFOCollector_GetXxx() ... failed with [%s].", sse_get_error_string(err));
    }
    break;
  case DEVINFO_COLLECTOR_STATUS_COLLECTING:
    LOG_DEBUG("Collecting ...");
    return SSE_E_INPROGRESS;
  case DEVINFO_COLLECTOR_STATUS_COMPLETED:
    LOG_DEBUG("Complete with success.");
    TDEVINFOManager_LeaveState(self);
    TDEVINFOManager_EnterNextState(self);
    break;
  case DEVINFO_COLLECTOR_STATUS_ABEND:
    LOG_DEBUG("Complete with failure. Ignore and go to next.");
    TDEVINFOManager_LeaveState(self);
    TDEVINFOManager_EnterNextState(self);
    break;
  default:
    LOG_WARN("Unexpected status = [%d].", TDEVINFOCollector_GetStatus(&self->fCollector));
    TDEVINFOManager_LeaveState(self);
    TDEVINFOManager_EnterNextState(self);
    break;
  }
  return SSE_E_OK;
}

/*
 * Callback functions for getting device information.
 */

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
