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
#define LOG_DEBUG_SSESTRING(label, str) {				\
    sse_char buff[32];							\
    sse_uint len = (sizeof(buff) - 1 < sse_string_get_length(str) ? sizeof(buff) - 1 : sse_string_get_length(str)); \
    sse_strncpy(buff, sse_string_get_cstr(str), len);			\
    buff[len] = '\0';							\
    LOG_DEBUG(#label " = [%s]", buff);					\
  }

static void
DEVINFOCollector_GetHardwareSimOnComplateCallback(TSseUtilShellCommand* self,
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
DEVINFOCollector_GetHardwareSimOnReadCallback(TSseUtilShellCommand* self,
					      sse_pointer in_user_data)
{
  sse_int err;
  sse_char *buff;
  MoatValue *value;
  MoatObject *object;

  TDEVINFOCollector *collector = (TDEVINFOCollector *)in_user_data;
  ASSERT(collector);

  LOG_DEBUG("collector=[%p], command=[%s] is readable.", collector, self->fShellCommand);
  while ((err = TSseUtilShellCommand_ReadLine(self, &buff, sse_true)) == SSE_E_OK) {
    if (sse_strcmp(buff, "error") == 0) {
      LOG_ERROR("Getting phone number has been failed.");
      /* If "3g-phone-num" command is failed, it returns "error". In this case, this function passes
       * "error" to the callback functions as sccess.
       */
    } else {
      LOG_DEBUG("phone number=[%s]", buff);
    }
    object = moat_object_new();
    ASSERT(object);
    err = moat_object_add_string_value(object, DEVINFO_KEY_SIM_MSISDN, buff, sse_strlen(buff), sse_true, sse_false);
    ASSERT(err == SSE_E_OK);
    value = moat_value_new_object(object, sse_true);
    ASSERT(value);
    
    if(collector->fOnGetCallback) {
      collector->fOnGetCallback(value, collector->fUserData, err);
    }
    moat_value_free(value);
    moat_object_free(object);
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
TDEVINFOCollector_GetHardwareSimOnErrorCallback(TSseUtilShellCommand* self,
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
TDEVINFOCollector_GetHardwareSim(TDEVINFOCollector* self,
				 DEVINFOCollector_OnGetCallback in_callback,
				 sse_pointer in_user_data)
{
  sse_int err;

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

  /* Execute "3g-phone-num" command to get phone number (MSISDN). */
  err = TSseUtilShellCommand_Initialize(&self->fCommand);
  ASSERT(err == SSE_E_OK);
  err = TSseUtilShellCommand_SetShellCommand(&self->fCommand, "3g-phone-num");
  ASSERT(err == SSE_E_OK);

  /* Set callbacks */
  err = TSseUtilShellCommand_SetOnComplatedCallback(&self->fCommand, DEVINFOCollector_GetHardwareSimOnComplateCallback, self);
  ASSERT(err == SSE_E_OK);
  err = TSseUtilShellCommand_SetOnReadCallback(&self->fCommand, DEVINFOCollector_GetHardwareSimOnReadCallback, self);
  ASSERT(err == SSE_E_OK);
  err=  TSseUtilShellCommand_SetOnErrorCallback(&self->fCommand,TDEVINFOCollector_GetHardwareSimOnErrorCallback, self);
  ASSERT(err == SSE_E_OK);

  LOG_INFO("execute 3g-phone-num command on Armadillo-IoT to get phone number.");
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
