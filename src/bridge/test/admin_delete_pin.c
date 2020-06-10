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
            printf( "saving ble TASK_BLE_ADMIN_UNLOCK data.\n");
            ble_admin_result_t *result = (ble_admin_result_t *)ble_data->ble_result;
            int error = result->delete_pin_request_result;
            IgDeletePinResponse *response = result->cmd_response;
            
            if (error)
            {
              printf("delete pin request error.\n");
            }
            else
            {
                printf( "delete pin request success.\n");
                if (response->has_operation_id)
                    printf( "operation ID: [%d].\n", response->operation_id);
                if (response->has_result)
                    printf( "result: [%d].\n", response->result);
            }
            break;
        }
        default:
            break;
        }
    }
}


int testDeletePin(igm_lock_t *lock, IgDeletePinRequest *request) {
    printf("delete pin request cmd ask invoker to release the lock.\n");
      
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
            printf("[%d] step error.\n", j);
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
        printf("lock error.\n");
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
    printf( "lock end-------.\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
      printf( "%s <device_address> <admin_key> <passwd> <pin>.\n", argv[0]);
      return 1;
    }
    printf("test ble delete pin request to start.\n");
    
    printf("select the lock you want to unlock.\n");
    igm_lock_t *lock=NULL;
    getLock(&lock);
    initLock(lock);
    setLockAddr(lock, argv[1], strlen(argv[1]));
    printf( "setLockAddr success.\n");

    uint8_t tmp_buff[100];
    memset(tmp_buff, 0, sizeof(tmp_buff));
    int admin_len = hexStrToByte(argv[2], tmp_buff, strlen(argv[2]));
    setLockAdminKey(lock, tmp_buff, admin_len);
    printf( "setLockAdminKey success.\n");

    memset(tmp_buff, 0, sizeof(tmp_buff));
    int password_size = hexStrToByte(argv[3], tmp_buff, strlen(argv[3]));
    setLockPassword(lock, tmp_buff, password_size);
    printf( "setLockPassword success,\n");

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
    int pin_size = strlen(argv[4]);
	for(int i=0;i<pin_size; i++) {
		tmp_buff[i] = argv[4][i];
	}
    printf("pin is:[");
	for(int k=0;k<pin_size;k++) {
		printf("%d", tmp_buff[k]);
	}
	printf("]\n");
    
    ig_DeletePinRequest_set_old_pin(
      &delete_pin_request, tmp_buff, pin_size);
    printf( "delete pin reques cmd test go.\n");
	ig_DeletePinRequest_set_operation_id(&delete_pin_request, 1);
    int res = testDeletePin(lock, &delete_pin_request);
    // ig_DeletePinRequest_deinit(&delete_pin_request);
    releaseLock(&lock);
    return 0;
}

