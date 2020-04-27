#include <bridge/bridge_main/wait_ble.h>
#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/log.h>
#include <unistd.h>

int WaitBLE(void *arg){
    //Thread_start(wait_BLE, sysinfo)
    sysinfo_t *g_sysif = (sysinfo_t *)arg;
    int ble_task = g_sysif->ble_task;
    for(;;) {
        switch (ble_task)
        {
        case BLE_NONE_TASK:
        {
          /* code */
          serverLog(LL_DEBUG, "waiting for BLE...");
          break;
        }
        case BLE_NONE_TASK:
        {
          /* code */
          serverLog(LL_DEBUG, " BLE_DISCOVER_LOCK ...");
          
          break;
        }
        default:
        {
          serverLog(LL_DEBUG, "BLE task error, don't have this BLE_TASK");
          break;
        } // default
        } // swatch
        
        

        sleep(1);
    }
    return 0;
}