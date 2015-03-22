/*
 * LEGAL NOTICE
 *
 * Copyright (C) 2012-2013 InventIt Inc. All rights reserved.
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
 * @file	devinfo.c
 * @brief	端末情報取得 MOATアプリケーション
 *
 *			サーバからの端末情報収集要求を受け、端末情報(DeviceInfoモデル)を収集し、結果をサーバに通知するMOATアプリケーション。
 */


#include <servicesync/moat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ev.h>

/* important place or command to get device information */
#define DEVINF_UBENV_CMD_NAME				"ubenv"
#define DEVINF_FW_VERSION_FILE_PATH		"/etc/version"
#define DEVINF_CM_INFO_FILE_PATH			"/etc/cm.info"

/* known filed string*/
#define DEVINF_FIELD_SERIALNUM				"serialNumber"
#define DEVINF_FIELD_ETH_0_MAC				"eth0MacAddr"
#define DEVINF_FIELD_ETH_1_MAC				"eth1MacAddr"
#define DEVINF_FIELD_HW_VERSION			"hardwareVersion"
#define DEVINF_FIELD_FW_VERSION			"firmwareVersion"
#define DEVINF_FIELD_MODEM_TYPE			"modemModuleType"
#define DEVINF_FIELD_MODEM_REVISION		"modemModuleRevision"
#define DEVINF_FIELD_MODEM_FW_VERSION		"modemModuleFwVersion"
#define DEVINF_FIELD_ICCID					"iccid"
#define DEVINF_FIELD_IMSI					"imsi"
#define DEVINF_FIELD_MSISDN				"msisdn"
#define DEVINF_INIT_VALUE					"no-data"

/*! DeviceInfoモデルのフィールドタイプ */
enum DeviceInfoFieldType_ {
	DEVINF_TYPE_SERIALNUM = 0,
	DEVINF_TYPE_ETH_0_MAC,
	DEVINF_TYPE_ETH_1_MAC,
	DEVINF_TYPE_HW_VERSION,
	DEVINF_TYPE_FW_VERSION,
	DEVINF_TYPE_MODEM_TYPE,
	DEVINF_TYPE_MODEM_REVISION,
	DEVINF_TYPE_MODEM_FW_VERSION,
	DEVINF_TYPE_ICCID,
	DEVINF_TYPE_IMSI,
	DEVINF_TYPE_MSISDN,
	DEVINF_TYPEs
};
typedef enum DeviceInfoFieldType_ DeviceInfoFieldType;


/* model name */
#define DEVINF_MDL_NAMESTRING				"DeviceInfo"

/* service id */
#define DEVINF_SERVICEID_COLLECT			"inquire-result"

/* max heap size */
#define DEVINF_MAXHEAP_SZ					1024
/* max token level */
#define DEVINF_MAXTOKEN_LV					128


/*! DeviceInfoモデルコンテキスト */
struct DeviceInfoContext_ {
	Moat Moat;					/**< moat object		*/
	sse_char *AppID;			/**< set addres to app id is same as string of argv[0] */
	sse_char *ServiceID;		/**< set address to service id string.*/
	sse_char *AsyncKey;			/**< save pointer to key string which is copied from receiving key at command case */
	sse_char *ModelName;		/**< set const address to model name. NO NEED TO RELEASE HEAP */
};
typedef struct DeviceInfoContext_ DeviceInfoContext;

/*! DeviceInfoフィールド情報型 */
struct DeviceInfoFiledInfo_ {
	sse_char *field;
	DeviceInfoFieldType type;
};
typedef struct DeviceInfoFiledInfo_ DeviceInfoFiledInfo;


