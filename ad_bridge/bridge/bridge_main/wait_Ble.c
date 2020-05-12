#include <bridge/bridge_main/wait_ble.h>
#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/log.h>
#include <unistd.h>
#include <bridge/ble/ble_discover.h>
int WaitBLE(void *arg){
    //Thread_start(wait_BLE, sysinfo)
        
    for(;;) {
      serverLog(LL_NOTICE, "waiting for discover lock...");
      contnueDiscoverLock();
      sleep(1);
    }
    return 0;
}