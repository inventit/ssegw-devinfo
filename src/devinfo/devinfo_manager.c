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

static sse_int TDEVINFOManager_Execute(TDEVINFOManager *self);
static sse_int TDEVINFOManager_EnterNextState(TDEVINFOManager *self);
static sse_int TDEVINFOManager_LeaveState(TDEVINFOManager *self);
static sse_int TDEVINFOManager_ProgressSubState(TDEVINFOManager *self);
static sse_int TDEVINFOManager_Progress(TDEVINFOManager *self);
static sse_bool DEVINFOManager_Progress(sse_int in_timer_id, sse_pointer in_user_data);

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
static void DEVINFOManager_GetSimCallback(MoatValue* in_sim, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetOsCallback(MoatValue* in_sim, sse_pointer in_user_data, sse_int in_error_code);
static void DEVINFOManager_GetSsclCallback(MoatValue* in_sim, sse_pointer in_user_data, sse_int in_error_code);

const static TDEVINFOManagerGetDevinfoProc DEVINFO_MANAGER_GET_DEVINFO_PROC_TABLE[DEVINFO_MANAGER_STATEs] = {
  { NULL, NULL },
  { TDEVINFOCollector_GetHardwarePlatformVendor,    DEVINFOManager_GetVendorCallback },
  { TDEVINFOCollector_GetHardwarePlatformProduct,   DEVINFOManager_GetProductCallback },
  { TDEVINFOCollector_GetHardwarePlatformModel,     DEVINFOManager_GetModelCallback },
  { TDEVINFOCollector_GetHardwarePlatformSerial,    DEVINFOManager_GetSerialCallback },
  { TDEVINFOCollector_GetHardwarePlatformHwVersion, DEVINFOManager_GetHwVersionCallback },
  { TDEVINFOCollector_GetHardwarePlatformFwVersion, DEVINFOManager_GetFwVersionCallback },
  { TDEVINFOCollector_GetHardwarePlatformDeviceId,  DEVINFOManager_GetDeviceIdCallback },
  { TDEVINFOCollector_GetHardwarePlatformCategory,  DEVINFOManager_GetCategoryCallback },
  { TDEVINFOCollector_GetHardwareModemType,         DEVINFOManager_GetModemTypeCallback },
  { TDEVINFOCollector_GetHardwareModemHwVersion,    DEVINFOManager_GetModemHwVersionCallback },
  { TDEVINFOCollector_GetHardwareModemFwVersion,    DEVINFOManager_GetModemFwVersionCallback },
  { TDEVINFOCollector_GetHardwareNetworkInterface,  DEVINFOManager_GetNetworkInterfaceCallback },
  { TDEVINFOCollector_GetHardwareNetworkNameserver, DEVINFOManager_GetNetworkNameserverCallback },
  { TDEVINFOCollector_GetHardwareSim,               DEVINFOManager_GetSimCallback },
  { TDEVINFOCollector_GetSoftwareOS,                DEVINFOManager_GetOsCallback },
  { TDEVINFOCollector_GetSoftwareSscl,              DEVINFOManager_GetSsclCallback },
  { NULL, NULL }
};

sse_int
TDEVINFOManager_Initialize(TDEVINFOManager *self, Moat in_moat)
{
  ASSERT(self);
  ASSERT(in_moat);

  LOG_DEBUG("self=[%p]", self);

  self->fMoat = in_moat;
  self->fCollectCallback = NULL;
  self->fCollectCallbackUserData = NULL;
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
  self->fCollectCallback = NULL;
  self->fCollectCallbackUserData = NULL;
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
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED,                  TDEVINFOManager_GetState(self));
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
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_HARDWARE_SIM,                 TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_SOFTWARE_OS,                  TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTING_SOFTWARE_SSCL,                TDEVINFOManager_GetState(self));
  RETURN_STRING_IF_MATCH(DEVINFO_MANAGER_STATE_COLLECTION_DONE,                         TDEVINFOManager_GetState(self));
  return "Non-defined state";
}