/*! DeviceInfoフィールド情報 */
const DeviceInfoFiledInfo flist[] = {
										{DEVINF_FIELD_SERIALNUM, DEVINF_TYPE_SERIALNUM},
										{DEVINF_FIELD_ETH_0_MAC, DEVINF_TYPE_ETH_0_MAC},
										{DEVINF_FIELD_ETH_1_MAC, DEVINF_TYPE_ETH_1_MAC},
										{DEVINF_FIELD_HW_VERSION, DEVINF_TYPE_HW_VERSION},
										{DEVINF_FIELD_FW_VERSION, DEVINF_TYPE_FW_VERSION},
										{DEVINF_FIELD_MODEM_TYPE, DEVINF_TYPE_MODEM_TYPE},
										{DEVINF_FIELD_MODEM_REVISION, DEVINF_TYPE_MODEM_REVISION},
										{DEVINF_FIELD_MODEM_FW_VERSION, DEVINF_TYPE_MODEM_FW_VERSION},
										{DEVINF_FIELD_ICCID, DEVINF_TYPE_ICCID},
										{DEVINF_FIELD_IMSI, DEVINF_TYPE_IMSI},
										{DEVINF_FIELD_MSISDN, DEVINF_TYPE_MSISDN},
										{NULL, DEVINF_TYPEs}
									};


#define DEVINF_TAG						"devinfo"
#if defined(SSE_LOG_USE_SYSLOG)
#define LOG_DI_ERR(format, ...)			SSE_SYSLOG_ERROR(DEVINF_TAG, format, ##__VA_ARGS__)
#define LOG_DI_DBG(format, ...)			SSE_SYSLOG_DEBUG(DEVINF_TAG, format, ##__VA_ARGS__)
#define LOG_DI_INF(format, ...)			SSE_SYSLOG_INFO(DEVINF_TAG, format, ##__VA_ARGS__)
#define LOG_DF_TRC(format, ...)			SSE_SYSLOG_TRACE(DEVINF_TAG, format, ##__VA_ARGS__)
#define LOG_DI_IN()						SSE_SYSLOG_TRACE(DEVINF_TAG, "enter")
#define LOG_DI_OUT()						SSE_SYSLOG_TRACE(DEVINF_TAG, "exit")
#else
#define LOG_PRINT(type, tag, format, ...)	printf("[" type "]" tag " %s():L%d " format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_DI_ERR(format, ...)			LOG_PRINT("**ERROR**", DEVINF_TAG, format, ##__VA_ARGS__)
#define LOG_DI_DBG(format, ...)			LOG_PRINT("DEBUG", DEVINF_TAG, format, ##__VA_ARGS__)
#define LOG_DI_INF(format, ...)			LOG_PRINT("INFO", DEVINF_TAG, format, ##__VA_ARGS__)
#define LOG_DF_TRC(format, ...)			LOG_PRINT("TRACE", DEVINF_TAG, format, ##__VA_ARGS__)
#define LOG_DI_IN()						LOG_PRINT("TRACE", DEVINF_TAG, "enter")
#define LOG_DI_OUT()						LOG_PRINT("TRACE", DEVINF_TAG, "exit")
#endif

/////////////////////// Utility API ///////////////////////

/**
 * @brief	改行削除
 *　			文字列から改行を削除('\0'に置換)する。
 *
 * @param	str 文字列
 * @param	len 文字列長
 */
static void
remove_new_line(sse_char *str, sse_uint len)
{
	sse_uint i = 0;

	if (str == NULL || len <= 0) {
		return;
	}

	for (i = 0; i < len; i++) {
		if (str[i] == 0x0A) {
			str[i] = 0x0;
			break;
		}
	}
	return;
}

/**
 * @brief	文字列分割
 *　			文字列を指定された区切り文字列で分割し、分割した文字列の配列を返却する。
 *
 * @param	string 文字列
 * @param	delim 区切り文字列
 * @param	maxtokenlevel トークンの最大分割数
 * @return	トークンの配列
 */
