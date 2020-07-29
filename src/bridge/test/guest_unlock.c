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

#include <bridge/bridge_main/ign_constants.h>
#include <bridge/bridge_main/log.h>
#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/ign.h>
#include <bridge/bridge_main/task.h>
#include <bridge/bridge_main/thread_helper.h>
#include <bridge/bridge_main/mutex_helper.h>
#include <bridge/bridge_main/task_queue.h>
#include <bridge/bridge_main/wait_ble.h>
#include <bridge/ble/ble_discover.h>
#include <bridge/ble/lock.h>
#include <bridge/bridge_main/lock_list.h>
#include <bridge/ble/ble_guest.h>
#include "bridge/lock/messages/UnlockResponse.h"

void saveTaskData(task_node_t *ptn) {
	if (!ptn) return;

	if(ptn->ble_data && ptn->ble_data_len) {
		ble_data_t *ble_data = ptn->ble_data;
		int task_type = ptn->task_type;
		switch (task_type) {
			case TASK_BLE_ADMIN_UNLOCK:
				{
					printf( "saving ble TASK_BLE_ADMIN_UNLOCK data\n");
					ble_admin_result_t *admin_unlock_result = (ble_admin_result_t *)ble_data->ble_result;

					int unlock_error = admin_unlock_result->unlock_result;
					if (unlock_error) {
						printf( "unlock error\n");
					} else {
						printf( "unlock success\n");
					}
					printf( "admin_unlock response\n");
					IgUnlockResponse *unlock_response = admin_unlock_result->cmd_response;
					if (unlock_response->has_operation_id) {
						printf( "admin_unlock operation id[%d].\n", unlock_response->operation_id);
					}
					if (unlock_response->has_result) {
						printf( "admin_unlock result[%d].\n", unlock_response->result);
					}
					break;
				}
			default:
				break;
		}
	}
}

int testUnLock(igm_lock_t *lock) {
    printf("UnLock cmd ask invoker to release the lock.\n");
      
    ble_admin_param_t *admin_param = (ble_admin_param_t *)malloc(sizeof(ble_admin_param_t));
    bleInitGuestParam(admin_param);
    bleSetGuestParam(admin_param, lock);

    ble_data_t *ble_data = malloc(sizeof(ble_data_t));
    bleInitData(ble_data);
    bleSetBleParam(ble_data, admin_param, sizeof(ble_admin_param_t));

    fsm_table_t *unlock_fsm = getGuestUnlockFsmTable();
    int fsm_max_n = getGuestUnlockFsmTableLen();
    int current_state = BLE_GUEST_BEGIN;
    int error = 0;

    task_node_t *tn = (task_node_t *)malloc(sizeof(task_node_t));
    tn->ble_data = ble_data;

    tn->sm_table_len = fsm_max_n;
    tn->task_sm_table = unlock_fsm;

    tn->task_type = TASK_BLE_ADMIN_UNLOCK;

    for (int j = 0; j < fsm_max_n; j++) {
        if (current_state == tn->task_sm_table[j].cur_state) {
            // 增加一个判断当前函数, 是否当前函数出错. 0 表示没问题
			int event_result = tn->task_sm_table[j].eventActFun(tn);
            if (event_result) {
                printf( "%d step error\n", j);
                error = 1;
                break;
            } else {
                current_state = tn->task_sm_table[j].next_state;
            }
		}
    }
    if (error) {
        printf( "unlock error\n");
        return error;
    }

    saveTaskData(tn);
    ble_admin_result_t *admin_unlock_result = (ble_admin_result_t *)ble_data->ble_result;
    releaseGuestResult(&admin_unlock_result);
    bleReleaseBleResult(ble_data);
    free(ble_data);
    ble_data = NULL;
    free(tn);
    tn = NULL;
    free(admin_param);
    admin_param = NULL;
    printf( "unlock end-------\n");
    return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 5) {
		printf( "%s <device_address> <key> <passwd> <token> \n", argv[0]);
		return 1;
	}
	printf("test ble unlock ing to start.\n");

	printf("select the lock you want to unlock.\n");
	igm_lock_t *lock=NULL;
	getLock(&lock);
	initLock(lock);
	setLockAddr(lock, argv[1], strlen(argv[1]));
	printf( "setLockAddr success\n");

	uint8_t tmp_buff[1024];
	memset(tmp_buff, 0, sizeof(tmp_buff));
	int admin_len = hexStrToByte(argv[2], tmp_buff, strlen(argv[2]));
	setLockKey(lock, tmp_buff, admin_len);
    printf( "setLockKey success, size[%d]\n", admin_len);

    memset(tmp_buff, 0, sizeof(tmp_buff));
    int password_size = hexStrToByte(argv[3], tmp_buff, strlen(argv[3]));
    setLockPassword(lock, tmp_buff, password_size);
    printf( "setLockPassword success, size[%d]\n", password_size);

    memset(tmp_buff, 0, sizeof(tmp_buff));
	printf("get token[%s], len[%d]\n", argv[4], strlen(argv[4]));
    int token_size = hexStrToByte(argv[4], tmp_buff, strlen(argv[4]));
    setLockToken(lock, tmp_buff, token_size);
    printf( "setLockToken success, size[%d]\n", token_size);
	printf("token[");
	for(int i=0;i<token_size;i++){
		printf("%x", tmp_buff[i]);
	}
	printf("]\n");


    printf( "unlock cmd test start\n");
    int res = testUnLock(lock);
    printf( "unlock cmd test end\n");
    releaseLock(&lock);
    return 0;
}

