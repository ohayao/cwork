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
#include <bridge/ble/ble_pairing.h>
#include <bridge/bridge_main/lock_list.h>
#include <bridge/ble/ble_admin.h>
#include <bridge/ble/ble_pairing.h>
#include <bridge/mqtt/mqtt_constant.h>
#include <bridge/mqtt/mqtt_util.h>
#include <bridge/mqtt/cJSON.h>
#include <bridge/proto/ign.pb.h>
#include <bridge/proto/pb_encode.h>
#include <bridge/proto/pb_decode.h>
sysinfo_t g_sysif;

extern int WaitBtn(void *arg);
ign_BridgeProfile Create_IgnBridgeProfile(char *bridgeID);
void addPairingTask(igm_lock_t *lock, int msg_id);
void addDiscoverTask(int msg_id);

int FSMHandle(task_node_t* tn) {
    if(NULL == tn->task_sm_table) {
        serverLog(LL_ERROR, "sm_table is NULL.");
        return -1;
    }
    // sizeof 在定义某数组的时候, sizeof 指针 返回整个数组的字节数
    // sizeof 对指针的时候, 只能返回指针大小
    // sizeof 对指针所知数组的某个内容的时候, 只能返回该项大小, 
    // 也就是只会返回 sm_table 当中某一项的大小
    // 所以我手动返回了长度.
	unsigned int table_max_num = tn->sm_table_len;
    serverLog(LL_NOTICE, "table_max_num %d", table_max_num);
	int flag = 0;
    // 这儿是遍历所有的状态.
	for (int i = 0; i<table_max_num; i++) {
        serverLog(LL_NOTICE, "FSMHandle i %d ", i);
		if (tn->cur_state == tn->task_sm_table[i].cur_state) {
            serverLog(LL_NOTICE, "eventActFun begin---------------");
            // 增加一个判断当前函数, 是否当前函数出错. 0 表示没问题
			int event_result = tn->task_sm_table[i].eventActFun(tn);
            if (event_result)
            {
                flag = 0;
                break;
            }
            else
            {
                tn->cur_state = tn->task_sm_table[i].next_state;
                flag = 1;
            }
            serverLog(LL_NOTICE, "eventActFun end-----------------");
            
		}
	}
    serverLog(LL_NOTICE, "FSMHandle out for end-----------------");
    if (0 == flag) {
		// do nothing
        // sm or cur_state err
        serverLog(LL_ERROR, "something wrogin int fsm, one of the event function error, state(%d) error.", tn->cur_state);
        return -1;
	}
    serverLog(LL_NOTICE, "FSMHandle end");
    return 0;
}

ign_BridgeProfile Create_IgnBridgeProfile(char *bridgeID){
    ign_BridgeProfile bp={};
    bp.os_info=ign_OSType_LINUX;
    char temp[100];
    memset(temp,0,sizeof(temp));
    strcpy(temp,bridgeID);
    bp.bt_id.size=strlen(temp);
    memcpy(bp.bt_id.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"mac_addr");
    bp.mac_addr.size=strlen(temp);
    memcpy(bp.mac_addr.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"local_ip");
    bp.local_ip.size=strlen(temp);
    memcpy(bp.local_ip.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"public_ip");
    bp.public_ip.size=strlen(temp);
    memcpy(bp.public_ip.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"sys_statics");
    bp.sys_statics.size=strlen(temp);
    memcpy(bp.sys_statics.bytes,temp,strlen(temp));

    memset(temp,0,sizeof(temp));
    strcpy(temp,"wifi_ssid");
    bp.wifi_ssid.size=strlen(temp);
    memcpy(bp.wifi_ssid.bytes,temp,strlen(temp));
    bp.wifi_signal=2;
    bp.inited_time=get_ustime();

    memset(temp,0,sizeof(temp));
    strcpy(temp,"bridge_name");
    bp.name.size=strlen(temp);
    memcpy(bp.name.bytes,temp,strlen(temp));
    return bp;
}

int GetUserInfo(void* si) {
	//send request to server to get userinfo
	printf("send request to server to get userinfo!\n");
    return 0;
}

