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
#include <devinfo/devinfo.h>
#include <sseutils.h>

#define TAG "Devinfo:"
#define LOG_ERROR(format, ...) MOAT_LOG_ERROR(TAG, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  MOAT_LOG_WARN(TAG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  MOAT_LOG_INFO(TAG, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) MOAT_LOG_DEBUG(TAG, format, ##__VA_ARGS__)
#define LOG_TRACE(format, ...) MOAT_LOG_TRACE(TAG, format, ##__VA_ARGS__)
#include <stdlib.h>
#define ASSERT(cond) if(!(cond)) { LOG_ERROR("ASSERTION FAILED:" #cond); abort(); }

sse_int
TDEVINFORepository_Initialize(TDEVINFORepository* self, Moat in_moat)
{
  ASSERT(self);
  ASSERT(in_moat);

  self->fMoat = in_moat;
  self->fDevinfo = moat_object_new();
  ASSERT(self->fDevinfo);

  return SSE_E_OK;
}

void
TDEVINFORepository_Finalize(TDEVINFORepository* self)
{
  ASSERT(self);
  self->fMoat = NULL;
  if (self->fDevinfo) {
    moat_object_free(self->fDevinfo);
    self->fDevinfo = NULL;
  }
}

/*@TODO Over write only existing keys*/
sse_int
TDEVINFORepository_LoadDevinfo(TDEVINFORepository* self, SSEString *in_path)
{
  sse_char *path = NULL;
  MoatObject *devinfo = NULL;
  sse_int err = SSE_E_GENERIC;
  sse_char *err_msg = NULL;

  ASSERT(self);
  ASSERT(in_path);

  path = sse_strndup(sse_string_get_cstr(in_path),
		     sse_string_get_length(in_path));
  ASSERT(path);
  LOG_DEBUG("Load devinfo from file=[%s].", path);

  err = moat_json_file_to_moat_object(path, &devinfo, &err_msg);
  if (err != SSE_E_OK) {
    LOG_ERROR("moat_json_file_to_moat_object() ... failed with [%s]", err_msg);
    goto error_exit;
  }

  if (self->fDevinfo) {
    moat_object_free(self->fDevinfo);
  }
  self->fDevinfo = devinfo;
  sse_free(path);
  return SSE_E_OK;
  
 error_exit:
  if (path) sse_free(path);
  if (devinfo) moat_object_free(devinfo);
  if (err_msg) sse_free(err_msg);
  return err;
}

sse_int
TDEVINFORepository_GetDevinfoWithJson(TDEVINFORepository* self,
				      SSEString *in_key,
				      SSEString **out_devinfo)
{
  sse_int err;
  sse_char *json_string;
  sse_uint json_len;
  SSEString *devinfo;

  ASSERT(self);
  ASSERT(out_devinfo);

  if (in_key) {
    LOG_WARN("Todo: not implemented yet.");
    return SSE_E_GENERIC;
  } else {
    err = moat_object_to_json_string(self->fDevinfo, NULL, &json_len);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_object_to_json_string() ... failed with [%d]", err);
      return err;
    }

    json_string = sse_malloc(json_len);
    ASSERT(json_string);
   
    err = moat_object_to_json_string(self->fDevinfo, json_string, &json_len);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_object_to_json_string() ... failed with [%d]", err);
      return err;
    }

    devinfo = sse_string_new_with_length(json_string, json_len);
    ASSERT(devinfo);
    sse_free(json_string);
  }

  *out_devinfo = devinfo;
  return SSE_E_OK;
}

static sse_int
DEVINFORepository_GetDevinfo(MoatObject* in_object,
			     SSESList* in_key,
			     MoatValue** out_value)
{
  sse_int err;
  sse_char* key;
  MoatValue* value;
  MoatObject* tmp_object;

  ASSERT(in_key);
  ASSERT(out_value);

  key = sse_strndup(sse_string_get_cstr((SSEString*)sse_slist_data(in_key)),
		    sse_string_get_length((SSEString*)sse_slist_data(in_key)));
  ASSERT(key);

  value = moat_object_get_value(in_object, key);
  if (value == NULL) {
    LOG_DEBUG("key = [%s] was not found.", key);
    sse_free(key);
    return SSE_E_NOENT;
  }

  if (sse_slist_length(in_key) == 1) {
    *out_value = moat_value_clone(value);
    ASSERT(*out_value);
    sse_free(key);
    return SSE_E_OK;
  } else {
    if (moat_value_get_type(value) != MOAT_VALUE_TYPE_OBJECT) {
      LOG_ERROR("key = [%s] was not found.", key);
      sse_free(key);
      return SSE_E_NOENT;
    }
    err = moat_value_get_object(value, &tmp_object);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_value_get_object() ... failed with [%d].", err);
      return err;
    }
    err = DEVINFORepository_GetDevinfo(tmp_object, sse_slist_next(in_key), out_value);
    sse_free(key);
    return err;
  }
}

