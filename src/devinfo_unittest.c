#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <servicesync/moat.h>
#include <devinfo/devinfo.h>
#include <sseutils.h>

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
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

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
  SSEString* vendor;
  SSEString* json = NULL;
  sse_char* json_cstr;
  sse_char expected[] = "{\"hardware\":{\"platform\":{\"vendor\":\"ABC Corp.\"}}}";

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  // Add hardware.platform.vendor into empty repository.
  ASSERT((vendor = sse_string_new("ABC Corp.")) != NULL);
  ASSERT(TDEVINFORepository_SetHardwarePlatformVendor(&repo, vendor) == SSE_E_OK);
  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Devinfo = [%s]", json_cstr);
  ASSERT(sse_strcmp(json_cstr, expected) == 0);

  // Cleanup
  TDEVINFORepository_Finalize(&repo);
  sse_string_free(vendor, sse_true);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);

  return TEST_RESULT_SUCCESS;
}

static sse_int
test_devinfo_repository__overwrite_vendor(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* path;
  SSEString* vendor;
  SSEString* json = NULL;
  sse_char* json_cstr;
  sse_char expected[] = "ABC Corp.";

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  ASSERT((path = sse_string_new("./devinfo.json")) != NULL);
  ASSERT(TDEVINFORepository_LoadDevinfo(&repo, path) == SSE_E_OK);

  ASSERT((vendor = sse_string_new("ABC Corp.")) != NULL);
  ASSERT(TDEVINFORepository_SetHardwarePlatformVendor(&repo, vendor) == SSE_E_OK);

  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Devinfo = [%s]", json_cstr);
  ASSERT(sse_strstr(json_cstr, expected));

  // Cleanup
  TDEVINFORepository_Finalize(&repo);
  sse_string_free(path, sse_true);
  sse_string_free(vendor, sse_true);
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

  SSEString* eth0_name;
  SSEString* eth0_macaddr;
  SSEString* eth0_ipv4addr;
  SSEString* eth0_netmask;
  SSEString* eth0_ipv6addr;

  SSEString* usb0_name;
  SSEString* usb0_macaddr;
  SSEString* usb0_ipv4addr;
  SSEString* usb0_netmask;
  SSEString* usb0_ipv6addr;

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  // Register intaface info
  ASSERT((eth0_name     = sse_string_new("eth0")));
  ASSERT((eth0_macaddr  = sse_string_new("00:11:22:aa:bb:cc")));
  ASSERT((eth0_ipv4addr = sse_string_new("192.128.1.32")));
  ASSERT((eth0_netmask  = sse_string_new("255.255.255.0")));
  ASSERT((eth0_ipv6addr = sse_string_new("fe80::a00:27ff:fea9:d684/64")));

  ASSERT((usb0_name     = sse_string_new("usb0")));
  ASSERT((usb0_macaddr  = sse_string_new("44:55:66:dd:ee:ff")));
  ASSERT((usb0_ipv4addr = sse_string_new("10.0.2.15")));
  ASSERT((usb0_netmask  = sse_string_new("255.255.254.0")));
  ASSERT((usb0_ipv6addr = sse_string_new("fe80::a00:27ff:fea9:d684/64")));

  ASSERT(TDEVINFORepository_AddHadwareNetworkInterface(&repo, eth0_name, eth0_macaddr, eth0_ipv4addr, eth0_netmask, eth0_ipv6addr) == SSE_E_OK);
  ASSERT(TDEVINFORepository_AddHadwareNetworkInterface(&repo, usb0_name, usb0_macaddr, usb0_ipv4addr, usb0_netmask, usb0_ipv6addr) == SSE_E_OK);

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

  sse_string_free(eth0_name, sse_true);
  sse_string_free(eth0_macaddr, sse_true);
  sse_string_free(eth0_ipv4addr, sse_true);
  sse_string_free(eth0_netmask, sse_true);
  sse_string_free(eth0_ipv6addr, sse_true);

  sse_string_free(usb0_name, sse_true);
  sse_string_free(usb0_macaddr, sse_true);
  sse_string_free(usb0_ipv4addr, sse_true);
  sse_string_free(usb0_netmask, sse_true);
  sse_string_free(usb0_ipv6addr, sse_true);

  return TEST_RESULT_SUCCESS;
}