static char **
split_string(const sse_char *string, const sse_char *delim, sse_uint maxtokenlevel)
{

	sse_char **tokens = NULL;
	sse_char *working = NULL;
	sse_char *token = NULL;
	sse_int idx = 0;

	if (string == NULL || delim == NULL || maxtokenlevel == 0) {
		return NULL;
	}

	tokens = sse_malloc(sizeof(char *) * maxtokenlevel);
	if (tokens == NULL) {
		return NULL;
	}

	working = sse_malloc(sizeof(char) * sse_strlen(string) + 1);
	if (working == NULL) {
		return NULL;
	}

	/* to make sure, copy string to a safe place */
	sse_strcpy(working, string);
	for (idx = 0; idx < maxtokenlevel; idx++) {
		tokens[idx] = NULL;
	}

	token = strtok(working, delim);
	idx = 0;

	/* always keep the last entry NULL termindated */
	while ((idx < (maxtokenlevel - 1)) && (token != NULL)) {
		tokens[idx] = sse_malloc(sizeof(char) * sse_strlen(token) + 1);
		if (tokens[idx] != NULL) {
			sse_strcpy(tokens[idx], token);
			idx++;
			token = strtok(NULL, delim);
		}
	}

	free(working);
	return tokens;
}

/**
 * @brief	トークン配列の解放
 *　			トークン配列を解放する
 *
 * @param	tokens トークン配列
 */
static void
delete_split_list(sse_char **tokens)
{
	sse_int i;

	if (tokens != NULL) {
		for (i = 0; tokens[i] != NULL; i++) {
			sse_free(tokens[i]);
		}
		sse_free(tokens);
	}
	return;
}

/**
 * @brief	通知IDの生成
 *			URNとサービスIDから通知IDを生成する。
 *			通知IDのフォーマット：
 *			"urn:moat:" + ${APP_ID} + ":" + ${PACKAGE_ID} + ":" + ${SERVICE_NAME} + ":" + ${VERSION}
 *
 * @param	in_urn URN
 * @param	in_service_name サービス名
 * @return	通知ID（呼び出し側で解放すること）
 */
static sse_char *
create_notification_id(sse_char *in_urn, sse_char *in_service_name)
{
	sse_char *prefix = "urn:moat:";
	sse_uint prefix_len;
	sse_char *suffix = ":1.0";
	sse_uint suffix_len;
	sse_uint urn_len;
	sse_uint service_len;
	sse_char *noti_id = NULL;
	sse_char *p;

	prefix_len = sse_strlen(prefix);
	urn_len = sse_strlen(in_urn);
	service_len = sse_strlen(in_service_name);
	suffix_len = sse_strlen(suffix);
	noti_id = sse_malloc(prefix_len + urn_len + 1 + service_len + suffix_len + 1);
	if (noti_id == NULL) {
		return NULL;
	}
	p = noti_id;
	sse_memcpy(p, prefix, prefix_len);
	p += prefix_len;
	sse_memcpy(p, in_urn, urn_len);
	p += urn_len;
	*p = ':';
	p++;
	sse_memcpy(p, in_service_name, service_len);
	p += service_len;
	sse_memcpy(p, suffix, suffix_len);
	p += suffix_len;
	*p = '\0';
	return noti_id;
}

/**
 * @brief	タグ情報取得
 *　			指定されたファイルから、指定されたタグの内容を取得する。
 *
 * @param	in_file 検索対象のファイルへのパス
 * @param	tag 取得するタグ
 * @param	out_addr 検索結果の出力先
 * @return	エラーコード
 */
static sse_int
pick_line_having_target_tag(const sse_char *in_file, const sse_char *tag, sse_char **out_addr)
{
	FILE *fp;
	sse_char *p = NULL;
	sse_int err = SSE_E_GENERIC;

	if (in_file == NULL || tag == NULL || out_addr == NULL) {
		LOG_DI_ERR("invalid arg. err");
		return SSE_E_INVAL;
	}

	/* init */
	*out_addr = NULL;

	fp = fopen(in_file, "r");
	if (fp == NULL) {
		LOG_DI_DBG("failed to open file. err");
		return SSE_E_INVAL;
	}

	p = sse_malloc(DEVINF_MAXHEAP_SZ + 1); /* +1 for null terminate by fgets*/
	if (p == NULL) {
		LOG_DI_ERR("nomem. err");
		fclose(fp);
		return SSE_E_NOMEM;
	}

	do {
		if (fgets(p, DEVINF_MAXHEAP_SZ, fp) != NULL) {
			if (sse_strstr(p, tag) != NULL) {
				/* found tag */
				remove_new_line(p, (DEVINF_MAXHEAP_SZ + 1));
				/* copy target line */
				*out_addr = sse_strdup(p);
				if (out_addr == NULL) {
					LOG_DI_ERR("nomem. err");
					err = SSE_E_NOMEM;
				} else {
					err = SSE_E_OK;
				}
				/* break while loop */
				break;
			}
		}
	} while (feof(fp) == 0);

	free(p);
	fclose(fp);
	return err;
}

