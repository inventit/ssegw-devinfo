#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <servicesync/moat.h>
#include <sseutils.h>
#include <devinfo/devinfo.h>

jmp_buf g_env;
sse_int g_success = 0;
sse_int g_failure = 0;
sse_int g_unknown = 0;

const sse_int TEST_RESULT_SUCCESS = 0;
const sse_int TEST_RESULT_FAILURE = 1;
const sse_int TEST_RESULT_UNKNOWN = 2;

#define LOG_PRINT(format, ...) printf("## TEST ## %s L%d: " format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DO_TEST(func) {				\
  sse_int ret;					\
  sse_int result;				\
  LOG_PRINT("Test case: " #func);		\
  if ((ret = setjmp(g_env)) == 0) {		\
    result = func;				\
    if (result == TEST_RESULT_SUCCESS) {	\
      LOG_PRINT(#func "... Success\n");		\
      g_success++;				\
    } else {					\
      LOG_PRINT(#func "... Unknown\n");		\
      g_unknown++;				\
    }						\
  } else if (ret == 1) {			\
    LOG_PRINT(#func "... Failure\n");		\
    g_failure++;				\
  }						\
}

#define ASSERT(cond) {				\
  if (!(cond)) {				\
    LOG_PRINT("Assersion failure: " #cond);	\
    longjmp(g_env, 1);				\
  }						\
}

static sse_int
test_devinfo_repository__load_devinfo_from_file(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* path;
  SSEString* json = NULL;
  sse_char* json_cstr;
  sse_char* expected = "{\"hardware\":{\"platform\":{\"vendor\":\"Inventit\",\"product\":\"IVI IoT Gateway\",\"model\":\"IVI-0001\",\"serial\":\"123456\",\"hwVersion\":\"1.0.0\",\"fwVersion\":\"2.0.1\",\"deviceId\":\"IVI:SN:123456\",\"category\":\"gateway\"},\"modem\":{\"type\":\"XXX\",\"hwVersion\":\"rev1.2.0\",\"fwVersion\":\"v2.0.0\"},\"network\":{\"interface\":[{\"name\":\"eth0\",\"hwAddress\":\"00:11:22:33:44:55\",\"ipv4Address\":\"192.168.1.25\",\"netmask\":\"255.255.255.0\",\"ipv6Address\":\"fe80::a00:27ff:feaa:38a9/64\"},{\"name\":\"usb0\",\"hwAddress\":\"EE:07:72:2F:01:07\",\"ipv4Address\":\"153.158.37.30\",\"netmask\":\"255.255.255.255\",\"ipv6Address\":\"fe80::ec07:72ff:fe2f:107/64\"}],\"nameserver\":[\"222.146.35.1\",\"221.184.25.1\"]},\"sim\":[{\"iccid\":\"898110011111111111111\",\"imsi\":\"121234561234560\",\"msisdn\":\"09012345678\"}]},\"software\":{\"os\":{\"type\":\"Linux\",\"version\":\"2.6.26\"},\"sscl\":{\"type\":\"SSEGW\",\"version\":\"1.0.6\",\"sdkVersion\":\"1.0.5\"}}}";

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat, NULL) == SSE_E_OK);

  // Load devinfo from the file.
  ASSERT((path = sse_string_new("./devinfo.json")) != NULL);
  ASSERT(TDEVINFORepository_LoadDevinfo(&repo, path) == SSE_E_OK);

  // Seriallize with JSON
  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Actual   = [%s]\n", json_cstr);
  LOG_PRINT("Expected = [%s]\n", expected);
  ASSERT(sse_strcmp(json_cstr, expected) == 0);

  // Cleanup
  TDEVINFORepository_Finalize(&repo);
  sse_string_free(path, sse_true);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);

  return TEST_RESULT_SUCCESS;
}

static sse_int
test_devinfo_repository__add_vendor(Moat in_moat)
{
  TDEVINFORepository repo;
  MoatValue* vendor;
  SSEString* json = NULL;
  sse_char* json_cstr;
  sse_char expected[] = "{\"hardware\":{\"platform\":{\"vendor\":\"ABC Corp.\"}}}";

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat, NULL) == SSE_E_OK);

  // Add hardware.platform.vendor into empty repository.
  ASSERT((vendor = moat_value_new_string("ABC Corp.", 0, sse_true)) != NULL);
  ASSERT(TDEVINFORepository_SetHardwarePlatformVendor(&repo, vendor) == SSE_E_OK);
  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Devinfo = [%s]", json_cstr);
  ASSERT(sse_strcmp(json_cstr, expected) == 0);

  // Cleanup
  TDEVINFORepository_Finalize(&repo);
  moat_value_free(vendor);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);

  return TEST_RESULT_SUCCESS;
}

