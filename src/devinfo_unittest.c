#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <servicesync/moat.h>
#include <devinfo/devinfo_repository.h>
#include <sseutils.h>

jmp_buf g_env;
sse_int g_success = 0;
sse_int g_failure = 0;

#define LOG_PRINT(format, ...) printf("## TEST ## %s L%d: " format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DO_TEST(func) {				\
  sse_int ret;					\
  LOG_PRINT("Test case: " #func);		\
  if ((ret = setjmp(g_env)) == 0) {		\
    func;					\
    LOG_PRINT(#func "... Success\n");		\
    g_success++;				\
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

static void
test_devinfo_repository__load_devinfo_from_file(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* path;

  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  ASSERT((path = sse_string_new("./devinfo.json")) != NULL);
  ASSERT(TDEVINFORepository_LoadDevinfo(&repo, path) == SSE_E_OK);
  SseUtilMoatObjectDump(repo.fDevinfo);

  TDEVINFORepository_Finalize(&repo);
  sse_string_free(path, sse_true);
}

static void
test_devinfo_repository__get_all_with_json(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* path;
  SSEString* json = NULL;
  sse_char* json_cstr;

  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  ASSERT((path = sse_string_new("./devinfo.json")) != NULL);
  ASSERT(TDEVINFORepository_LoadDevinfo(&repo, path) == SSE_E_OK);

  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Devinfo = [%s]", json_cstr);

  TDEVINFORepository_Finalize(&repo);
  sse_string_free(path, sse_true);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);
}

static void
test_devinfo_repository__add_vendor(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* vendor;
  SSEString* json = NULL;
  sse_char* json_cstr;

  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  ASSERT((vendor = sse_string_new("ABC Corp.")) != NULL);
  ASSERT(TDEVINFORepository_SetHardwarePlatformVendor(&repo, vendor) == SSE_E_OK);
  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Devinfo = [%s]", json_cstr);

  TDEVINFORepository_Finalize(&repo);
  sse_string_free(vendor, sse_true);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);
}

static void
test_devinfo_repository__overwrite_vendor(Moat in_moat)
{
  TDEVINFORepository repo;
  SSEString* path;
  SSEString* vendor;
  SSEString* json = NULL;
  sse_char* json_cstr;

  ASSERT(TDEVINFORepository_Initialize(&repo, in_moat) == SSE_E_OK);

  ASSERT((path = sse_string_new("./devinfo.json")) != NULL);
  ASSERT(TDEVINFORepository_LoadDevinfo(&repo, path) == SSE_E_OK);

  ASSERT((vendor = sse_string_new("ABC Corp.")) != NULL);
  ASSERT(TDEVINFORepository_SetHardwarePlatformVendor(&repo, vendor) == SSE_E_OK);
  ASSERT(TDEVINFORepository_GetDevinfoWithJson(&repo, NULL, &json) == SSE_E_OK);
  ASSERT((json_cstr = sse_strndup(sse_string_get_cstr(json), sse_string_get_length(json))) != NULL);
  LOG_PRINT("Devinfo = [%s]", json_cstr);

  TDEVINFORepository_Finalize(&repo);
  sse_string_free(path, sse_true);
  sse_string_free(vendor, sse_true);
  sse_string_free(json, sse_true);
  sse_free(json_cstr);
}

static void
test_report(void)
{
  LOG_PRINT("Result : %d / %d", g_success, g_success + g_failure);
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
	test_report();
#if 0
	/* setup event handlers, timers, etc */
	/* register models */

	err = moat_run(moat);
	if (err != SSE_E_OK) {
		goto error_exit;
	}

	/* unregister models */
	/* teardown */
#endif
	moat_destroy(moat);
	return SSE_E_OK;

error_exit:
	if (moat != NULL) {
		moat_destroy(moat);
	}
	return err;
}