/**
 * @brief	タグ情報取得
 *　			指定されたファイルから、指定されたタグの内容を取得する。
 *
 * @param	in_file 検索対象のファイルへのパス
 * @param	tag 取得するタグ
 * @param	out_addr 検索結果の出力先
 * @return	エラーコード
 */
static sse_int
clone_file_to_heap(const sse_char *in_file, sse_char **out_addr, sse_uint *out_length)
{
	sse_int fd = -1;
	sse_int err = SSE_E_GENERIC;
	struct stat st;

	sse_char *p = NULL;
	sse_int read_bytes;

	if (in_file == NULL || out_addr == NULL || out_length == NULL) {
		LOG_DI_ERR("invalid args. err");
		return SSE_E_INVAL;
	}

	/* init */
	*out_addr = NULL;
	*out_length = 0;

	fd = open(in_file, O_RDONLY);
	if (fd < 0) {
		LOG_DI_ERR("failed to open file :: %s", strerror(errno));
		return SSE_E_INVAL;
	}

	err = fstat(fd, &st);
	if (err < 0) {
		LOG_DI_ERR("failed to ftat :: %s", strerror(errno));
		err = SSE_E_INVAL;
		goto done;
	}

	p = sse_malloc(st.st_size + 1);
	if (p == NULL) {
		LOG_DI_ERR("nomemory. err");
		err = SSE_E_NOMEM;
		goto done;
	}
	sse_memset(p, 0, st.st_size + 1);
	*out_addr = p;
	while (1) {
		read_bytes = read(fd, p, st.st_size);
		if (read_bytes < 0) {
			LOG_DI_ERR("failed to read file :: %s", strerror(errno));
			err = SSE_E_INVAL;
			goto done;
		}
		p += read_bytes;
		if (read_bytes <= st.st_size) {
			break;
		}
	}
	close(fd);
	*out_length = st.st_size;
	return SSE_E_OK;

done:
	if (out_addr != NULL) {
		sse_free(out_addr);
	}
	if (fd > 0) {
		close(fd);
	}
	return err;
}

/////////////////////// Update Field Value API ///////////////////////

/**
 * @brief	タグ文字列取得
 *　			指定されたフィールドタイプから、cm.infoファイルのタグ文字列を取得する。
 *
 * @param	DevInfoモデルのフィールドタイプ
 * @return	タグ文字列
 */
static sse_char *
devinf_get_cminfo_tag(DeviceInfoFieldType type)
{
	sse_char *tag = NULL;

	switch (type) {
	case DEVINF_TYPE_MODEM_TYPE:
		tag = "PRODUCT_MODEL_ID";
		break;
	case DEVINF_TYPE_MODEM_REVISION:
		tag = "SW_VERSION";
		break;
	case DEVINF_TYPE_MODEM_FW_VERSION:
		tag = "HW_VERSION";
		break;
	case DEVINF_TYPE_ICCID:
		tag = "ICCID";
		break;
	case DEVINF_TYPE_IMSI:
		tag = "IMSI";
		break;
	case DEVINF_TYPE_MSISDN:
		tag = "MSISDN";
		break;
	default:
		break;
	}
	return tag;
}

/**
 * @brief	ubenvパラメータ取得
 *　			指定されたフィールドタイプから、ubenvに渡すパラメータ文字列を取得する。
 *
 * @param	DevInfoモデルのフィールドタイプ
 * @return	パラメータ文字列
 */
