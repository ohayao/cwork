#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_
#include <bridge/bridge_main/task.h>

// doing operation
int IsDEmpty();
task_node_t *GetDHeadNode();
task_node_t *NextDTask(task_node_t *task_node);
task_node_t *InsertDTaskFront(
		unsigned int msg_id, unsigned char cs, 
		mqtt_data_t *mqtt_data, int mqtt_data_len,
		ble_data_t *ble_data, int ble_data_len, 
		char* lock_cmd, unsigned int lock_cmd_size,
		fsm_table_t *task_sm_table, int sm_table_len, int task_type);
task_node_t *InsertDTaskTail(
		unsigned int msg_id, unsigned char cs, 
		mqtt_data_t *mqtt_data, int mqtt_data_len,
		ble_data_t *ble_data, int ble_data_len, 
		fsm_table_t *task_sm_table, int sm_table_len, int task_type);
task_node_t *InsertBle2DFront(
		unsigned int msg_id, unsigned char cs, 
		ble_data_t *ble_data, int ble_data_len, 
		char* lock_cmd, unsigned int lock_cmd_size,
		fsm_table_t *task_sm_table, int sm_table_len, int task_type);
task_node_t *InsertBle2DTail(
		unsigned int msg_id, unsigned char cs, 
		ble_data_t *ble_data, int ble_data_len, 
		fsm_table_t *task_sm_table, int sm_table_len, int task_type);
void DeleteDTask(task_node_t **ptn);
void DTask2Waiting(task_node_t* tn);

// waiting operation
int IsWEmpty();
void WTask2Doing(task_node_t* tn);
void DeleteWTask(task_node_t **tn);

// void InsertTask(
//     struct list_head* th, unsigned int msg_id, unsigned char cs, 
//     void * p_dataMQTT, void *p_dataBLE) {
// 	task_node_t *new_task = (task_node_t *)malloc(sizeof(struct task_node));
//     new_task->msg_id = msg_id;
// 	new_task->dataMQTT = p_dataMQTT;
// 	new_task->dataBLE = p_dataBLE;
//     new_task->cur_state = cs;
//     list_add(&new_task->list, th);
// }

// void DeleteTask(task_node_t* tn) {
//     list_del(&tn->list);
//     free(tn);
// }

// void MoveTask(list_head_t *list, list_head_t* to_list) {
//     list_move(list, to_list);
// }

// int DeleteTaskByMsgID(unsigned int msg_id, list_head_t* plh) {
//     list_head_t *pos = NULL;
//     task_node_t *pnode = NULL;
//     list_for_each(pos, plh) {
//         pnode = list_entry(pos, task_node_t, list);
//         if (msg_id == pnode->msg_id) {
//             list_del(&pnode->list);
//             free(pnode);
//             return 0;
//         }
//     }
//     return -1;
// }

// task_node_t* NextTask(list_head_t* cur, list_head_t* lh) {
//     if (list_is_last(cur, lh))
//         return NULL;
//     else
//         return list_entry(cur->next, task_node_t, list);
// }

// int IsEmpty(list_head_t* plh) {
//     return list_empty(plh);
// }

// task_node_t* FindTaskByMsgID(unsigned int msg_id, list_head_t* plh) {
//     list_head_t *pos = NULL;
//     task_node_t *pnode = NULL;
//     list_for_each(pos, plh) {
//         pnode = list_entry(pos, task_node_t, list);
//         if (msg_id == pnode->msg_id) {
//             return pnode;
//         }
//     }
//     return NULL;
// }

#endif
