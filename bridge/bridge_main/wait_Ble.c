#include <bridge/bridge_main/wait_ble.h>
#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/log.h>
#include <unistd.h>

int WaitBLE(void *arg){
    //Thread_start(wait_BLE, sysinfo)
    for(;;) {
      serverLog(LL_DEBUG, "waiting for BLE...");
      sleep(1);
    }
    return 0;
}