static sse_char *
devinf_get_ubenv_param_string(DeviceInfoFieldType type)
{
	sse_char *tag = NULL;

	switch (type) {
	case DEVINF_TYPE_SERIALNUM:
		tag = "serial_number";
		break;
	case DEVINF_TYPE_ETH_0_MAC:
		tag = "ethaddr";
		break;
	case DEVINF_TYPE_ETH_1_MAC:
		tag = "eth1addr";
		break;
	case DEVINF_TYPE_HW_VERSION:
		tag = "hw_version";
		break;
	default:
		break;
	}
	return tag;
}

/**
 * @brief	値文字列複製
 *　			${TAG}${DELIM}${VALUE}形式の文字列リストから、指定された${TAG}の${VALUE}の内容を取り出し、複製を返却する。
 *
 * @param	target_buf 対象文字列
 * @param	delim 区切り文字列
 * @param	tag 検索するタグ
 * @return	取得した値文字列
 */
static sse_char *
devinf_clone_target_tag_string(const sse_char *target_buf, const sse_char *delim, const sse_char *tag)
{
	sse_char *val = NULL;
	sse_char **token_list = NULL;
	sse_int target_len = 0;
	sse_int token_len = 0;
	sse_int idx = 0;

	if (target_buf == NULL || delim == NULL || tag == NULL) {
		LOG_DI_ERR("invalid args. err");
		return NULL;
	}

	token_list = split_string(target_buf, delim, DEVINF_MAXTOKEN_LV);
	if (token_list == NULL) {
		LOG_DI_ERR(
		        "failed to split string. err. string = %s, delim = %s, tag = %s",
		        target_buf, delim, tag);
		return NULL;
	}

	target_len = sse_strlen(tag);
	while (token_list[idx] != NULL) {
		token_len = sse_strlen(token_list[idx]);
		LOG_DI_DBG("@@@ tag=%s, targetlen=%d, token=%s, tokenlen=%d",
		        tag, target_len, token_list[idx], token_len);
		if ((target_len == token_len)
		        && (sse_strncmp(tag, token_list[idx], target_len) == 0)) {
			val = sse_strdup(token_list[idx + 1]);
			break;
		}
		idx++;
	}
	delete_split_list(token_list);
	return val;
}

/**
 * @brief	ubenv情報取得
 *　			ubenvコマンドを呼び出し端末の情報を取得する。
 *
 * @param	tag ubenvのパラメータ文字列
 * @return	取得した値文字列
 */
static sse_char *
devinf_get_ubenv_info(const sse_char *tag)
{
	FILE *fp;
	sse_char *buffer = NULL;
	sse_char *val = NULL;
	sse_char *cmd_line = NULL;
	sse_int cmdline_buflen = 64;
	sse_int cmd_line_len = 0;

	if (tag == NULL) {
		LOG_DI_ERR("invalid args. err");
		return NULL;
	}

	buffer = sse_malloc(DEVINF_MAXHEAP_SZ);
	if (buffer == NULL) {
		LOG_DI_ERR("nomem. err");
		return NULL;
	}
	cmd_line = sse_malloc(cmdline_buflen);
	if (cmd_line == NULL) {
		sse_free(buffer);
		LOG_DI_ERR("nomem. err");
		return NULL;
	}
	sse_memset(buffer, 0, DEVINF_MAXHEAP_SZ);
	sse_memset(cmd_line, 0, cmdline_buflen);

	cmd_line_len = sprintf(cmd_line, "%s -r %s", DEVINF_UBENV_CMD_NAME, tag);
	if ((cmd_line_len <= 0) || (cmd_line_len > cmdline_buflen)) {
		sse_free(cmd_line);
		sse_free(buffer);
		LOG_DI_ERR("failed to create command string. err");
		return NULL;
	}

	fp = popen(cmd_line, "r");
	if (fp == NULL) {
		sse_free(cmd_line);
		sse_free(buffer);
		LOG_DI_ERR("failed to popen. err");
		return NULL;
	}
	/* we assume that getting data is only once.*/
	//fgets(buffer, DEVINF_MAXHEAP_SZ, fp);
	while (fgets(buffer, DEVINF_MAXHEAP_SZ, fp)) {
	}
	/* close soon */
	pclose(fp);

	remove_new_line(buffer, DEVINF_MAXHEAP_SZ);
	val = sse_strdup(buffer);
	sse_free(cmd_line);
	sse_free(buffer);
	return val;
}