sse_int
TDEVINFORepository_GetDevinfo(TDEVINFORepository* self,
			      SSEString* in_key,
			      MoatValue** out_value)
{
  sse_int err;
  SSESList* key;
  SSESList* elem;

  ASSERT(in_key);
  ASSERT(out_value);

  ASSERT((key = sse_string_split(in_key, ".", 0)));
  err = DEVINFORepository_GetDevinfo(self->fDevinfo, key, out_value);

  while (key) {
    elem = key;
    key = sse_slist_unlink(key, elem);
    sse_string_free(sse_slist_data(elem), sse_true);
    sse_slist_free(elem);
  }

  return err;
}

static sse_int
DEVINFORepository_SetDevinfo(MoatObject* in_object,
			     SSESList* in_key,
			     MoatValue* in_value)
{
  sse_int err;
  sse_char* key;
  MoatObject* new_object;
  MoatObject* tmp_object;
  MoatValue* value;

  ASSERT(in_object);
  ASSERT(in_key);
  ASSERT(in_value);

  if (sse_slist_length(in_key) == 1) {
    key = sse_strndup(sse_string_get_cstr((SSEString *)sse_slist_data(in_key)),
		      sse_string_get_length((SSEString *)sse_slist_data(in_key)));
    ASSERT(key);
    if ((err = moat_object_add_value(in_object, key, in_value, sse_true, sse_true)) != SSE_E_OK) {
      LOG_ERROR("moat_object_add_value() ... failed with [%d]", err);
    }
    sse_free(key);
    return err;
  } else {
    key = sse_strndup(sse_string_get_cstr((SSEString *)sse_slist_data(in_key)),
		      sse_string_get_length((SSEString *)sse_slist_data(in_key)));
    ASSERT(key);

    value = moat_object_get_value(in_object, key);
    if (value == NULL) {
      new_object = moat_object_new();
      ASSERT(new_object);
      err = moat_object_add_object_value(in_object, key, new_object, sse_false, sse_true);
      if (err != SSE_E_OK) {
	LOG_ERROR("moat_object_add_object_value() ... failed with [%d].", err);
	sse_free(key);
	moat_object_free(new_object);
	return err;
      }
      value = moat_object_get_value(in_object, key);
      ASSERT(value);
    }
    sse_free(key);
    err = moat_value_get_object(value, &tmp_object);
    if (err != SSE_E_OK) {
      LOG_ERROR("moat_value_get_object() ... failed with [%d].", err);
      return err;
    }
    return DEVINFORepository_SetDevinfo(tmp_object, sse_slist_next(in_key), in_value);
  }
  ASSERT(!"Never rearch here!");
}

sse_int
TDEVINFORepository_SetDevinfo(TDEVINFORepository* self,
			      SSEString* in_key,
			      MoatValue* in_value)
{
  sse_int err;
  SSESList* key;
  SSESList* elem;

  ASSERT(in_key);
  ASSERT(in_value);

  key = sse_string_split((SSEString*)in_key, ".", 0);
  ASSERT(key);

  err = DEVINFORepository_SetDevinfo(self->fDevinfo, key, in_value);

  while (key) {
    elem = key;
    key = sse_slist_unlink(key, elem);
    sse_string_free(sse_slist_data(elem), sse_true);
    sse_slist_free(elem);
  }

  return err;
}