static sse_int
test_devinfo_repository__overwrite_vendor(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* path;
  MoatValue* vendor;
  SSEString* json = NULL;
  sse_char* json_cstr;
  sse_char expected[] = "ABC Corp.";

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat, NULL) == SSE_E_OK);

  ASSERT((path = sse_string_new("./devinfo.json")) != NULL);
  ASSERT(TDEVINFORepository_LoadDevinfo(&repo, path) == SSE_E_OK);

  ASSERT((vendor = moat_value_new_string("ABC Corp.", 0, sse_true)) != NULL);
  ASSERT(TDEVINFORepository_SetHardwarePlatformVendor(&repo, vendor) == SSE_E_OK);

  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Devinfo = [%s]", json_cstr);
  ASSERT(sse_strstr(json_cstr, expected));

  // Cleanup
  TDEVINFORepository_Finalize(&repo);
  sse_string_free(path, sse_true);
  moat_value_free(vendor);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);

  return TEST_RESULT_SUCCESS;
}


static sse_int
test_devinfo_repository__add_interfaces(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* json = NULL;
  sse_char* json_cstr;

  MoatValue* eth0_name;
  MoatValue* eth0_macaddr;
  MoatValue* eth0_ipv4addr;
  MoatValue* eth0_netmask;
  MoatValue* eth0_ipv6addr;

  MoatValue* usb0_name;
  MoatValue* usb0_macaddr;
  MoatValue* usb0_ipv4addr;
  MoatValue* usb0_netmask;
  MoatValue* usb0_ipv6addr;

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat, NULL) == SSE_E_OK);

  // Register intaface info
  ASSERT((eth0_name     = moat_value_new_string("eth0", 0, sse_true)));
  ASSERT((eth0_macaddr  = moat_value_new_string("00:11:22:aa:bb:cc", 0, sse_true)));
  ASSERT((eth0_ipv4addr = moat_value_new_string("192.128.1.32", 0, sse_true)));
  ASSERT((eth0_netmask  = moat_value_new_string("255.255.255.0", 0, sse_true)));
  ASSERT((eth0_ipv6addr = moat_value_new_string("fe80::a00:27ff:fea9:d684/64", 0, sse_true)));

  ASSERT((usb0_name     = moat_value_new_string("usb0", 0, sse_true)));
  ASSERT((usb0_macaddr  = moat_value_new_string("44:55:66:dd:ee:ff", 0, sse_true)));
  ASSERT((usb0_ipv4addr = moat_value_new_string("10.0.2.15", 0, sse_true)));
  ASSERT((usb0_netmask  = moat_value_new_string("255.255.254.0", 0, sse_true)));
  ASSERT((usb0_ipv6addr = moat_value_new_string("fe80::a00:27ff:fea9:d684/64", 0, sse_true)));

  ASSERT(TDEVINFORepository_AddHardwareNetworkInterface(&repo, eth0_name, eth0_macaddr, eth0_ipv4addr, eth0_netmask, eth0_ipv6addr) == SSE_E_OK);
  ASSERT(TDEVINFORepository_AddHardwareNetworkInterface(&repo, usb0_name, usb0_macaddr, usb0_ipv4addr, usb0_netmask, usb0_ipv6addr) == SSE_E_OK);

  // Print devinfo with JSON
  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Devinfo = [%s]", json_cstr);

  ASSERT(sse_strstr(json_cstr, "eth0"));
  ASSERT(sse_strstr(json_cstr, "usb0"));

  // Cleanup
  TDEVINFORepository_Finalize(&repo);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);

  moat_value_free(eth0_name);
  moat_value_free(eth0_macaddr);
  moat_value_free(eth0_ipv4addr);
  moat_value_free(eth0_netmask);
  moat_value_free(eth0_ipv6addr);

  moat_value_free(usb0_name);
  moat_value_free(usb0_macaddr);
  moat_value_free(usb0_ipv4addr);
  moat_value_free(usb0_netmask);
  moat_value_free(usb0_ipv6addr);

  return TEST_RESULT_SUCCESS;
}