/**
 * @brief	ファームウェアバージョンの取得
 *　			バージョン情報ファイルからバージョンを取得する。
 *
 * @return	取得した値文字列
 */
static sse_char *
devinf_get_fw_version(void)
{
	sse_int err = SSE_E_GENERIC;
	sse_char *buffer = NULL;
	sse_uint read_len = 0;

	err = clone_file_to_heap(DEVINF_FW_VERSION_FILE_PATH, &buffer, &read_len);
	if (err) {
		LOG_DI_ERR("failed to read file. filename =  %s",
		        DEVINF_FW_VERSION_FILE_PATH);
		return NULL;
	}

	return buffer;
}

/**
 * @brief	モデム情報取得
 *　			cm.infoファイルからモデムの情報を取得する。
 *
 * @param	tag cm.infoのタグ文字列
 * @return	取得した値文字列
 */
static sse_char *
devinf_get_modem_info(const sse_char *tag)
{
	sse_int err = SSE_E_GENERIC;
	sse_char *val = NULL;
	sse_char *buffer = NULL;

	if (tag == NULL) {
		LOG_DI_ERR("invalid args. err");
		return NULL;
	}
	err = pick_line_having_target_tag(DEVINF_CM_INFO_FILE_PATH, tag, &buffer);
	if (err) {
		LOG_DI_DBG("failed to read file. filename =  %s. maybe its not modem model.",
				DEVINF_CM_INFO_FILE_PATH);
		return NULL;
	}

	val = devinf_clone_target_tag_string(buffer, "=", tag);
	sse_free(buffer);
	return val;
}

/**
 * @brief	フィールド値取得
 *　			端末情報のフィールド値を取得する。
 *
 * @param	info 端末情報のフィールド情報
 * @return	取得した値文字列
 */
static sse_char *
devinf_update_field_value(DeviceInfoFiledInfo info)
{
	sse_char *val = NULL;

	switch (info.type) {
	case DEVINF_TYPE_SERIALNUM:
	case DEVINF_TYPE_ETH_0_MAC:
	case DEVINF_TYPE_ETH_1_MAC:
	case DEVINF_TYPE_HW_VERSION:
		/* use ubenv command */
		val = devinf_get_ubenv_info(devinf_get_ubenv_param_string(info.type));
		break;
	case DEVINF_TYPE_FW_VERSION:
		/* read "/etc/version */
		val = devinf_get_fw_version();
		break;
	case DEVINF_TYPE_MODEM_TYPE:
	case DEVINF_TYPE_MODEM_REVISION:
	case DEVINF_TYPE_MODEM_FW_VERSION:
	case DEVINF_TYPE_ICCID:
	case DEVINF_TYPE_IMSI:
	case DEVINF_TYPE_MSISDN:
		/* read /etc/cm.info */
		val = devinf_get_modem_info(devinf_get_cminfo_tag(info.type));
		break;
	default:
		break;
	}

	return val;
}

/////////////////////// Timer API ///////////////////////

/**
 * @brief	非同期処理実行開始コールバック
 *　			非同期コマンド実行時のコールバック。
 *			実際のコマンド処理の実行を開始する。
 *
 * @param	in_moat MOATハンドル
 * @param	in_uid UUID
 * @param	in_key 非同期実行キー
 * @param	in_data コマンドパラメータ
 * @param	in_model_context モデルコンテキスト
 * @param	処理結果
 */