// int DealUserInfo(void* si) {
// 	//Recv UserInfo from server 
// 	printf("recv userinfo from server!\n");
//     return 0;
// }

// 首先实现这个扫描的 
int ScanLock(void* tn) {
	//Scan and connect with locks
	printf("scan & connect with locks!\n");
    return 0;
}

int UpdateLockState(void* tn) {
	//update lock status to server
	printf("update lock status to server!\n");
    return 0;
}

int UnLock(void* tn) {
	//unlock
	printf("unlock!\n");
    return 0;
}

int BLEParing(void* tn){
	printf("BLEParing!\n");
    return 0;
}

fsm_table_t g_fsm_table[] = {
    // {  CMD_INIT,                Init,               GET_WIFI_USER},
    // {  GET_WIFI_USER,           BLEParing,          CMD_REQ_USERINFO},
    // {  CMD_REQ_USERINFO,        GetUserInfo,        CMD_UPDATE_USERINFO},
    // {  CMD_UPDATE_USERINFO,     DealUserInfo,       CMD_CONNECT_LOCK},
    {  CMD_CONNECT_LOCK,        ScanLock,           CMD_UPDATE_LOCKSTATUS},
    {  CMD_UPDATE_LOCKSTATUS,   UpdateLockState,    DONE},
    {  CMD_UNLOCK,              UnLock,             CMD_UPDATE_LOCKSTATUS},
    {  }
};


// void GoExit(sysinfo_t *si) {
//     //BLE  
//     //MQTT
//     if(NULL != si->mqtt_c){
//         MQTTClient_disconnect(si->mqtt_c, 0);
//         MQTTClient_destroy(&si->mqtt_c);
//     }
// }

    /*
static int FSM(MQTTClient_message *msg){
    switch (msg) {
    case BTREE_TYPE_RBMT:
        err = ctree_tree_open(ctree, path, attr, fd);
        break;

    case BTREE_TYPE_BTRFS:
        err = ctree_bpt_open(ctree, path, attr, fd);
        break;

    case BTREE_TYPE_BITMAP:
        err = do_open_bitmap(ctree, path, attr, fd);
        break;

    default:
        err = -EINVAL;
    }
    return 0;
}
    */


// int HeartBeat(){
//     //send MQTT HB to Server
//     return 0;
// }


// 用法
void visitScanResult(ble_data_t *ble_data)
{
    serverLog(LL_NOTICE, "6. after the task is finished, we can get the data like this.");
    serverLog(LL_NOTICE, "7. get the result point.");
    int num_of_result = bleGetNumsOfResult(ble_data);
    void *result = ble_data->ble_result;
    for (int j=0; j < num_of_result; j++)
    {
        serverLog(LL_NOTICE, "8. get the j:% lock.", j);
        igm_lock_t *lock = bleGetNResult(ble_data, j, sizeof(igm_lock_t));
        serverLog(LL_NOTICE, "name %s  addr: %s", lock->name, lock->addr);
        // addPairingTask(lock);
    }
    serverLog(LL_NOTICE, "9. Release the ble data");
    bleReleaseData(&ble_data);
}

// 添加扫描方式样例
// ble_data 里面全市
void addDiscoverTask(int msg_id)
{
    // 设置需要的参数
    serverLog(LL_NOTICE, "Add Discover task");
    serverLog(LL_NOTICE, "1. set ble parameters");
    ble_discover_param_t discover_param;
    serverLog(LL_NOTICE, "1. set scan_timeout to 5");
    discover_param.scan_timeout = 5;
    serverLog(LL_NOTICE, "2. set msg_id to 0(or anything you want)");
    // 把参数写入data, 当前有个问题就是, 使用完, 得访问的人记的释放.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, &discover_param, sizeof(ble_discover_param_t));
    // 与记有多少个结果, 30个锁
    serverLog(LL_NOTICE, "3. init ble result memory, suppose the max num of locks is 30");
    bleInitResults(ble_data, 30, sizeof(igm_lock_t));

    // 插入系统的队列
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DTail(msg_id, BLE_DISCOVER_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getDiscoverFsmTable(), getDiscoverFsmTableLen(), TASK_BLE_DISCOVER
    );
    serverLog(LL_NOTICE, "5. Add Discover task.");
    return;
}

