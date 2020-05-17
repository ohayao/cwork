#include <bridge/ble/ble_discover.h>
#include <regex.h>
#include <bridge/gattlib/gattlib.h>
#include <bridge/bridge_main/task.h>
#include <regex.h>
#include <bridge/ble/lock.h>
#include <bridge/bridge_main/lock_list.h>

int discoverLock(void *arg);

enum{
  DISCOVER_SM_TABLE_LEN = 1
};

fsm_table_t discover_fsm_table[DISCOVER_SM_TABLE_LEN] = {
  {BLE_DISCOVER_BEGIN, discoverLock,  BLE_DISCOVER_DONE}
};

int discoverLock(void *arg)
{
  task_node_t *task_node = (task_node_t *)arg;
  ble_data_t * ble_data = (ble_data_t *)task_node->ble_data;
  ble_discover_param_t *param = (ble_discover_param_t *)ble_data->ble_param;

  // 先默认
  char *addr_name = NULL;
  void *adapter = NULL;
  int ret;
  ret = gattlib_adapter_open(addr_name, &adapter);
  if (ret) {
		serverLog(LL_ERROR, "discoverLock ERROR: Failed to open adapter.\n");
    // TODO: our own error;
		return ret;
	}

  ret = gattlib_adapter_scan_enable(
		adapter, ble_discovered_device, param->scan_timeout, arg /* task node*/);
	if (ret) {
    serverLog(LL_NOTICE, "ERROR: Failed to scan.");
		goto EXIT;
	}
EXIT:
  gattlib_adapter_close(adapter);
  return 0;
}


fsm_table_t *getDiscoverFsmTable()
{
  return discover_fsm_table;
}

int getDiscoverFsmTableLen()
{
  return DISCOVER_SM_TABLE_LEN;
}

void ble_discovered_device(
  void *adapter, const char* addr, const char* name, void *user_data) {
  // 通过 user_data, 传入一个数据结构, 然后返回结果
	int ret;
	int addr_size;
	char *up_addr;
	int name_size;
  task_node_t *task_node = (task_node_t *)user_data;
  ble_data_t * ble_data = (ble_data_t *)(task_node->ble_data);
  ble_discover_param_t *param = (ble_discover_param_t *)(ble_data->ble_param);

	if (!name) {
    return ;
	} 
  // IGM303e31a5c
  regex_t regex;
  int reti;
  reti = regcomp(&regex, "^IGM", 0);
  // reti = regcomp(&regex, "^IGR", 0);
  if (reti) {
    serverLog(LL_ERROR, "Could not compile regex");
    return;
  }
  reti = regexec(&regex, name, 0, NULL, 0);
  if (!reti) {
    // match
    igm_lock_t nearby_lock;
    initLock(&nearby_lock);
    setLockName(&nearby_lock, name, strlen(name));
    setLockAddr(&nearby_lock, addr, strlen(addr));
    blePutResults(ble_data, &nearby_lock, sizeof(igm_lock_t));
  }
  else
  {
    return;
  }
  // 测试多个数据的返回
  // serverLog(LL_NOTICE, "match name %s add %s", name, addr);
  //   igm_lock_t nearby_lock;
  //   initLock(&nearby_lock);
  //   setLockName(&nearby_lock, name, strlen(name));
  //   setLockAddr(&nearby_lock, addr, strlen(addr));
  //   blePutResults(ble_data, &nearby_lock, sizeof(igm_lock_t));
  // printf("Discovered %s, don't have name, return\n", addr);
	return;
}

void continueDiscoveredDevice(
  void *adapter, const char* addr, const char* name, void *user_data);

int contnueDiscoverLock()
{
  char *addr_name = NULL;
  void *adapter = NULL;
  int ret;
  ret = gattlib_adapter_open(addr_name, &adapter);
  if (ret) {
		serverLog(LL_ERROR, "discoverLock ERROR: Failed to open adapter.\n");
    // TODO: our own error;
		return ret;
	}

  ret = gattlib_adapter_scan_enable(
		adapter, continueDiscoveredDevice, 2, NULL /* task node*/);
	if (ret) {
    serverLog(LL_ERROR, "ERROR: Failed to scan.");
		goto CONTINUE_DISCOVER_EXIT;
	}
CONTINUE_DISCOVER_EXIT:
  gattlib_adapter_close(adapter);
  return 0;
}

void continueDiscoveredDevice(
  void *adapter, const char* addr, const char* name, void *user_data) {
  // 通过 user_data, 传入一个数据结构, 然后返回结果
	int ret;
	int addr_size;
	char *up_addr;
	int name_size;
  serverLog(LL_NOTICE, "123 continueDiscoveredDevice");

	if (!name) {
    return ;
	} 
  // IGM303e31a5c
  regex_t regex;
  int reti;
  reti = regcomp(&regex, "^IGM", 0);
  // reti = regcomp(&regex, "^IGR", 0);
  if (reti) {
    serverLog(LL_ERROR, "Could not compile regex");
    return;
  }
  reti = regexec(&regex, name, 0, NULL, 0);
  if (!reti) {
    // match
    igm_lock_t *lock = findLockByName(name);
    if (!lock)
    {
      serverLog(LL_NOTICE, "finde new lock name %s addr %s and insert", name, addr);
      igm_lock_t nearby_lock;
      initLock(&nearby_lock);
      setLockName(&nearby_lock, name, strlen(name));
      setLockAddr(&nearby_lock, addr, strlen(addr));
      insertLock(&nearby_lock);
    }
    
  }
  else
  {
    return;
  }
  // 测试多个数据的返回
  // serverLog(LL_NOTICE, "match name %s add %s", name, addr);
  //   igm_lock_t nearby_lock;
  //   initLock(&nearby_lock);
  //   setLockName(&nearby_lock, name, strlen(name));
  //   setLockAddr(&nearby_lock, addr, strlen(addr));
  //   blePutResults(ble_data, &nearby_lock, sizeof(igm_lock_t));
  // printf("Discovered %s, don't have name, return\n", addr);
	return;
}