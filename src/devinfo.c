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

/*!
 * @filedevinfo.c
 * @briefMOAT application in order to collect device information
 *
 *An application that collecting a device information  responding to  a request from ServiceSync server.
 */

#include <stdlib.h> // EXIT_SUCCESS
#include <servicesync/moat.h>
#include <sseutils.h>
#include <devinfo/devinfo.h>

#define  DEVINFO_MODEL_NAME "DeviceInfo"

#define TAG "Devinfo"
#define LOG_ERROR(format, ...) MOAT_LOG_ERROR(TAG, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  MOAT_LOG_WARN(TAG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  MOAT_LOG_INFO(TAG, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) MOAT_LOG_DEBUG(TAG, format, ##__VA_ARGS__)
#define LOG_TRACE(format, ...) MOAT_LOG_TRACE(TAG, format, ##__VA_ARGS__)
#include <stdlib.h>
#define ASSERT(cond) if(!(cond)) { LOG_ERROR("ASSERTION FAILED:" #cond); abort(); }

static void
DeviceInfo_CollectOnComplete(sse_int in_err,
			     sse_pointer in_user_data)
{
  sse_int err;
  TDEVINFOModelCommand *context;
  TDEVINFOManager *mgr;
  SSEString *devinfo = NULL;
  MoatObject *object = NULL;
  sse_char *job_service_id;
  sse_int request_id;
  sse_size encoded_len;
  sse_char *encoded;

  context = (TDEVINFOModelCommand *)in_user_data;
  ASSERT(context);
  mgr = context->fMgr;
  ASSERT(mgr);

  object = moat_object_new();
  ASSERT(object);

  job_service_id = moat_create_notification_id_with_moat(context->fMoat, "inquire-result", "1.0.0");
  ASSERT(job_service_id);
  LOG_DEBUG("URI=[%s]", job_service_id);

  /* Check the result of collecting devinfo. */
  if (in_err != SSE_E_OK) {
    LOG_ERROR("DeviceInfo_collect_async() ... failed with [%s].", sse_get_error_string(in_err));
    goto error_exit;
  }

  /* Get devinfo from the repository. */
  err = TDEVINFOManager_GetDevinfo(mgr, &devinfo);
  if (err != SSE_E_OK) {
    LOG_ERROR("TDEVINFOManager_GetDevinfo() ... failed with [%s].", sse_get_error_string(err));
    goto error_exit;
  }

  /* Base64 encode */
  encoded_len = sse_base64_get_encoded_length(sse_string_get_length(devinfo));
  encoded = sse_zeroalloc(encoded_len);
  ASSERT(encoded);
  sse_base64_encode((sse_byte*)sse_string_get_cstr(devinfo), sse_string_get_length(devinfo), encoded);
  LOG_INFO("Encoded=[%s]", encoded);

  /* Create a model object */
  err = moat_object_add_string_value(object, "deviceInfo", encoded, encoded_len, sse_true, sse_false);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_object_add_string_value() ... failed with [%s].", err);
    goto error_exit;
  }

  /* Send a notification. */
  request_id = moat_send_notification(context->fMoat,
				      job_service_id,
				      context->fKey,
				      DEVINFO_MODEL_NAME,
				      object,
				      NULL, //FIXME
				      NULL); //FIXME
  if (request_id < 0) {
    LOG_ERROR("moat_send_notification() ... failed with [%s].", request_id);
  }
  LOG_INFO("moat_send_notification(job_service_id=[%s], key=[%s]) ... in progress.", job_service_id, context->fKey);
  sse_string_free(devinfo, sse_true);
  moat_object_free(object);
  return;

 error_exit:
  /* No way to reply error after responding SSE_E_INPROGRESS, so reply empty object as error.
   */
  request_id = moat_send_notification(context->fMoat,
				      job_service_id,
				      context->fKey,
				      DEVINFO_MODEL_NAME,
				      object,
				      NULL, //FIXME
				      NULL); //FIXME
  if (request_id < 0) {
    LOG_ERROR("moat_send_notification() ... failed with [%s].", request_id);
  }
  if (devinfo) {
    sse_string_free(devinfo, sse_true);
  }
  if (object) {
    moat_object_free(object);
  }
  sse_free(context);
  return;
}