sse_int
TDEVINFOManager_Collect(TDEVINFOManager *self,
			DEVINFOManager_CollectCallback in_callback,
			sse_pointer in_user_data)
{
  sse_int timer_id;

  ASSERT(self);

  if (self->fState != DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED) {
    LOG_WARN("Now collecting ... please try later.");
    return SSE_E_ALREADY;
  }

  self->fCollectCallback = in_callback;
  self->fCollectCallbackUserData = in_user_data;

  TDEVINFOManager_EnterNextState(self);
  timer_id = moat_timer_set(self->fStateMonitor, self->fStateMonitorInterval, DEVINFOManager_Progress, self);
  if (timer_id < 0) {
    LOG_ERROR("moat_timer_set() ... failed with [%s].", timer_id);
    return SSE_E_GENERIC;
  }
  self->fStateMonitorTimerId = timer_id;
  return SSE_E_OK;
}

sse_int
TDEVINFOManager_GetDevinfo(TDEVINFOManager *self,
			   SSEString **out_devinfo)
{
  sse_int err;
  SSEString *devinfo;

  ASSERT(self);
  ASSERT(out_devinfo);

  if (self->fState != DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED &&
      self->fState != DEVINFO_MANAGER_STATE_COLLECTION_DONE) {
    LOG_WARN("Now collecting devinf.");
    return SSE_E_AGAIN;
  }

  err = TDEVINFORepository_GetDevinfoWithJson(&self->fRepository, NULL, &devinfo);
  if (err != SSE_E_OK) {
    LOG_ERROR("TDEVINFORepository_GetDevinfoWithJson() ... failed with [%s].", sse_get_error_string(err));
    return err;
  }
  { /* for DEBUG print */
    sse_char *devinfo_cstr = sse_strndup(sse_string_get_cstr(devinfo), sse_string_get_length(devinfo));
    ASSERT(devinfo);
    LOG_INFO("devinfo=[%s]", devinfo_cstr);
    sse_free(devinfo_cstr);
  }
  *out_devinfo = devinfo;

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

  ASSERT(self);

  if (TDEVINFOManager_GetState(self) == DEVINFO_MANAGER_STATE_COLLECTION_DONE) {
    LOG_INFO("Devinfo collection done.");
    if (self->fCollectCallback) {
      self->fCollectCallback(SSE_E_OK, self->fCollectCallbackUserData);
    }
    self->fState = DEVINFO_MANAGER_STATE_COLLECTION_NOT_STARTED;
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
  MoatValue *value = NULL;
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    value = in_vendor;
  } else if (in_error_code == SSE_E_NOENT) {
    value = moat_value_new_string("NO DATA", 0, sse_true);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  if (value) {
    TDEVINFORepository_SetHardwarePlatformVendor(&self->fRepository, value);
  }

  if (value != in_vendor) {
    moat_value_free(value);
  }
}

static void
DEVINFOManager_GetProductCallback(MoatValue* in_product, sse_pointer in_user_data, sse_int in_error_code)
{
  MoatValue *value = NULL;
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    value = in_product;
  } else if (in_error_code == SSE_E_NOENT) {
    value = moat_value_new_string("NO DATA", 0, sse_true);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  if (value) {
    TDEVINFORepository_SetHardwarePlatformProduct(&self->fRepository, value);
  }

  if (value != in_product) {
    moat_value_free(value);
  }    

  return;
}

static void
DEVINFOManager_GetModelCallback(MoatValue* in_model, sse_pointer in_user_data, sse_int in_error_code)
{
  MoatValue *value = NULL;
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    value = in_model;
  } else if (in_error_code == SSE_E_NOENT) {
    value = moat_value_new_string("NO DATA", 0, sse_true);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  if (value) {
    TDEVINFORepository_SetHardwarePlatformModel(&self->fRepository, value);
  }

  if (value != in_model) {
    moat_value_free(value);
  }    

  return;
}

static void
DEVINFOManager_GetSerialCallback(MoatValue* in_serial, sse_pointer in_user_data, sse_int in_error_code)
{
  MoatValue *value = NULL;
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    value = in_serial;
  } else if (in_error_code == SSE_E_NOENT) {
    value = moat_value_new_string("NO DATA", 0, sse_true);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  if (value) {
    TDEVINFORepository_SetHardwarePlatformSerial(&self->fRepository, value);
  }

  if (value != in_serial) {
    moat_value_free(value);
  }    

  return;
}

static void
DEVINFOManager_GetHwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    TDEVINFORepository_SetHardwarePlatformHwVersion(&self->fRepository, in_version);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  return;
}

