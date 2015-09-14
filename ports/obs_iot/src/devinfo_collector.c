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

#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
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
#define LOG_DEBUG_SSESTRING(label, str) {\
  sse_char buff[32];\
  sse_uint len = (sizeof(buff) - 1 < sse_string_get_length(str) ? sizeof(buff) - 1 : sse_string_get_length(str)); \
  sse_strncpy(buff, sse_string_get_cstr(str), len);\
  buff[len] = '\0';\
  LOG_DEBUG(#label " = [%s]", buff);\
  }


static char
DEVINFOCollector_IntToChar(int in_val)
{
  if ((in_val >= 0) && (in_val <= 9)) {
    return '0' + in_val;
  } else if ((in_val >= 0xa) && (in_val <= 0xf)) {
    return 'a' + in_val - 0xa;
  }
  return '?';
}

sse_int
TDEVINFOCollector_GetHardwarePlatformVendor(TDEVINFOCollector* self,
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
TDEVINFOCollector_GetHardwarePlatformProduct(TDEVINFOCollector* self,
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
TDEVINFOCollector_GetHardwarePlatformModel(TDEVINFOCollector* self,
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
TDEVINFOCollector_GetHardwarePlatformSerial(TDEVINFOCollector* self,
                                            DEVINFOCollector_OnGetCallback in_callback,
                                            sse_pointer in_user_data)
{
  MoatValue *value = NULL;
  sse_int ret;
  sse_int i;
  sse_char cmd[32];
  sse_char serial[9];

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

  for (i = 0; i < 8; i++) {
    sprintf(cmd, "kosanu r 1 0xa0 0x%d", i+1);
    ret = system(cmd);
    if (WIFEXITED(ret)) {
      serial[i] = DEVINFOCollector_IntToChar(WEXITSTATUS(ret));
    } else if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
      LOG_ERROR("system() has been failed with SIGINT or SIGQUIT [%d].", ret);
      if (self->fOnGetCallback) {
        self->fOnGetCallback(NULL, self->fUserData, SSE_E_NOENT);
      }
      self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
      return SSE_E_OK;
    } else {
      LOG_ERROR("system() has been failed with [%d].", ret);
      if (self->fOnGetCallback) {
        self->fOnGetCallback(NULL, self->fUserData, SSE_E_NOENT);
      }
      self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
      return SSE_E_OK;
    }
  }
  serial[8] = '\0';

  /* Call callback */
  LOG_DEBUG("Serial number = [%s]", serial);
  if(self->fOnGetCallback) {
    value = moat_value_new_string(serial, 0, sse_true);
    ASSERT(value);
    self->fOnGetCallback(value, self->fUserData, SSE_E_OK);
    moat_value_free(value);
  }

  /* Cleanup */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;
}

sse_int
TDEVINFOCollector_GetHardwarePlatformHwVersion(TDEVINFOCollector* self,
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
TDEVINFOCollector_GetHardwarePlatformFwVersion(TDEVINFOCollector* self,
					       DEVINFOCollector_OnGetCallback in_callback,
					       sse_pointer in_user_data)
{
  MoatValue *value = NULL;
  FILE *fd;
  sse_char buff[32];

  LOG_DEBUG("Set callback = [%p] and user data= [%p]", in_callback, in_user_data);

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;

  if ((fd = popen("dpkg -l kernel-image-obsbx1 | tail -1 | awk \'{ print $3 }\' | tr -d \"\n\"", "r")) == NULL) {
    sse_int err_no = errno;
    LOG_ERROR("popen() has been failed with [%s].", strerror(err_no));
    if (self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, SSE_E_NOENT);
    }
    self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
    return SSE_E_OK;
  }

  if (fgets(buff, sizeof(buff), fd) == NULL) {
    sse_int err_no = errno;
    LOG_ERROR("popen() has been failed with [%s].", strerror(err_no));
    fclose(fd);
    if (self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, SSE_E_NOENT);
    }
    self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
    return SSE_E_OK;
  }
  fclose(fd);

  /* Call callback */
  LOG_DEBUG("FW version = [%s]", buff);
  if(self->fOnGetCallback) {
    value = moat_value_new_string(buff, 0, sse_true);
    ASSERT(value);
    self->fOnGetCallback(value, self->fUserData, SSE_E_OK);
    moat_value_free(value);
  }

  /* Cleanup */
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;
}

sse_int
TDEVINFOCollector_GetHardwareSim(TDEVINFOCollector* self,
				 DEVINFOCollector_OnGetCallback in_callback,
				 sse_pointer in_user_data)
{
  sse_int err;
  MoatObject *object = NULL;
  MoatValue *value = NULL;
  FILE *fd;
  sse_char buff[128];
  sse_char *iccid = NULL;
  sse_char *imsi = NULL;
  sse_char *msisdn = NULL;
  sse_int i;

  ASSERT(self);

  self->fOnGetCallback = in_callback;
  self->fUserData = in_user_data;
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COLLECTING;


  /* Get SIM Info */
  fd = fopen("/tmp/.flags/.sim_exist", "r");
  if (fd == NULL) {
    LOG_INFO("No SIM info.");
    if (self->fOnGetCallback) {
      self->fOnGetCallback(NULL, self->fUserData, SSE_E_NOENT);
    }
    self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
    return SSE_E_OK;
  }


  while (fgets(buff, sizeof(buff), fd) != NULL) {
    LOG_DEBUG("SIM: %s", buff);
    if ((imsi == NULL) && (sse_strlen(buff) > 15)) {
      for (i = 0; i < 15; i++) {
        if (!isdigit(buff[i])) {
          break;
        }
      }
      imsi = sse_strndup(buff, 15);
      ASSERT(imsi);
      LOG_INFO("IMSI=%[%s].", imsi);
    } else if ((iccid == NULL) && (sse_strlen(buff) > 6) && (sse_strncmp(buff, "+CCID:", 6) == 0)) {
      iccid = sse_strndup(buff + 7, 19);
      ASSERT(iccid);
      LOG_INFO("ICCID=[%s].", iccid);
    } else if ((msisdn == NULL) && (sse_strlen(buff) > 6) && (sse_strncmp(buff, "+CNUM:", 6) == 0)) {
      msisdn = sse_strndup(buff + 11, 11);
      ASSERT(msisdn);
      LOG_INFO("MSISDN=[%s].", msisdn);
    }
  }
  fclose(fd);

  /* Create MoatObject */
  object = moat_object_new();
  ASSERT(object);
  if (iccid) {
    err = moat_object_add_string_value(object, DEVINFO_KEY_SIM_ICCID, iccid, sse_strlen(iccid), sse_true, sse_false); 
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
      if(self->fOnGetCallback) {
        self->fOnGetCallback(NULL, self->fUserData, err);
      }
      goto error_exit;
    }
  }
  if (imsi) {
    err = moat_object_add_string_value(object, DEVINFO_KEY_SIM_IMSI, imsi, sse_strlen(imsi), sse_true, sse_false); 
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
      if(self->fOnGetCallback) {
        self->fOnGetCallback(NULL, self->fUserData, err);
      }
      goto error_exit;
    }
  }
  if (msisdn) {
    err = moat_object_add_string_value(object, DEVINFO_KEY_SIM_MSISDN, msisdn, sse_strlen(msisdn), sse_true, sse_false); 
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_object_set_string_value() ... failed with [%s].", sse_get_error_string(err));
      if(self->fOnGetCallback) {
        self->fOnGetCallback(NULL, self->fUserData, err);
      }
      goto error_exit;
    }
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
  if (imsi)   sse_free(imsi);
  if (iccid)  sse_free(iccid);
  if (msisdn) sse_free(msisdn);
  self->fStatus = DEVINFO_COLLECTOR_STATUS_COMPLETED;
  return SSE_E_OK;

  /* Abnormal end */
 error_exit:
  self->fStatus = DEVINFO_COLLECTOR_STATUS_ABEND;
  if (object) moat_object_free(object);
  if (imsi)   sse_free(imsi);
  if (iccid)  sse_free(iccid);
  if (msisdn) sse_free(msisdn);
  return err;
}
