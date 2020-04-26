#include <bridge/bridge_main/task.h>
#include <deque>
static std::deque<task_node_t> waiting_task;
static std::deque<task_node_t> doing_task;

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
void InsertTask(unsigned int msg_id, unsigned char cs, void * p_dataMQTT, void *p_dataBLE)
{
  
}