static void
DEVINFOManager_GetFwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code)
{
  MoatValue *value = NULL;
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    value = in_version;
  } else if (in_error_code == SSE_E_NOENT) {
    value = moat_value_new_string("NO DATA", 0, sse_true);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  if (value) {
    TDEVINFORepository_SetHardwarePlatformFwVersion(&self->fRepository, value);
  }

  if (value != in_version) {
    moat_value_free(value);
  }
  return;
}

static void
DEVINFOManager_GetDeviceIdCallback(MoatValue* in_device_id, sse_pointer in_user_data, sse_int in_error_code)
{
  MoatValue *value = NULL;
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    value = in_device_id;
  } else if (in_error_code == SSE_E_NOENT) {
    value = moat_value_new_string("NO DATA", 0, sse_true);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  if (value) {
    TDEVINFORepository_SetHardwarePlatformDeviceId(&self->fRepository, value);
  }

  if (value != in_device_id) {
    moat_value_free(value);
  }

  return;
}

static void
DEVINFOManager_GetCategoryCallback(MoatValue* in_category, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    TDEVINFORepository_SetHardwarePlatformCategory(&self->fRepository, in_category);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  return;
}

static void
DEVINFOManager_GetModemTypeCallback(MoatValue* in_type, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    TDEVINFORepository_SetHardwareModemType(&self->fRepository, in_type);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  return;
}

static void
DEVINFOManager_GetModemHwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    TDEVINFORepository_SetHardwareModemHwVersion(&self->fRepository, in_version);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  return;
}

static void
DEVINFOManager_GetModemFwVersionCallback(MoatValue* in_version, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    TDEVINFORepository_SetHardwareModemFwVersion(&self->fRepository, in_version);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

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

  self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
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
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  return;
}

static void
DEVINFOManager_GetNetworkNameserverCallback(MoatValue* in_nameserver, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    TDEVINFORepository_AddHardwareNetworkNameserver(&self->fRepository, in_nameserver);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  return;
}

static void
DEVINFOManager_GetSimCallback(MoatValue* in_sim, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  MoatObject *object;
  MoatValue *iccid;
  MoatValue *imsi;
  MoatValue *msisdn;
  sse_int err;

  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    err = moat_value_get_object(in_sim, &object);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_value_get_object() ... failed with [%s].", sse_get_error_string(err));
      return;
    }
    iccid = moat_object_get_value(object, DEVINFO_KEY_SIM_ICCID);
    imsi = moat_object_get_value(object, DEVINFO_KEY_SIM_IMSI);
    msisdn = moat_object_get_value(object, DEVINFO_KEY_SIM_MSISDN);
    TDEVINFORepository_AddHardwareSim(&self->fRepository, iccid, imsi, msisdn);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  return;

}

static void
DEVINFOManager_GetOsCallback(MoatValue* in_os, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  MoatObject *object;
  MoatValue *type;
  MoatValue *version;
  sse_int err;

  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    err = moat_value_get_object(in_os, &object);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_value_get_object() ... failed with [%s].", sse_get_error_string(err));
      return;
    }
    type = moat_object_get_value(object, DEVINFO_KEY_OS_TYPE);
    version = moat_object_get_value(object, DEVINFO_KEY_OS_VERSION);
    TDEVINFORepository_SetSoftwareOS(&self->fRepository, type, version);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  return;
}

static void
DEVINFOManager_GetSsclCallback(MoatValue* in_os, sse_pointer in_user_data, sse_int in_error_code)
{
  TDEVINFOManager *self = (TDEVINFOManager *)in_user_data;
  MoatObject *object;
  MoatValue *type;
  MoatValue *version;
  MoatValue *sdk_version;
  sse_int err;

  ASSERT(self);

  if (in_error_code == SSE_E_OK) {
    err = moat_value_get_object(in_os, &object);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_value_get_object() ... failed with [%s].", sse_get_error_string(err));
      return;
    }
    type = moat_object_get_value(object, DEVINFO_KEY_SSCL_TYPE);
    version = moat_object_get_value(object, DEVINFO_KEY_SSCL_VERSION);
    sdk_version = moat_object_get_value(object, DEVINFO_KEY_SSCL_SDK_VERSION);
    TDEVINFORepository_SetSoftwareSscl(&self->fRepository, type, version, sdk_version);
  } else {
    LOG_ERROR("Getting devinfo has been failed with [%s].", sse_get_error_string(in_error_code));
  }

  return;
}
