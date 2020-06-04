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
#include "bridge/lock/messages/CreatePinRequest.h"
#include "bridge/lock/messages/CreatePinResponse.h"


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
            printf( "saving ble TASK_BLE_ADMIN_UNLOCK data");
            ble_admin_result_t *create_pin_result = (ble_admin_result_t *)ble_data->ble_result;
            int create_pin_request_error = create_pin_result->create_pin_request_result;
            IgCreatePinResponse *create_pin_response = create_pin_result->cmd_response;
            
            if (create_pin_request_error)
            {
                printf( "create pin request error");
            }
            else
            {
                printf( "create pin request success");
                if (create_pin_response->has_operation_id)
                    printf( "operation ID: %d", create_pin_response->operation_id);
                if (create_pin_response->has_result)
                    printf( "result: %d", create_pin_response->result);
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


int testCreatePin(igm_lock_t *lock, IgCreatePinRequest *request) {
    printf("create pin request cmd ask invoker to release the lock.");
      
    ble_admin_param_t *admin_param = (ble_admin_param_t *)malloc(sizeof(ble_admin_param_t));
    bleInitAdminParam(admin_param);
    bleSetAdminParam(admin_param, lock);
    bleSetAdminRequest(admin_param, request, sizeof(IgCreatePinRequest));

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    fsm_table_t *create_pin_fsm = getAdminCreatePinRequestFsmTable();
    int fsm_max_n = getAdminCreatePinRequestFsmTableLen();
    int current_state = BLE_ADMIN_BEGIN;
    int error = 0;

    task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
    tn->ble_data = ble_data;

    tn->sm_table_len = fsm_max_n;
    tn->task_sm_table = create_pin_fsm;

    tn->task_type = TASK_BLE_ADMIN_CREATE_PIN_REQUEST;

    for (int j = 0; j < fsm_max_n; j++)
    {
        if (current_state == tn->task_sm_table[j].cur_state) {
            // 增加一个判断当前函数, 是否当前函数出错. 0 表示没问题
			int event_result = tn->task_sm_table[j].eventActFun(tn);
            if (event_result)
            {
                printf("%d step error", j);
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
        printf("lock error");
        return error;
    }

    saveTaskData(tn);
    ble_admin_result_t *result = (ble_admin_result_t *)ble_data->ble_result;
    ig_CreatePinResponse_deinit(result->cmd_response);
    free(result->cmd_response);
    result->cmd_response = NULL;
    bleReleaseBleResult(ble_data);
    free(ble_data);
    ble_data = NULL;
    free(tn);
    tn = NULL;
    // 一定要等到现在才能释放, 因为里面的内容, 是会
    // 被复制, 然后使用
    free(admin_param);
    admin_param = NULL;
    printf( "lock end-------");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
      printf("%s <device_address> <admin_key> <passwd> <pin> \n", argv[0]);
      return 1;
    }
    printf("test ble create pin request ing to start.");
    
    printf("select the lock you want to unlock.");
    igm_lock_t *lock=NULL;
    getLock(&lock);
    initLock(lock);
    setLockAddr(lock, argv[1], strlen(argv[1]));
    printf( "setLockAddr success");

    uint8_t tmp_buff[100];
    memset(tmp_buff, 0, sizeof(tmp_buff));
    int admin_len = hexStrToByte(argv[2], tmp_buff, strlen(argv[2]));
    setLockAdminKey(lock, tmp_buff, admin_len);
    printf( "setLockAdminKey success");

    memset(tmp_buff, 0, sizeof(tmp_buff));
    int password_size = hexStrToByte(argv[3], tmp_buff, strlen(argv[3]));
    setLockPassword(lock, tmp_buff, password_size);
    printf( "setLockPassword success");

    IgCreatePinRequest create_pin_request;
    ig_CreatePinRequest_init(&create_pin_request);

    memset(tmp_buff, 0, sizeof(tmp_buff));
    
    memset(tmp_buff, 0, sizeof(tmp_buff));
    password_size = hexStrToByte(argv[3], tmp_buff, strlen(argv[3]));
    ig_CreatePinRequest_set_password(&create_pin_request, tmp_buff, password_size);

    memset(tmp_buff, 0, sizeof(tmp_buff));
    int pin_size = hexStrToByte(argv[4], tmp_buff, strlen(argv[4]));
    ig_CreatePinRequest_set_new_pin(&create_pin_request, tmp_buff, pin_size);
    printf( "test create lock cmd test go");
    int res = testCreatePin(lock, &create_pin_request);
    printf( "test create lock end");
    // 为什么不需要这个, 因为我订立的协议是, 使用完, 立刻释放, 这已经被释放了.
    // 在 writeCreatePinRequest 里面已经释放了
    // ig_CreatePinRequest_deinit(&create_pin_request);
    releaseLock(&lock);
    return 0;
}

