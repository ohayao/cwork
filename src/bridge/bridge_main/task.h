#ifndef _TASK_H_
#define _TASK_H_

#include <bridge/bridge_main/sysinfo.h>
#include <bridge/bridge_main/fsm.h>
#include <bridge/bridge_main/mqttData.h>
#include <bridge/bridge_main/bleData.h>
#include <bridge/bridge_main/list.h>
#include <glib.h>

enum TASK_TYPE {
    TASK_BLE_DISCOVER = 1,
    TASK_BLE_PAIRING = 2,
    TASK_BLE_ADMIN_CONNECTION = 3,
    TASK_BLE_ADMIN_UNPAIR = 4,
    TASK_BLE_ADMIN_UNLOCK = 5,
    TASK_BLE_ADMIN_LOCK = 6,
    TASK_BLE_ADMIN_GETLOGS = 7,
    TASK_BLE_ADMIN_GETLOCKSTATUS = 8,
    TASK_BLE_ADMIN_CREATE_PIN_REQUEST = 9,
    TASK_BLE_ADMIN_GET_BATTERY_LEVEL = 10,
};

typedef struct TaskNode {
    struct list_head list;
    char lock_id[32];
    int task_type; // 需要它进行数据的分类处理, 例如扫描的数据,和paired的数据, admin_connection的数据
    // void* lockinfo; // 外部数据?
	//struct task_node *next;
	unsigned int msg_id;
	char* lock_cmd;
	unsigned int lock_cmd_size;
    ble_data_t *ble_data; // 外部数据, 外部释放?
    int ble_data_len;
    mqtt_data_t *mqtt_data; // 外部数据, 外部释放?
    int mqtt_data_len;
    //void * dataBtn;
    unsigned int start_time;
    unsigned char cur_state;
    sysinfo_t *sysif;
    fsm_table_t *task_sm_table; // 外部数据
    int sm_table_len;
    GMainLoop *loop;
    guint timeout_id;
}task_node_t;

// 获取param长度
int taskGetBleParamLen(task_node_t *sysinfo, int *result_len);
// 自备上述长度内存, 拷贝内容到自己相应区域
int taskGetBleParam(task_node_t *sysinfo, void *result);
// 释放已经设置的内容
int taskReleaseBleParam(task_node_t *sysinfo);
// 设置相应的参数内容
int taskSetBleParam(task_node_t *sysinfo, void *ble_param, int ble_param_len);

// 获取长度
int taskGetBleResultLen(task_node_t *sysinfo, int *result_len);
// 自备上诉长度内存, 拷贝结果内容到自备内存
int taskGetBleResult(task_node_t *sysinfo, void *result);
// 释放Ble,结果的内容
int taskReleaseBleResult(task_node_t *sysinfo);
// 设置相应结果
int taskSetBleResult(task_node_t *sysinfo, void *ble_result, int ble_result_len);

#endif