igm_lock_t *checkLockIsDiscovered(igm_lock_t *lock)
{
    int n_try_scan = 3;
    igm_lock_t *lock_nearby;
    while (n_try_scan--)
    {
        lock_nearby = findLockByName(lock->name);
        if (!lock_nearby)
        {
            serverLog(LL_NOTICE, "Pairing lock, not discover by the bridge, bridge scan first");
            contnueDiscoverLock();
        }
    }
    if (!lock_nearby)
    {
        serverLog(LL_NOTICE, "Pairing lock, not discover by the bridge");
        return NULL;
    }
    return lock_nearby;
}

igm_lock_t *checkLockIsPaired(igm_lock_t *lock)
{
    igm_lock_t *paired_lock;
    int n_try_paired = 3;
    int msg_id = 233;
    while (n_try_paired--)
    {
        if (lock && lock->paired)
        {
            return lock;
        }
        addPairingTask(lock, msg_id);
        sleep(1);
    }
    return NULL;
}

void addPairingTask(igm_lock_t *lock, int msg_id)
{

    // igm_lock_t *lock_nearby = checkLockIsDiscovered(lock);
    // if (!lock_nearby)
    // {
    //     serverLog(LL_ERROR, "can't not find lock nearby");
    //     return;
    // }
    igm_lock_t *lock_nearby = lock;
    // 设置需要的参数
    serverLog(LL_NOTICE, "Add Pairing task");
    serverLog(LL_NOTICE, "1. set ble pairing parameters");
    ble_pairing_param_t *pairing_param = (ble_pairing_param_t *)calloc(sizeof(ble_pairing_param_t), 1);
    serverLog(LL_NOTICE, "1. set pairing param lock to name %s addr %s", lock_nearby->name, lock_nearby->addr);
    bleSetPairingParam(pairing_param, lock_nearby);
    serverLog(LL_NOTICE, "2. set msg_id to 1(or anything you want)");
    // int msg_id = 1;
    // 把参数写入data, 当前有个问题就是, 使用完, 得访问的人记的释放.
    serverLog(LL_NOTICE, "3. alloc ble data datatype, ble_data is used to devliver parameters and get result data");
    ble_data_t *ble_data = calloc(sizeof(ble_data_t), 1);
    serverLog(LL_NOTICE, "3. init ble_data");
    bleInitData(ble_data);
    serverLog(LL_NOTICE, "3. set ble parametes to ble data");
    bleSetBleParam(ble_data, pairing_param, sizeof(ble_pairing_param_t));

    // 插入系统的队列
    serverLog(LL_NOTICE, "4. used InsertBle2DFront to insert the task to system.");
    InsertBle2DTail(msg_id, BLE_PAIRING_BEGIN, 
        ble_data, sizeof(ble_data_t),
        getPairingFsmTable(), getPairingFsmTableLen(), TASK_BLE_PAIRING);
    serverLog(LL_NOTICE, "5. Add Pairing task.");
    return;
}