static sse_int
test_devinfo_repository__add_all_items(Moat in_moat)
{
  TDEVINFORepository repo;
  MoatValue* item;
  SSEString* json = NULL;
  sse_char* json_cstr;

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat, NULL) == SSE_E_OK);

  // Add all items
  { // Hardware
    { // Platform
      // Vendor
      ASSERT((item = moat_value_new_string("Raspberry Pi Fundation", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformVendor(&repo, item) == SSE_E_OK);
      moat_value_free(item);

      // Product
      ASSERT((item = moat_value_new_string("Raspberry Pi", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformProduct(&repo, item) == SSE_E_OK);
      moat_value_free(item);

      // Model
      ASSERT((item = moat_value_new_string("RASPBERRY PI 1 MODEL B+", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformModel(&repo, item) == SSE_E_OK);
      moat_value_free(item);

      // Serial No.
      ASSERT((item = moat_value_new_string("ABC12345", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformSerial(&repo, item) == SSE_E_OK);
      moat_value_free(item);

      // Hardware version
      ASSERT((item = moat_value_new_string("1.2.3", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformHwVersion(&repo, item) == SSE_E_OK);
      moat_value_free(item);

      // Firmware versionn
      ASSERT((item = moat_value_new_string("2.4.5", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformFwVersion(&repo, item) == SSE_E_OK);
      moat_value_free(item);

      // Device Id
      ASSERT((item = moat_value_new_string("RSP:SN:123456", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformDeviceId(&repo, item) == SSE_E_OK);
      moat_value_free(item);

      // Category
      ASSERT((item = moat_value_new_string("gateway", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformCategory(&repo, item) == SSE_E_OK);
      moat_value_free(item);
    }
    { // Modem
      // Type
      ASSERT((item = moat_value_new_string("MC8090", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwareModemType(&repo, item) == SSE_E_OK);
      moat_value_free(item);

      // Hardware version
      ASSERT((item = moat_value_new_string("1.2.3", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwareModemHwVersion(&repo, item) == SSE_E_OK);
      moat_value_free(item);

      // Frimware version
      ASSERT((item = moat_value_new_string("4.5.6", 0, sse_true)) != NULL);
      ASSERT(TDEVINFORepository_SetHardwareModemFwVersion(&repo, item) == SSE_E_OK);
      moat_value_free(item);
    }
    { // Network
      { // Interface

	MoatValue* if_name;
	MoatValue* if_macaddr;
	MoatValue* if_ipv4addr;
	MoatValue* if_netmask;
	MoatValue* if_ipv6addr;

	// eth0
	ASSERT((if_name     = moat_value_new_string("eth0", 0, sse_true)));
	ASSERT((if_macaddr  = moat_value_new_string("00:11:22:aa:bb:cc", 0, sse_true)));
	ASSERT((if_ipv4addr = moat_value_new_string("192.128.1.32", 0, sse_true)));
	ASSERT((if_netmask  = moat_value_new_string("255.255.255.0", 0, sse_true)));
	ASSERT((if_ipv6addr = moat_value_new_string("fe80::a00:27ff:fea9:d684/64", 0, sse_true)));
	ASSERT(TDEVINFORepository_AddHardwareNetworkInterface(&repo, if_name, if_macaddr, if_ipv4addr, if_netmask, if_ipv6addr) == SSE_E_OK);
	moat_value_free(if_name);
	moat_value_free(if_macaddr);
	moat_value_free(if_ipv4addr);
	moat_value_free(if_netmask);
	moat_value_free(if_ipv6addr);

	// wlan0
	ASSERT((if_name     = moat_value_new_string("wlan0", 0, sse_true)));
	ASSERT((if_macaddr  = moat_value_new_string("88:1f:a1:10:fd:c8", 0, sse_true)));
	ASSERT((if_ipv4addr = moat_value_new_string("192.168.1.11", 0, sse_true)));
	ASSERT((if_netmask  = moat_value_new_string("255.255.255.0", 0, sse_true)));
	// Some interface might not have IPv6 address.
	ASSERT(TDEVINFORepository_AddHardwareNetworkInterface(&repo, if_name, if_macaddr, if_ipv4addr, if_netmask, NULL) == SSE_E_OK);
	moat_value_free(if_name);
	moat_value_free(if_macaddr);
	moat_value_free(if_ipv4addr);
	moat_value_free(if_netmask);

	// usb0
	ASSERT((if_name     = moat_value_new_string("usb0", 0, sse_true)));
	ASSERT((if_macaddr  = moat_value_new_string("00:11:22:aa:bb:cc", 0, sse_true)));
	ASSERT((if_ipv6addr = moat_value_new_string("fe80::a00:27ff:fea9:d684/64", 0, sse_true)));
	// Some interface might not have IPv4 address.
	ASSERT(TDEVINFORepository_AddHardwareNetworkInterface(&repo, if_name, if_macaddr, NULL, NULL, if_ipv6addr) == SSE_E_OK);
	moat_value_free(if_name);
	moat_value_free(if_macaddr);
	moat_value_free(if_ipv6addr);
      }
      { // Nameserver
	// Server 1
	ASSERT((item = moat_value_new_string("222.146.35.1", 0, sse_true)) != NULL);
	ASSERT(TDEVINFORepository_AddHardwareNetworkNameserver(&repo, item) == SSE_E_OK);
	moat_value_free(item);

	// Server 2
	ASSERT((item = moat_value_new_string("221.184.25.1", 0, sse_true)) != NULL);
	ASSERT(TDEVINFORepository_AddHardwareNetworkNameserver(&repo, item) == SSE_E_OK);
	moat_value_free(item);
      }
    }
    { // SIM
      MoatValue* iccid;
      MoatValue* imsi;
      MoatValue* msisdn;

      // SIM 1
      ASSERT((iccid = moat_value_new_string("898110011111111111111", 0, sse_true)));
      ASSERT((imsi = moat_value_new_string("121234561234560", 0, sse_true)));
      ASSERT((msisdn = moat_value_new_string("09012345678", 0, sse_true)));
      ASSERT(TDEVINFORepository_AddHardwareSim(&repo, iccid, imsi, msisdn) == SSE_E_OK);
      moat_value_free(iccid);
      moat_value_free(imsi);
      moat_value_free(msisdn);

      // SIM 2
      ASSERT((msisdn = moat_value_new_string("08056781234", 0, sse_true)));
      ASSERT(TDEVINFORepository_AddHardwareSim(&repo, NULL, NULL, msisdn) == SSE_E_OK);
      moat_value_free(msisdn);
    }
    { // Software
      { // OS
	MoatValue* type;
	MoatValue* version;

	ASSERT((type = moat_value_new_string("Linux", 0, sse_true)) != NULL);
	ASSERT((version = moat_value_new_string("2.6.26", 0, sse_true)) != NULL);
	ASSERT(TDEVINFORepository_SetSoftwareOS(&repo, type, version) == SSE_E_OK);
	moat_value_free(type);
	moat_value_free(version);
      }
      { // ServiceSync Client
	MoatValue* type;
	MoatValue* version;
	MoatValue* sdk_version;

	ASSERT((type = moat_value_new_string("SSEGW", 0, sse_true)) != NULL);
	ASSERT((version = moat_value_new_string("1.0.7", 0, sse_true)) != NULL);
	ASSERT((sdk_version = moat_value_new_string("1.0.5", 0, sse_true)) != NULL);
	ASSERT(TDEVINFORepository_SetSoftwareSscl(&repo, type, version, sdk_version) == SSE_E_OK);
	moat_value_free(type);
	moat_value_free(version);
	moat_value_free(sdk_version);
      }
    }
  }

  // Get devinfo with JSON
  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Actual = [%s]", json_cstr);

  // Cleanup
  TDEVINFORepository_Finalize(&repo);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);

  return TEST_RESULT_UNKNOWN;
}

static void
test_devinfo_manager_callback(sse_int in_err, sse_pointer in_user_data)
{
  TDEVINFOManager *self;
  SSEString *devinfo;
  sse_char *p;

  LOG_PRINT("err = [%s]", sse_get_error_string(in_err));

  self = (TDEVINFOManager*)in_user_data;
  ASSERT(self);

  ASSERT(TDEVINFOManager_GetDevinfo(self, &devinfo) == SSE_E_OK);
  ASSERT((p = sse_strndup(sse_string_get_cstr(devinfo), sse_string_get_length(devinfo))));
  LOG_PRINT("Devinfo = [ %s ]", p);
}


static sse_int
test_devinfo_manager(Moat in_moat, TDEVINFOManager *in_manager)
{
  TDEVINFOManager_Initialize(in_manager, in_moat);
  TDEVINFOManager_Collect(in_manager,
			  test_devinfo_manager_callback,
			  in_manager);
  return TEST_RESULT_UNKNOWN;
}

static void
test_report(void)
{
  LOG_PRINT("Result : success = %d / unknown = %d / failure = %d", g_success, g_unknown,  g_failure);
}

sse_int
moat_app_main(sse_int in_argc, sse_char *argv[])
{
	Moat moat = NULL;
	sse_int err = SSE_E_OK;
	TDEVINFOManager manager;

	err = moat_init(argv[0], &moat);
	if (err != SSE_E_OK) {
		goto error_exit;
	}

	DO_TEST(test_devinfo_repository__load_devinfo_from_file(moat));
	DO_TEST(test_devinfo_repository__add_vendor(moat));
	DO_TEST(test_devinfo_repository__overwrite_vendor(moat));
	DO_TEST(test_devinfo_repository__add_interfaces(moat));
	DO_TEST(test_devinfo_repository__add_all_items(moat));
	DO_TEST(test_devinfo_manager(moat, &manager));

	err = moat_run(moat);
	if (err != SSE_E_OK) {
	  goto error_exit;
	}

	test_report();

	moat_destroy(moat);
	return SSE_E_OK;

error_exit:
	if (moat != NULL) {
		moat_destroy(moat);
	}
	return err;
}