static sse_int
test_devinfo_repository__add_all_items(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* item;
  SSEString* json = NULL;
  sse_char* json_cstr;

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  // Add all items
  { // Hardware
    { // Platform
      // Vendor
      ASSERT((item = sse_string_new("Raspberry Pi Fundation")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformVendor(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);

      // Product
      ASSERT((item = sse_string_new("Raspberry Pi")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformProduct(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);

      // Model
      ASSERT((item = sse_string_new("RASPBERRY PI 1 MODEL B+")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformModel(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);

      // Serial No.
      ASSERT((item = sse_string_new("ABC12345")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformSerial(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);

      // Hardware version
      ASSERT((item = sse_string_new("1.2.3")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformHwVersion(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);

      // Firmware versionn
      ASSERT((item = sse_string_new("2.4.5")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformFwVersion(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);

      // Device Id
      ASSERT((item = sse_string_new("RSP:SN:123456")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformDeviceId(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);

      // Category
      ASSERT((item = sse_string_new("gateway")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwarePlatformCategory(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);
    }
    { // Modem
      // Type
      ASSERT((item = sse_string_new("MC8090")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwareModemType(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);

      // Hardware version
      ASSERT((item = sse_string_new("1.2.3")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwareModemHwVersion(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);

      // Frimware version
      ASSERT((item = sse_string_new("4.5.6")) != NULL);
      ASSERT(TDEVINFORepository_SetHardwareModemFwVersion(&repo, item) == SSE_E_OK);
      sse_string_free(item, sse_true);
    }
    { // Network
      { // Interface

	SSEString* if_name;
	SSEString* if_macaddr;
	SSEString* if_ipv4addr;
	SSEString* if_netmask;
	SSEString* if_ipv6addr;

	// eth0
	ASSERT((if_name     = sse_string_new("eth0")));
	ASSERT((if_macaddr  = sse_string_new("00:11:22:aa:bb:cc")));
	ASSERT((if_ipv4addr = sse_string_new("192.128.1.32")));
	ASSERT((if_netmask  = sse_string_new("255.255.255.0")));
	ASSERT((if_ipv6addr = sse_string_new("fe80::a00:27ff:fea9:d684/64")));
	ASSERT(TDEVINFORepository_AddHadwareNetworkInterface(&repo, if_name, if_macaddr, if_ipv4addr, if_netmask, if_ipv6addr) == SSE_E_OK);
	sse_string_free(if_name, sse_true);
	sse_string_free(if_macaddr, sse_true);
	sse_string_free(if_ipv4addr, sse_true);
	sse_string_free(if_netmask, sse_true);
	sse_string_free(if_ipv6addr, sse_true);

	// wlan0
	ASSERT((if_name     = sse_string_new("wlan0")));
	ASSERT((if_macaddr  = sse_string_new("88:1f:a1:10:fd:c8")));
	ASSERT((if_ipv4addr = sse_string_new("192.168.1.11")));
	ASSERT((if_netmask  = sse_string_new("255.255.255.0")));
	// Some interface might not have IPv6 address.
	ASSERT(TDEVINFORepository_AddHadwareNetworkInterface(&repo, if_name, if_macaddr, if_ipv4addr, if_netmask, NULL) == SSE_E_OK);
	sse_string_free(if_name, sse_true);
	sse_string_free(if_macaddr, sse_true);
	sse_string_free(if_ipv4addr, sse_true);
	sse_string_free(if_netmask, sse_true);

	// usb0
	ASSERT((if_name     = sse_string_new("usb0")));
	ASSERT((if_macaddr  = sse_string_new("00:11:22:aa:bb:cc")));
	ASSERT((if_ipv6addr = sse_string_new("fe80::a00:27ff:fea9:d684/64")));
	// Some interface might not have IPv4 address.
	ASSERT(TDEVINFORepository_AddHadwareNetworkInterface(&repo, if_name, if_macaddr, NULL, NULL, if_ipv6addr) == SSE_E_OK);
	sse_string_free(if_name, sse_true);
	sse_string_free(if_macaddr, sse_true);
	sse_string_free(if_ipv6addr, sse_true);
      }
      { // Nameserver
	// Server 1
	ASSERT((item = sse_string_new("222.146.35.1")) != NULL);
	ASSERT(TDEVINFORepository_AddHardwareNetworkNameserver(&repo, item) == SSE_E_OK);
	sse_string_free(item, sse_true);

	// Server 2
	ASSERT((item = sse_string_new("221.184.25.1")) != NULL);
	ASSERT(TDEVINFORepository_AddHardwareNetworkNameserver(&repo, item) == SSE_E_OK);
	sse_string_free(item, sse_true);
      }
    }
    { // SIM
      SSEString* iccid;
      SSEString* imsi;
      SSEString* msisdn;

      // SIM 1
      ASSERT((iccid = sse_string_new("898110011111111111111")));
      ASSERT((imsi = sse_string_new("121234561234560")));
      ASSERT((msisdn = sse_string_new("09012345678")));
      ASSERT(TDEVINFORepository_AddHardwareSim(&repo, iccid, imsi, msisdn) == SSE_E_OK);
      sse_string_free(iccid, sse_true);
      sse_string_free(imsi, sse_true);
      sse_string_free(msisdn, sse_true);

      // SIM 2
      ASSERT((msisdn = sse_string_new("08056781234")));
      ASSERT(TDEVINFORepository_AddHardwareSim(&repo, NULL, NULL, msisdn) == SSE_E_OK);
      sse_string_free(msisdn, sse_true);
    }
    { // Software
      { // OS
	SSEString* type;
	SSEString* version;

	ASSERT((type = sse_string_new("Linux")) != NULL);
	ASSERT((version = sse_string_new("2.6.26")) != NULL);
	ASSERT(TDEVINFORepository_SetSoftwareOS(&repo, type, version) == SSE_E_OK);
	sse_string_free(type, sse_true);
	sse_string_free(version, sse_true);
      }
      { // ServiceSync Client
	SSEString* type;
	SSEString* version;
	SSEString* sdk_version;

	ASSERT((type = sse_string_new("SSEGW")) != NULL);
	ASSERT((version = sse_string_new("1.0.7")) != NULL);
	ASSERT((sdk_version = sse_string_new("1.0.5")) != NULL);
	ASSERT(TDEVINFORepository_SetSoftwareSscl(&repo, type, version, sdk_version) == SSE_E_OK);
	sse_string_free(type, sse_true);
	sse_string_free(version, sse_true);
	sse_string_free(sdk_version, sse_true);
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

void
test_devinfo_collector_get_hardware_vendor_callback(MoatObject* in_collected, sse_pointer in_user_data, sse_int in_error_code)
{
  MoatObject *object = (MoatObject *)in_collected;
  sse_char *vendor;
  sse_uint vendor_len;
  sse_char* expected = "Unknown";

  ASSERT(in_collected);
  ASSERT(in_error_code == SSE_E_OK);

  ASSERT(moat_object_get_string_value(object, DEVINFO_KEY_VENDOR, &vendor, &vendor_len) == SSE_E_OK);
  ASSERT(sse_strlen(expected) == vendor_len);
  ASSERT(sse_memcmp(expected, vendor, vendor_len) == 0);

  LOG_PRINT("%s ... Success\n", __FUNCTION__);
}

static sse_int
test_devinfo_collector_get_hardware_vendor(Moat in_moat)
{
  TDEVINFOCollector collector;
  ASSERT(TDEVINFOCollector_Initialize(&collector, in_moat) == SSE_E_OK);
  ASSERT(TDEVINFOCollector_GetHardwarePlatformVendor(&collector, test_devinfo_collector_get_hardware_vendor_callback, NULL) == SSE_E_OK);

  return TEST_RESULT_UNKNOWN;
}

void
test_devinfo_collector_get_hardware_network_interface_callback(MoatObject* in_collected, sse_pointer in_user_data, sse_int in_error_code)
{
  MoatObject *object = (MoatObject *)in_collected;
  SSESList *list;

  ASSERT(in_collected);
  ASSERT(in_error_code == SSE_E_OK);

  ASSERT(moat_object_get_list_value(object, DEVINFO_KEY_NET_INTERFACE, &list) == SSE_E_OK);
  LOG_PRINT("List len = [%d]", sse_slist_length(list));
  while (list) {
    MoatObject* interface = sse_slist_data(list);
    SseUtilMoatObjectDump(interface);
    list = sse_slist_next(list);
  }
}

static sse_int
test_devinfo_collector_get_hardware_network_interface(Moat in_moat)
{
  TDEVINFOCollector collector;
  ASSERT(TDEVINFOCollector_Initialize(&collector, in_moat) == SSE_E_OK);
  ASSERT(TDEVINFOCollector_GetHadwareNetworkInterface(&collector, test_devinfo_collector_get_hardware_network_interface_callback, NULL) == SSE_E_OK);

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

	err = moat_init(argv[0], &moat);
	if (err != SSE_E_OK) {
		goto error_exit;
	}

	DO_TEST(test_devinfo_repository__load_devinfo_from_file(moat));
	DO_TEST(test_devinfo_repository__add_vendor(moat));
	DO_TEST(test_devinfo_repository__overwrite_vendor(moat));
	DO_TEST(test_devinfo_repository__add_interfaces(moat));
	DO_TEST(test_devinfo_repository__add_all_items(moat));
	DO_TEST(test_devinfo_collector_get_hardware_vendor(moat));
	DO_TEST(test_devinfo_collector_get_hardware_network_interface(moat));

	test_report();

	moat_destroy(moat);
	return SSE_E_OK;

error_exit:
	if (moat != NULL) {
		moat_destroy(moat);
	}
	return err;
}
