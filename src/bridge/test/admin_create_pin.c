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
            printf( "saving ble TASK_BLE_ADMIN_CREATE_PIN_REQUEST data.\n");
            ble_admin_result_t *create_pin_result = (ble_admin_result_t *)ble_data->ble_result;
            int create_pin_request_error = create_pin_result->create_pin_request_result;
            IgCreatePinResponse *create_pin_response = create_pin_result->cmd_response;
            
            if (create_pin_request_error) {
                printf( "create pin request error[%d].\n", create_pin_request_error);
            } else {
                printf( "create pin request success.\n");
                if (create_pin_response->has_operation_id)
                    printf( "operation ID: [%d].\n", create_pin_response->operation_id);
                if (create_pin_response->has_result)
                    printf( "result: [%d].\n", create_pin_response->result);
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
    printf("create pin request cmd ask invoker to release the lock.\n");
      
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
                printf("%d step error.\n", j);
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
    printf( "lock end-------\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
      printf("%s <device_address> <admin_key> <passwd> <pin> \n", argv[0]);
      return 1;
    }
    printf("test ble create pin request ing to start.\n");
    
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
    printf( "setLockPassword success.\n");

    IgCreatePinRequest create_pin_request;
    ig_CreatePinRequest_init(&create_pin_request);

    memset(tmp_buff, 0, sizeof(tmp_buff));
    password_size = hexStrToByte(argv[3], tmp_buff, strlen(argv[3]));
    ig_CreatePinRequest_set_password(&create_pin_request, tmp_buff, password_size);

    memset(tmp_buff, 0, sizeof(tmp_buff));
	
    //int pin_size = hexStrToByte(argv[4], tmp_buff, strlen(argv[4]));
	int pin_size = strlen(argv[4]);
	for(int i=0;i<pin_size; i++) {
		tmp_buff[i] = argv[4][i];
	}
	//unsigned int n = atoi(argv[4]);
	/*
	if (n<1000000 || 999999999<n) {
		printf("pin [%d] err.\n", n);
		return -1;
	}*/
	/*
	for(int j=1;j<=pin_size;j++) {
		tmp_buff[pin_size-j] = n%10;
		n = n/10;
	}
	*/
	printf("pin is:[");
	for(int k=0;k<pin_size;k++) {
		printf("%d", tmp_buff[k]);
	}
	printf("]\n");

    ig_CreatePinRequest_set_new_pin(&create_pin_request, tmp_buff, pin_size);
    printf( "test create lock cmd test go.\n");
	ig_CreatePinRequest_set_start_date(&create_pin_request, time(0));
	ig_CreatePinRequest_set_end_date(&create_pin_request, time(0)+10000);
	ig_CreatePinRequest_set_pin_type(&create_pin_request, 2);
	ig_CreatePinRequest_set_operation_id(&create_pin_request, 1);
    int res = testCreatePin(lock, &create_pin_request);
    printf( "test create lock end.\n");
    // 为什么不需要这个, 因为我订立的协议是, 使用完, 立刻释放, 这已经被释放了.
    // 在 writeCreatePinRequest 里面已经释放了
    // ig_CreatePinRequest_deinit(&create_pin_request);
    releaseLock(&lock);
    return 0;
}

