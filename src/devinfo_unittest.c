#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <servicesync/moat.h>
#include <devinfo/devinfo_repository.h>
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

  // Intialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  // Load devinfo from the file.
  ASSERT((path = sse_string_new("./devinfo.json")) != NULL);
  ASSERT(TDEVINFORepository_LoadDevinfo(&repo, path) == SSE_E_OK);

  // Cleanup
  TDEVINFORepository_Finalize(&repo);
  sse_string_free(path, sse_true);

  return TEST_RESULT_UNKNOWN;
}

static sse_int
test_devinfo_repository__get_all_with_json(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* path;
  SSEString* json = NULL;
  sse_char* json_cstr;

  // Initialize
  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  // Load devinfo from the file.
  ASSERT((path = sse_string_new("./devinfo.json")) != NULL);
  ASSERT(TDEVINFORepository_LoadDevinfo(&repo, path) == SSE_E_OK);

  // Seriallize with JSON
  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Devinfo = [%s]", json_cstr);

  // Cleanup
  TDEVINFORepository_Finalize(&repo);
  sse_string_free(path, sse_true);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);

  return TEST_RESULT_UNKNOWN;
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
	DO_TEST(test_devinfo_repository__get_all_with_json(moat));
	DO_TEST(test_devinfo_repository__add_vendor(moat));
	DO_TEST(test_devinfo_repository__overwrite_vendor(moat));
	DO_TEST(test_devinfo_repository__add_interfaces(moat));
	test_report();

	moat_destroy(moat);
	return SSE_E_OK;

error_exit:
	if (moat != NULL) {
		moat_destroy(moat);
	}
	return err;
}
