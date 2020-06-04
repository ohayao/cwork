#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <syslog.h>
#include <stdlib.h>
#include <ctype.h>
#include "bridge/bridge_main/ign_constants.h"
#include "bridge/bridge_main/log.h"
#include "bridge/bridge_main/sysinfo.h"
#include "bridge/bridge_main/ign.h"
#include "bridge/bridge_main/task.h"
#include "bridge/bridge_main/thread_helper.h"
#include "bridge/bridge_main/mutex_helper.h"
#include "bridge/bridge_main/task_queue.h"
#include "bridge/bridge_main/wait_ble.h"
#include "bridge/ble/ble_discover.h"
#include "bridge/ble/lock.h"
#include "bridge/ble/ble_pairing.h"
#include "bridge/bridge_main/lock_list.h"
#include "bridge/ble/ble_admin.h"
#include "bridge/ble/ble_pairing.h"
#include "bridge/lock/messages/DeletePinRequest.h"
#include "bridge/lock/messages/DeletePinResponse.h"


void saveTaskData(task_node_t *ptn)
{
    if (!ptn) return;

    if(ptn->ble_data && ptn->ble_data_len)
    {
        ble_data_t *ble_data = ptn->ble_data;
        int task_type = ptn->task_type;
        switch (task_type)
        {
        case TASK_BLE_ADMIN_CREATE_PIN_REQUEST:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_ADMIN_UNLOCK data");
            ble_admin_result_t *result = (ble_admin_result_t *)ble_data->ble_result;
            int error = result->delete_pin_request_result;
            IgDeletePinResponse *response = result->cmd_response;
            
            if (error)
            {
              serverLog(LL_ERROR, "delete pin request error");
            }
            else
            {
                serverLog(LL_NOTICE, "delete pin request success");
                if (response->has_operation_id)
                    serverLog(LL_NOTICE, "operation ID: %d", response->operation_id);
                if (response->has_result)
                    serverLog(LL_NOTICE, "result: %d", response->result);
            }
            break;
        }
        default:
            break;
        }
    }
}

int hexStrToByte(const char* source, uint8_t* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;
    
    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);
        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;
 
        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;
 
        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return sourceLen /2 ;
}


int testDeletePin(igm_lock_t *lock, IgDeletePinRequest *request) {
    serverLog(LL_NOTICE,"delete pin request cmd ask invoker to release the lock.");
      
    ble_admin_param_t *admin_param = (ble_admin_param_t *)malloc(sizeof(ble_admin_param_t));
    bleInitAdminParam(admin_param);
    bleSetAdminParam(admin_param, lock);
    bleSetAdminRequest(admin_param, request, sizeof(IgDeletePinRequest));

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    fsm_table_t *delete_pin_fsm = getAdminDeletePinRequestFsmTable();
    int fsm_max_n = getAdminDeletePinRequestFsmTableLen();
    int current_state = BLE_ADMIN_BEGIN;
    int error = 0;

    task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
    tn->ble_data = ble_data;

    tn->sm_table_len = fsm_max_n;
    tn->task_sm_table = delete_pin_fsm;

    tn->task_type = TASK_BLE_ADMIN_CREATE_PIN_REQUEST;

    for (int j = 0; j < fsm_max_n; j++)
    {
      if (current_state == tn->task_sm_table[j].cur_state) {
          // 增加一个判断当前函数, 是否当前函数出错. 0 表示没问题
        int event_result = tn->task_sm_table[j].eventActFun(tn);
        if (event_result)
        {
            serverLog(LL_ERROR, "%d step error", j);
            error = 1;
            break;
        }
        else
        {
            current_state = tn->task_sm_table[j].next_state;
        }
		}
    }
    if (error)
    {
        serverLog(LL_ERROR, "lock error");
        return error;
    }

    saveTaskData(tn);
    ble_admin_result_t *result = (ble_admin_result_t *)ble_data->ble_result;
    ig_DeletePinResponse_deinit(result->cmd_response);
    free(result->cmd_response);
    result->cmd_response = NULL;
    bleReleaseBleResult(ble_data);
    free(ble_data);
    ble_data = NULL;
    free(tn);
    tn = NULL;
    free(admin_param);
    admin_param = NULL;
    serverLog(LL_NOTICE, "lock end-------");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
      serverLog(LL_NOTICE, "%s <device_address> <admin_key> <passwd> <pin> \n", argv[0]);
      return 1;
    }
    serverLog(LL_NOTICE,"test ble delete pin request to start.");
    
    serverLog(LL_NOTICE,"select the lock you want to unlock.");
    igm_lock_t *lock=NULL;
    getLock(&lock);
    initLock(lock);
    setLockAddr(lock, argv[1], strlen(argv[1]));
    serverLog(LL_NOTICE, "setLockAddr success");

    uint8_t tmp_buff[100];
    memset(tmp_buff, 0, sizeof(tmp_buff));
    int admin_len = hexStrToByte(argv[2], tmp_buff, strlen(argv[2]));
    setLockAdminKey(lock, tmp_buff, admin_len);
    serverLog(LL_NOTICE, "setLockAdminKey success");

    memset(tmp_buff, 0, sizeof(tmp_buff));
    int password_size = hexStrToByte(argv[3], tmp_buff, strlen(argv[3]));
    setLockPassword(lock, tmp_buff, password_size);
    serverLog(LL_NOTICE, "setLockPassword success");

    // 创建一个变量
    IgDeletePinRequest delete_pin_request;
    // 初始化这个变量
    ig_DeletePinRequest_init(&delete_pin_request);
    // 设置 password
    memset(tmp_buff, 0, sizeof(tmp_buff));
    password_size = hexStrToByte(argv[3], tmp_buff, strlen(argv[3]));
    ig_DeletePinRequest_set_password(
      &delete_pin_request, tmp_buff, password_size);

    // 设置旧的password
    memset(tmp_buff, 0, sizeof(tmp_buff));
    int pin_size = hexStrToByte(
      argv[4], tmp_buff, strlen(argv[4]));
    ig_DeletePinRequest_set_old_pin(
      &delete_pin_request, tmp_buff, pin_size);
    serverLog(LL_NOTICE, "delete pin reques cmd test go");
    int res = testDeletePin(lock, &delete_pin_request);
    // ig_DeletePinRequest_deinit(&delete_pin_request);
    releaseLock(&lock);
    return 0;
}