static sse_int
DeviceInfo_CollectAsync(Moat in_moat,
			sse_char *in_uid,
			sse_char *in_key,
			MoatValue *in_data,
			sse_pointer in_model_context)
{
  sse_int err;
  TDEVINFOManager *devinfo_mgr;
  TDEVINFOModelCommand *context;

  devinfo_mgr = (TDEVINFOManager *)in_model_context;
  ASSERT(devinfo_mgr);

  context = DEVINFOModelCommand_New(in_moat, devinfo_mgr, in_uid, in_key, "collect", in_data);
  ASSERT(context);

  err = TDEVINFOManager_Collect(devinfo_mgr, DeviceInfo_CollectOnComplete, context);
  if (err != SSE_E_OK) {
    LOG_ERROR("TDEVINFOManager_Collect() ... failed with [%s].", sse_get_error_string(err));
    TDEVINFOModelCommand_Delete(context);
    return err;
  }

  return SSE_E_OK;
}

/**
 * @breaf   Entry point for the "collect" command.
 *
 * Entry point for the "collect" command.
 *
 * @param [in] in_moat          MOAT instance
 * @param [in] in_uid           UID of model object
 * @param [in] in_key           Key for async exec
 * @param [in] in_data          Parameter of command
 * @param [in] in_model_context Model context
 *
 * @retval SSE_E_OK         Completed successfully (But it will be never returned.)
 * @retval SSE_E_INPROGRESS Now executing
 * @retval Others           Failure
 */
sse_int
DeviceInfo_collect(Moat in_moat,
		   sse_char *in_uid,
		   sse_char *in_key,
		   MoatValue *in_data,
		   sse_pointer in_model_context)
{
  sse_int err;
  TDEVINFOManager *devinfo_mgr;

  LOG_INFO("DeviceInfo:collect command (uid =[%s], key=[%s]) has been executed.", in_uid, in_key); 
  ASSERT(in_model_context);

  devinfo_mgr = (TDEVINFOManager *)in_model_context;
  err = moat_start_async_command(in_moat, in_uid, in_key, in_data, DeviceInfo_CollectAsync, devinfo_mgr);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_start_async_command() ... failed with [%s].", sse_get_error_string(err));
    return err;
  }
  return SSE_E_INPROGRESS;
}
  
/**
 * @breaf   Main routine
 *
 * Main routine of MOAT application
 *
 * @param [in] argc Number of arguments
 * @param [in] argv Arguments
 *
 * @retval EXIT_SUCCESS Success
 * @retval EXIT_FAILURE Failure
 */
sse_int
moat_app_main(sse_int argc, sse_char *argv[])
{
  sse_int err;
  Moat moat;
  ModelMapper mapper;
  TDEVINFOManager devinfo_mgr;
  
  LOG_DEBUG("h-e-l-l-(o) m-o-a-t...");
  err = moat_init(argv[0], &moat);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_init() has been failed with [%d]", err);
    goto err_exit;
  }

  err = TDEVINFOManager_Initialize(&devinfo_mgr, moat);
  if (err != SSE_E_OK) {
    LOG_ERROR("TDEVINFOManager_Initialize() has been failed with [%s].", sse_get_error_string(err));
    goto err_exit;
  }

  /* Register the model */
  mapper.AddProc = NULL;
  mapper.RemoveProc = NULL;
  mapper.UpdateProc = NULL;
  mapper.UpdateFieldsProc = NULL;
  mapper.FindAllUidsProc = NULL;
  mapper.FindByUidProc = NULL;
  mapper.CountProc = NULL;
  err = moat_register_model(moat, DEVINFO_MODEL_NAME, &mapper, &devinfo_mgr);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_register_model() has been failed with[%s].", sse_get_error_string(err));
    goto err_exit;
  }

  /* main loop */
  moat_run(moat);

  /* Teardown */
  TDEVINFOManager_Finalize(&devinfo_mgr);
  moat_unregister_model(moat, DEVINFO_MODEL_NAME);
  moat_destroy(moat);

  LOG_INFO("Teardown with SUCCESS");
  return EXIT_SUCCESS;

 err_exit:
  TDEVINFOManager_Finalize(&devinfo_mgr);
  moat_unregister_model(moat, DEVINFO_MODEL_NAME);
  moat_destroy(moat);
  LOG_ERROR("Teardown with FAILURE");
  return EXIT_FAILURE;
}