sse_int
TDEVINFORepository_SetHardwarePlatformVendor(TDEVINFORepository* self,
					     SSEString* in_vendor)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_vendor);

  key = sse_string_new("hardware.platform.vendor");
  value = moat_value_new_string(sse_string_get_cstr(in_vendor),
				sse_string_get_length(in_vendor),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_SetHardwarePlatformProduct(TDEVINFORepository* self,
					      SSEString* in_product)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_product);

  key = sse_string_new("hardware.platform.product");
  value = moat_value_new_string(sse_string_get_cstr(in_product),
				sse_string_get_length(in_product),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);

  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_SetHardwarePlatformModel(TDEVINFORepository* self,
					    SSEString* in_model)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_model);

  key = sse_string_new("hardware.platform.model");
  value = moat_value_new_string(sse_string_get_cstr(in_model),
				sse_string_get_length(in_model),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_SetHardwarePlatformSerial(TDEVINFORepository* self,
					     SSEString* in_serial)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_serial);

  key = sse_string_new("hardware.platform.vendor");
  value = moat_value_new_string(sse_string_get_cstr(in_serial),
				sse_string_get_length(in_serial),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_SetHardwarePlatformHwVersion(TDEVINFORepository* self,
						SSEString* in_hw_version)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_hw_version);

  key = sse_string_new("hardware.platform.hwVersion");
  value = moat_value_new_string(sse_string_get_cstr(in_hw_version),
				sse_string_get_length(in_hw_version),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_SetHardwarePlatformFwVersion(TDEVINFORepository* self,
						SSEString* in_fw_version)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_fw_version);

  key = sse_string_new("hardware.platform.fwVersion");
  value = moat_value_new_string(sse_string_get_cstr(in_fw_version),
				sse_string_get_length(in_fw_version),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_SetHardwarePlatformDeviceId(TDEVINFORepository* self,
					       SSEString* in_device_id)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_device_id);

  key = sse_string_new("hardware.platform.deviceId");
  value = moat_value_new_string(sse_string_get_cstr(in_device_id),
				sse_string_get_length(in_device_id),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_SetHardwarePlatformCategory(TDEVINFORepository* self,
					       SSEString* in_category)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_category);

  key = sse_string_new("hardware.platform.category");
  value = moat_value_new_string(sse_string_get_cstr(in_category),
				sse_string_get_length(in_category),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_SetHardwareModemType(TDEVINFORepository* self,
					SSEString* in_type)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_type);

  key = sse_string_new("hardware.modem.type");
  value = moat_value_new_string(sse_string_get_cstr(in_type),
				sse_string_get_length(in_type),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}