static sse_int
devinf_async_command_cb(Moat in_moat, sse_char *in_uid, sse_char *in_key, MoatValue *in_data, sse_pointer in_model_context)
{
	DeviceInfoContext *self = (DeviceInfoContext *)in_model_context;
	MoatObject *latest_obj = NULL;
	sse_int idx = 0;
	sse_char *field = NULL;
	sse_char *latest_val = NULL;
	sse_int err = SSE_E_GENERIC;
	sse_int req_id;

	LOG_DI_IN();
	latest_obj = moat_object_new();
	if (latest_obj == NULL) {
		LOG_DI_ERR("nomem.err");
		goto done;
	}

	while (flist[idx].field != NULL) {

		/* DeviceInfoモデルのフィールド値を更新 */
		LOG_DI_DBG("LOOP %02d, field; %s, type = %d",idx, flist[idx].field, flist[idx].type);
		latest_val = devinf_update_field_value(flist[idx]);
		if (latest_val == NULL) {
			LOG_DI_DBG("failed to get value. devinfo doesn't attach %s field as moat object via notification", field);
			sse_free(field);
			field = NULL;
			err = SSE_E_GENERIC;
		} else {
			/* 更新した値でモデルデータのフィールド値を更新 */
			err = moat_object_add_string_value(latest_obj, flist[idx].field, latest_val, 0, sse_false, sse_true);
			if (err) {
				LOG_DI_ERR("failed to add string field. err=[%d]", err);
				goto done;
			}
		}
		idx++;
	}

	/* 端末情報収集結果をサーバに通知 */
	req_id = moat_send_notification(self->Moat, self->ServiceID, self->AsyncKey, self->ModelName, latest_obj, NULL, NULL);
	if (req_id < 0) {
		LOG_DI_ERR("failed to send notification. error code = %d", req_id);
		err = req_id;
	}

done:
	if (self->AsyncKey != NULL) {
		sse_free(self->AsyncKey);
		self->AsyncKey = NULL;
	}
	if (self->ServiceID != NULL) {
		sse_free(self->ServiceID);
		self->ServiceID = NULL;
	}
	if (latest_obj != NULL) {
		moat_object_free(latest_obj);
	}
	LOG_DI_OUT();
	return err;
}

/////////////////////// Command API ///////////////////////

/**
 * @brief	DeviceInfoモデルデータ取得
 *　			DeviceInfoモデルのデータ取得コマンドの実装。
 *　			端末の情報を収集し、サーバに結果を通知する。
 *
 * @param	in_moat Moatインスタンス
 * @param	in_uid 対象データのUUID (DeviceInfoはシングルトンのためNULL固定)
 * @param	in_key 非同期結果通知を行う際に使用するキー
 * @param	in_data コマンドの引数データ(collect()はパラメータなし)
 * @param	in_model_context コンテキスト
 * @return	コマンド実行結果: SSE_E_INPROGRESS (要求受付、非同期で結果通知), SSE_E_INPROGRESS以外はエラー
 */
sse_int
DeviceInfo_collect(Moat in_moat, sse_char *in_uid, sse_char *in_key,
        MoatValue *in_data, sse_pointer in_model_context)
{
	DeviceInfoContext *self = NULL;
	sse_int err = SSE_E_GENERIC;

	if (in_key == NULL || in_data == NULL || in_model_context == NULL) {
		LOG_DI_ERR("invalid args. err");
		return SSE_E_INVAL;
	}

	self = (DeviceInfoContext *)in_model_context;

	self->ServiceID = create_notification_id(self->AppID,
	        DEVINF_SERVICEID_COLLECT);
	if (self->ServiceID == NULL) {
		LOG_DI_ERR("failed to create service id, err.");
		err = SSE_E_INVAL;
		goto err_exit;
	}

	self->AsyncKey = sse_strdup(in_key);
	if (self->AsyncKey == NULL) {
		LOG_DI_ERR("nomem.err");
		err = SSE_E_INVAL;
		goto err_exit;
	}

	/* 非同期で結果通知を行うための処理開始。コマンド処理でブロックしないようにするために行う。 */
	err = moat_start_async_command(in_moat, in_uid, in_key, in_data, devinf_async_command_cb, self);
	if (err) {
		LOG_DI_ERR("failed to start async task, err = %d", err);
		err = SSE_E_GENERIC;
		goto err_exit;
	}

	LOG_DI_DBG("Comand returns as IMPROGRESS");
	return SSE_E_INPROGRESS;


err_exit:
	if (self->ServiceID != NULL) {
		sse_free(self->ServiceID);
		self->ServiceID = NULL;
	}
	if (self->AsyncKey != NULL) {
		sse_free(self->AsyncKey);
		self->AsyncKey = NULL;
	}
	return err;
}

