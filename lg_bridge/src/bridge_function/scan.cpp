#include "scan.h"
using namespace std;

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// 线程连接函数, 用于对发现的时被进行连接
// 用于对搜索结果进行连接


// 这儿是一个回调函数, 当发现设备的时候, 不断的被回调
// 运用一个线程,判断名字符合规则, 去连接设备,
//  
static void ble_discovered_device(
  void *adapter, const char* addr, const char* name, void *user_data) {
  struct connection_t *connection;
	int ret;

  if (name) {
    regex IGM_regex("^IGM.*?",  std::regex_constants::icase);
    if (regex_match(name, IGM_regex))
    {
      printf("result Discovered %s - '%s'\n", addr, name);
    }
    else
    {
      return;
    }
	} else {
		// printf("Discovered %s, don't have name, return\n", addr);
		return;
  }
  return;
}

// char* adapter_name;
// void* adapter;
int start_scan(
  void* adapter, const int &scan_timeout)
{
  int ret;
  // 访问
  ret = gattlib_adapter_scan_enable(
    adapter, ble_discovered_device, scan_timeout, NULL /* user_data */);
  if (ret) {
		cerr <<  "ERROR: Failed to scan." << endl;
		goto EXIT;
	}

EXIT:
	return ret;
}

int stop_scan(void* adapter)
{
  int ret = gattlib_adapter_scan_disable(adapter);
  return ret;
}

//


