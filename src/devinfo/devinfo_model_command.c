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

TDEVINFOModelCommand*
DEVINFOModelCommand_New(Moat in_moat,
			TDEVINFOManager *in_mgr,
			sse_char *in_uid,
			sse_char *in_key,
			sse_char *in_command,
			MoatValue *in_param)
{
  TDEVINFOModelCommand *self;

  self = sse_zeroalloc(sizeof(TDEVINFOModelCommand));
  ASSERT(self);

  self->fMoat = in_moat;
  self->fMgr = in_mgr;
  if (in_uid) {
    self->fUid = sse_strdup(in_uid);
    ASSERT(self->fUid);
  } else {
    self->fUid = NULL;
  }
  if (in_key) {
    self->fKey = sse_strdup(in_key);
    ASSERT(self->fKey);
  } else {
    self->fKey = NULL;
  }
  if (in_command) {
    self->fCommand = sse_strdup(in_command);
    ASSERT(self->fCommand);
  } else {
    self->fCommand = NULL;
  }
  if (in_param) {
    self->fParam = moat_value_clone(in_param);
    ASSERT(self->fParam);
  } else {
    self->fParam = NULL;
  }
  return self;
}

void
TDEVINFOModelCommand_Delete(TDEVINFOModelCommand *self)
{
  if (self) {
    self->fMoat = NULL;
    self->fMgr = NULL;
    if (self->fUid != NULL) sse_free(self->fUid);
    if (self->fKey != NULL) sse_free(self->fKey);
    if (self->fCommand != NULL) sse_free(self->fCommand);
    if (self->fParam != NULL) moat_value_free(self->fParam);
    sse_free(self);
  }
}