sse_int
TDEVINFORepository_SetHardwareModemHwVersion(TDEVINFORepository* self,
					     SSEString* in_hw_version)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_hw_version);

  key = sse_string_new("hardware.modem.hwVersion");
  value = moat_value_new_string(sse_string_get_cstr(in_hw_version),
				sse_string_get_length(in_hw_version),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_SetHardwareModemFwVersion(TDEVINFORepository* self,
					     SSEString* in_fw_version)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_fw_version);

  key = sse_string_new("hardware.modem.fwVersion");
  value = moat_value_new_string(sse_string_get_cstr(in_fw_version),
				sse_string_get_length(in_fw_version),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

sse_int
TDEVINFORepository_AddHadwareNetworkInterface(TDEVINFORepository* self,
					      SSEString* in_name,
					      SSEString* in_hw_address,
					      SSEString* in_ipv4_address,
					      SSEString* in_netmask,
					      SSEString* in_ipv6_address)
{
  sse_int err;
  SSEString* key;
  MoatValue* value = NULL;
  SSESList* ifs = NULL;
  MoatObject* new_if;
  MoatValue* new_if_value;

  ASSERT(self);
  ASSERT(in_name);
  ASSERT(in_hw_address);
  ASSERT(in_ipv4_address);
  ASSERT(in_netmask);
  ASSERT(in_ipv6_address);

  key = sse_string_new("hardware.network.interface");
  ASSERT(key);

  err = TDEVINFORepository_GetDevinfo(self, key, &value);
  if (err == SSE_E_OK) {
    LOG_DEBUG("Add new if into the existing list.");
    if (moat_value_get_type(value) != MOAT_VALUE_TYPE_LIST) {
      LOG_ERROR("MoatValue type of interface is not LSIT type.");
      sse_string_free(key, sse_true);
      return SSE_E_GENERIC;
    }
    err = moat_value_get_list(value, &ifs);
    if (err != SSE_E_OK) {
      LOG_ERROR("");
      sse_string_free(key, sse_true);
      return err;
    }
  } else if (err == SSE_E_NOENT) {
    LOG_DEBUG("Add new if into the empty list.");
    value = NULL;
    ifs = NULL;
  } else {
    LOG_ERROR("TDEVINFORepository_GetDevinfo() ... failed with [%d].", err);
    sse_string_free(key, sse_true);
    return err;
  }

  ASSERT((new_if = moat_object_new()));
  ASSERT(moat_object_add_string_value(new_if,
				      "name",
				      sse_string_get_cstr(in_name),
				      sse_string_get_length(in_name),
				      sse_true,
				      sse_false) == SSE_E_OK);
  ASSERT(moat_object_add_string_value(new_if,
				      "hwAddress",
				      sse_string_get_cstr(in_hw_address),
				      sse_string_get_length(in_hw_address),
				      sse_true,
				      sse_false) == SSE_E_OK);
  ASSERT(moat_object_add_string_value(new_if,
				      "ipv4Address",
				      sse_string_get_cstr(in_ipv4_address),
				      sse_string_get_length(in_ipv4_address),
				      sse_true,
				      sse_false) == SSE_E_OK);
  ASSERT(moat_object_add_string_value(new_if,
				      "netmask",
				      sse_string_get_cstr(in_netmask),
				      sse_string_get_length(in_netmask),
				      sse_true,
				      sse_false) == SSE_E_OK);
  ASSERT(moat_object_add_string_value(new_if,
				      "ipv6Address",
				      sse_string_get_cstr(in_ipv6_address),
				      sse_string_get_length(in_ipv6_address),
				      sse_true,
				      sse_false) == SSE_E_OK);

  ASSERT((new_if_value = moat_value_new_object(new_if, sse_false)));
  ASSERT((ifs = sse_slist_add(ifs, new_if_value)));

  if (value == NULL) {
    value = moat_value_new_list(ifs, sse_false);
    ASSERT(value);
  }
  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  return err;
}

sse_int
TDEVINFORepository_RemoveHardwareNetworkInterface(TDEVINFORepository* self,
						  SSEString* in_name)
{
  //@TODO Not implemented yet.
  return SSE_E_GENERIC;
}

sse_int
TDEVINFORepository_AddHardwareNetworkNameserver(TDEVINFORepository* self,
						SSEString* in_nameserver)
{
  sse_int err;
  SSEString* key;
  MoatValue* value = NULL;
  SSESList* nameserver_list = NULL;
  MoatValue* new_nameserver_value;

  ASSERT(self);
  ASSERT(in_nameserver);

  key= sse_string_new("hardware.network.nameserver");
  ASSERT(key);

  err = TDEVINFORepository_GetDevinfo(self, key, &value);
  if (err == SSE_E_OK) {
    LOG_DEBUG("Add new if into the existing list.");
    if (moat_value_get_type(value) != MOAT_VALUE_TYPE_LIST) {
      LOG_ERROR("MoatValue type of interface is not LSIT type.");
      sse_string_free(key, sse_true);
      return SSE_E_GENERIC;
    }
    err = moat_value_get_list(value, &nameserver_list);
    if (err != SSE_E_OK) {
      LOG_ERROR("");
      sse_string_free(key, sse_true);
      return err;
    }
  } else if (err == SSE_E_NOENT) {
    LOG_DEBUG("Add new if into the empty list.");
    value = NULL;
    nameserver_list = NULL;
  } else {
    LOG_ERROR("TDEVINFORepository_GetDevinfo() ... failed with [%d].", err);
    sse_string_free(key, sse_true);
    return err;
  }

  ASSERT((new_nameserver_value = moat_value_new_string(sse_string_get_cstr(in_nameserver),
						       sse_string_get_length(in_nameserver),
						       sse_true)));
  ASSERT((nameserver_list = sse_slist_add(nameserver_list, new_nameserver_value)));

  if (value == NULL) {
    value = moat_value_new_list(nameserver_list, sse_false);
    ASSERT(value);
  }
  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  return err;
}


sse_int
TDEVINFORepository_RemoveHardwareNetworkNameserver(TDEVINFORepository* self,
						   SSEString* in_nameserver)
{
  //@TODO Not implemented yet.
  return SSE_E_GENERIC;
}


sse_int
TDEVINFORepository_AddHardwareSim(TDEVINFORepository* self,
				  SSEString* in_iccid,
				  SSEString* in_imsi,
				  SSEString* in_msisdn)
{
  sse_int err;
  SSEString* key;
  MoatValue* value = NULL;
  SSESList* sim_list = NULL;
  MoatObject* new_sim;
  MoatValue* new_sim_value;

  ASSERT(self);

  key= sse_string_new("hardware.sim");
  ASSERT(key);

  err = TDEVINFORepository_GetDevinfo(self, key, &value);
  if (err == SSE_E_OK) {
    LOG_DEBUG("Add new if into the existing list.");
    if (moat_value_get_type(value) != MOAT_VALUE_TYPE_LIST) {
      LOG_ERROR("MoatValue type of interface is not LSIT type.");
      sse_string_free(key, sse_true);
      return SSE_E_GENERIC;
    }
    err = moat_value_get_list(value, &sim_list);
    if (err != SSE_E_OK) {
      LOG_ERROR("");
      sse_string_free(key, sse_true);
      return err;
    }
  } else if (err == SSE_E_NOENT) {
    LOG_DEBUG("Add new if into the empty list.");
    value = NULL;
    sim_list = NULL;
  } else {
    LOG_ERROR("TDEVINFORepository_GetDevinfo() ... failed with [%d].", err);
    sse_string_free(key, sse_true);
    return err;
  }

  ASSERT((new_sim = moat_object_new()));
  if (in_iccid) {
    ASSERT(moat_object_add_string_value(new_sim,
					"iccid",
					sse_string_get_cstr(in_iccid),
					sse_string_get_length(in_iccid),
					sse_true,
					sse_false) == SSE_E_OK);
  }
  if (in_imsi) {
    ASSERT(moat_object_add_string_value(new_sim,
					"imsi",
					sse_string_get_cstr(in_imsi),
					sse_string_get_length(in_imsi),
					sse_true,
					sse_false) == SSE_E_OK);
  }
  if (in_msisdn) {
    ASSERT(moat_object_add_string_value(new_sim,
					"msisdn",
					sse_string_get_cstr(in_msisdn),
					sse_string_get_length(in_msisdn),
					sse_true,
					sse_false) == SSE_E_OK);
  }

  ASSERT((new_sim_value = moat_value_new_object(new_sim, sse_false)));
  ASSERT((sim_list = sse_slist_add(sim_list, new_sim_value)));

  if (value == NULL) {
    value = moat_value_new_list(sim_list, sse_false);
    ASSERT(value);
  }
  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  return err;
}


sse_int
TDEVINFORepository_RemoveHardwareSim(TDEVINFORepository* self,
				     SSEString* in_iccid,
				     SSEString* in_imsi,
				     SSEString* in_msisdn)
{
  //@TODO Not implemented yet.
  return SSE_E_GENERIC;
}


sse_int
TDEVINFORepository_SetSoftwareOS(TDEVINFORepository* self,
				 SSEString* in_type,
				 SSEString* in_version)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_type);
  ASSERT(in_version);

  key = sse_string_new("software.os.type");
  value = moat_value_new_string(sse_string_get_cstr(in_type),
				sse_string_get_length(in_type),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  if (err != SSE_E_OK) {
    sse_string_free(key, sse_true);
    moat_value_free(value);
    return err;
  }
  sse_string_free(key, sse_true);

  key = sse_string_new("software.os.version");
  value = moat_value_new_string(sse_string_get_cstr(in_version),
				sse_string_get_length(in_version),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}


sse_int
TDEVINFORepository_SetSoftwareSscl(TDEVINFORepository* self,
				   SSEString* in_type,
				   SSEString* in_version,
				   SSEString* in_sdk_version)
{
  sse_int err;
  SSEString* key;
  MoatValue* value;

  ASSERT(self);
  ASSERT(in_type);
  ASSERT(in_version);
  ASSERT(in_sdk_version);

  key = sse_string_new("software.sscl.type");
  value = moat_value_new_string(sse_string_get_cstr(in_type),
				sse_string_get_length(in_type),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  if (err != SSE_E_OK) {
    sse_string_free(key, sse_true);
    moat_value_free(value);
    return err;
  }
  sse_string_free(key, sse_true);

  key = sse_string_new("software.sscl.version");
  value = moat_value_new_string(sse_string_get_cstr(in_version),
				sse_string_get_length(in_version),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  if (err != SSE_E_OK) {
    sse_string_free(key, sse_true);
    moat_value_free(value);
    return err;
  }
  sse_string_free(key, sse_true);

  key = sse_string_new("software.sscl.sdkVersion");
  value = moat_value_new_string(sse_string_get_cstr(in_sdk_version),
				sse_string_get_length(in_sdk_version),
				sse_true);

  err = TDEVINFORepository_SetDevinfo(self, key, value);
  sse_string_free(key, sse_true);
  moat_value_free(value);
  if (err != SSE_E_OK) {
    return err;
  }
  return SSE_E_OK;
}

