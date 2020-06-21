#include "bridge/gattlib/gattlib.h"
#include "bridge/wifi_service/SeWifiInfoRequest.h"
#include "bridge/bridge_main/log.h"
#include "bridge/ble/lock.h"

int main(int argc, char *argv[])
{
  int ret = 0; 
  int err = 0;
  serverLog(LL_NOTICE, "create igm_wifi_info_t pointer");
  SetWIFIInfoRequest *wifi_info_request = NULL;
  igm_lock_t *lock=NULL;
  char *adapter_name = NULL;
  void *adapter = NULL;
  gatt_connection_t* gatt_connection = NULL;

  if (argc != 4) {
    serverLog(LL_NOTICE, "%s <device_address> <wifi_ssid> <wifi_password>\n", argv[0]);
    return 1;
  }

  // 获得一个锁
  if(getLock(&lock))
  {
    serverLog(LL_ERROR, "getLock failed");
    return 1;
  }
  serverLog(LL_NOTICE, "getLock success");

  // 初始化地址
  if(initLock(lock))
  {
    serverLog(LL_ERROR, "initLock failed");
    return 1;
  }
  serverLog(LL_NOTICE, "initLock success");

  // 设置地址
  if (setLockAddr(lock, argv[1], strlen(argv[1])))
  {
    serverLog(LL_ERROR, "setLockAddr failed");
    return 1;
  }
  serverLog(LL_NOTICE, "setLockAddr success");


  // 获得一个请求
  if (getWifiInfoRequest(&wifi_info_request))
  {
    serverLog(LL_ERROR, "getWifiInfoRequest failed");
    return 1;
  }
  serverLog(LL_NOTICE, "getWifiInfoRequest success");

  // 获得连接
  ret = gattlib_adapter_open(adapter_name, &(adapter));
	if (ret) {
		serverLog(LL_ERROR, 
				"ERROR: register_admin_notfication Failed to open adapter.");
    return 1;
	}
  serverLog(LL_NOTICE, "gattlib_adapter_open success");

  serverLog(LL_NOTICE, "connect  %s", lock->addr);
  gatt_connection = gattlib_connect(
    adapter, lock->addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
  if (gatt_connection == NULL) {
		serverLog(LL_ERROR, "Fail to connect to the bluetooth device." );
		return 1;
	}
  serverLog(LL_NOTICE, "Succeeded to connect to the bluetooth device." );

}