void saveTaskData(task_node_t *ptn)
{
    if (!ptn) return;

    if(ptn->ble_data && ptn->ble_data_len)
    {
        ble_data_t *ble_data = ptn->ble_data;
        int task_type = ptn->task_type;
        switch (task_type)
        {
        case TASK_BLE_DISCOVER:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_DISCOVER data");
            int num_of_result = bleGetNumsOfResult(ble_data);
            void *result = ble_data->ble_result;
            for (int j=0; j < num_of_result; j++)
            {
                igm_lock_t *lock = bleGetNResult(ble_data, j, sizeof(igm_lock_t));
                serverLog(LL_NOTICE, "name %s  addr: %s", lock->name, lock->addr);
                insertLock(lock);
                // test 需要, 
                // if (!lock->paired)
                // {
                //     serverLog(LL_NOTICE, "try to pair name %s  addr: %s", lock->name, lock->addr);
                //     addPairingTask(lock);
                // }              
            }
            break;
        }
        case TASK_BLE_PAIRING:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_PAIRING data");
            ble_pairing_result_t *pairing_result = (ble_pairing_result_t *)ble_data->ble_result;
            serverLog(LL_NOTICE, "pairing_result:");
            serverLog(LL_NOTICE, "pairing success: %d", pairing_result->pairing_successed);
            serverLog(LL_NOTICE, "pairing admin_key:");
            for (int j = 0; j < pairing_result->admin_key_len; j++)
            {
              printf("%02x", pairing_result->admin_key[j]);
            }
            printf("\n");
            serverLog(LL_NOTICE, "pairing password:");
            for (int j = 0; j < pairing_result->password_size; j++)
            {
              printf("%02x", pairing_result->password[j]);
            }
            printf("\n");
            
            // igm_lock_t *lock = findLockByAddr(pairing_result->addr);
            // 只有找到这把锁, 并且匹配成功, 成保存称为Paired
            // 添加任务, 纯属测试需要
            // if (lock && pairing_result->pairing_successed)
            // {
            //     serverLog(LL_NOTICE, "set name %s addr %s to paired", lock->name, lock->addr);
            //     setLockPaired(lock);
            //     setLockAdminKey(lock, pairing_result->admin_key, pairing_result->admin_key_len);
            //     setLockPassword(lock, pairing_result->password, pairing_result->password_size);
            //     // addAdminTask(lock, 2);
            //     // addAdminUnpairTask(lock);
            //     // addAdminUnlockTask(lock);
            //     // addAdminLockTask(lock);
            // }
            break;
        }
        case TASK_BLE_ADMIN_CONNECTION:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_ADMIN_CONNECTION data");
        }
        case TASK_BLE_ADMIN_UNPAIR:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_ADMIN_UNPAIR data");
            ble_admin_result_t *admin_unpair_result = (ble_admin_result_t *)ble_data->ble_result;
            break;
        }
        case TASK_BLE_ADMIN_UNLOCK:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_ADMIN_UNLOCK data");
            ble_admin_result_t *admin_lock_result = (ble_admin_result_t *)ble_data->ble_result;
            break;
        }
        case TASK_BLE_ADMIN_LOCK:
        {
            serverLog(LL_NOTICE, "saving ble TASK_BLE_ADMIN_LOCK data");
            ble_admin_result_t *admin_lock_result = (ble_admin_result_t *)ble_data->ble_result;
            break;
        }
        default:
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    // Init(NULL);
    serverLog(LL_NOTICE,"test ble discover Ready to start.");
    if (argc != 2) {
      serverLog(LL_NOTICE, "%s <device_address>\n", argv[0]);
      return 1;
    }

    int msg_id = 1;
    // addDiscoverTask(1);
    igm_lock_t lock;
    setLockAddr(&lock, argv[1], strlen(argv[1]));
    addPairingTask(&lock, msg_id);
    while(1) {
        //if empty, sleep(0.5);
        //do it , after set into waiting_list
        if (IsDEmpty()) {
            serverLog(LL_NOTICE,"doing_task_head is empty, check Lock list.");
            // 应该去检查waiting list
            printLockList();
            serverLog(LL_NOTICE,"doing_task_head is empty, ready to sleep.");
            // sleep(1);
            break;
        } 
        else {
        // if doing list has task
            // 获取当前doing list 的头部
            task_node_t *ptn=GetDHeadNode();
            while (ptn)
            {
                // TODO, 当任务完成,需要怎么处理?
                int ret = FSMHandle(ptn);
                if(ret)
                {
                    serverLog(LL_NOTICE, "one mission error");
                }
                else
                {
                    serverLog(LL_NOTICE, "one mission finished, delete this task");
                }
                saveTaskData(ptn);
                DeleteDTask(&ptn); // 自动置 ptn 为 NULL
                
                task_node_t *tmp = NextDTask(ptn);
                if (tmp)
                {
                    serverLog(LL_NOTICE, "NextDTask not NULL");
                }
                
                ptn = tmp;
            }
        }
    }
    return 0;
}