/////////////////////// Application API ///////////////////////

/**
 * @brief	DeviceInfoモデルのコンテキスト生成
 *　			DeviceInfoモデルのコンテキストインスタンスを生成し返却する。
 *
 * @param	in_moat Moatインスタンス
 * @return	DeviceInfoモデルのコンテキストインスタンス
 */
static DeviceInfoContext *
devinfo_context_new(Moat in_moat)
{
	DeviceInfoContext *c = NULL;

	c = sse_malloc(sizeof(DeviceInfoContext));
	if (c == NULL) {
		return NULL;
	}
	sse_memset(c, 0, sizeof(DeviceInfoContext));
	c->Moat = in_moat;
	return c;
}

/**
 * @brief	DeviceInfoモデルのコンテキスト解放
 *　			DeviceInfoモデルのコンテキストインスタンスを解放する。
 *
 * @param	self 解放するインスタンス
 */
static void
devinfo_context_free(DeviceInfoContext *self)
{
	if (self != NULL) {
		if (self->AppID != NULL) {
			sse_free(self->AppID);
			self->AppID = NULL;
		}
		if (self->AsyncKey != NULL) {
			sse_free(self->AsyncKey);
			self->AsyncKey = NULL;
		}
		if (self->ServiceID != NULL) {
			sse_free(self->ServiceID);
			self->ServiceID = NULL;
		}
		free(self);
	}
}


////////////////////// Entry Point ///////////////////////

/**
 * @brief	DevInfoアプリエントリポイント
 *　			DevInfoアプリのメイン関数。
 *
 * @param	argc 引数の数
 * @param	引数
 * @return	終了コード
 */
sse_int
moat_app_main(sse_int argc, sse_char *argv[])
{
	Moat moat;
	ModelMapper mapper;
	sse_int err;
	DeviceInfoContext *devinfo_context = NULL;
	sse_int result = EXIT_SUCCESS;
	sse_char *argv_0 = NULL;

	LOG_DI_DBG("h-e-l-l-(o) m-o-a-t...");
	err = moat_init(argv[0], &moat);
	if (err != SSE_E_OK) {
		LOG_DI_ERR("failed to initialize.");
		result = EXIT_FAILURE;
		goto done;
	}

	argv_0 = sse_strdup(argv[0]);
	if (argv_0 == NULL) {
		LOG_DI_ERR("nomem. err");
		return EXIT_FAILURE;
	}

	/* DevInfoモデルのコンテキスト生成 */
	devinfo_context = devinfo_context_new(moat);
	if (devinfo_context == NULL) {
		LOG_DI_ERR("failed to create context.");
		return EXIT_FAILURE;
	} else {
		devinfo_context->ModelName = DEVINF_MDL_NAMESTRING;
		devinfo_context->AppID = argv_0;
	}

	/* DevInfoは、Mapperメソッドなし, コマンドのみ */
	mapper.AddProc = NULL;
	mapper.RemoveProc = NULL;
	mapper.UpdateProc = NULL;
	mapper.UpdateFieldsProc = NULL;
	mapper.FindAllUidsProc = NULL;
	mapper.FindByUidProc = NULL;
	mapper.CountProc = NULL;
	err = moat_register_model(moat, devinfo_context->ModelName, &mapper, devinfo_context);
	if (err != SSE_E_OK) {
		LOG_DI_ERR("failed to register model.");
		result = EXIT_FAILURE;
		goto done;
	}

	/* イベントループ実行 */
	moat_run(moat);

done:
	moat_unregister_model(moat, devinfo_context->ModelName);
	devinfo_context_free(devinfo_context);
	moat_destroy(moat);
	LOG_DI_DBG("devinfo end.");
	return result;
}
