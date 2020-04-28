#include <bridge/bridge_main/task_queue.h>
#include <bridge/bridge_main/list.h>
#include <string.h>
#include <stdlib.h>
#include <bridge/bridge_main/log.h>

list_head_t waiting_task_head = {&waiting_task_head, &waiting_task_head};
list_head_t doing_task_head = {&doing_task_head, &doing_task_head};
int waiting_task_inited = 0;
int doing_task_inited = 0;
pthread_mutex_t doing_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waiting_mutex = PTHREAD_MUTEX_INITIALIZER;

int _lockD()
{
  return pthread_mutex_lock(&doing_mutex);
}

int _unlockD()
{
  return pthread_mutex_unlock(&doing_mutex);
}

int _lockW()
{
  return pthread_mutex_lock(&waiting_mutex);
}

int _unlockW()
{
  return pthread_mutex_unlock(&waiting_mutex);
}

int _lockDW()
{
  int ret;
  ret = _lockD();
  ret += _lockW();
  return ret;
}

int _unlockDW()
{
  int ret;
  ret = _unlockW();
  ret += _unlockD();
  return ret;
}

// -------------- doing list operation


int IsDEmpty()
{
  int result;
  _lockD();
  result = list_empty(&doing_task_head);
  _unlockD();
  return result;
}

task_node_t *GetDHeadNode()
{
  task_node_t *result;
  if (IsDEmpty())
  {
    return NULL;
  }
  _lockD();
  result = list_entry(doing_task_head.next, task_node_t, list);
  _unlockD();
  return result;
  
}

// task_node 是当前的 node
task_node_t *NextDTask(task_node_t *task_node)
{
  if (!task_node)
  {
    return NULL;
  }
  task_node_t *result;
  list_head_t* cur = &(task_node->list);
  _lockD();
  if (list_is_last(cur, &doing_task_head))
  {
    _unlockD();
    return NULL;
  }
  result = list_entry(cur->next, task_node_t, list);
  _unlockD();
  return result;
}

task_node_t *InsertDTaskFront(
  unsigned int msg_id, unsigned char cs, 
  mqtt_data_t *mqtt_data, int mqtt_data_len,
  ble_data_t *ble_data, int ble_data_len, 
  fsm_table_t *task_sm_table, int sm_table_len)
{
  task_node_t *new_task = (task_node_t *)malloc(sizeof(task_node_t));
  new_task->msg_id = msg_id;
  // copy mqtt data
  if (mqtt_data_len && mqtt_data)
  {
    new_task->mqtt_data_len = mqtt_data_len;
    new_task->mqtt_data = calloc(new_task->mqtt_data_len, 1);
    // TODO, 内存分配不了?
    memcpy(new_task->mqtt_data, mqtt_data, new_task->mqtt_data_len);
  }
  if (ble_data_len && ble_data)
  {
    new_task->ble_data_len = ble_data_len;
    new_task->ble_data = calloc(new_task->ble_data_len, 1);
    // TODO, 内存分配不了?
    memcpy(new_task->ble_data, ble_data, new_task->ble_data_len);
  }
  if (task_sm_table && sm_table_len)
  {
    new_task->sm_table_len = sm_table_len;
    new_task->task_sm_table = calloc(new_task->sm_table_len, 1);
    // TODO, 内存分配不了?
    memcpy(new_task->task_sm_table, task_sm_table, new_task->sm_table_len);
  }
  new_task->cur_state = cs;
  _lockD();
  list_add(&(new_task->list), &doing_task_head);
  _unlockD();
  return new_task;
}

task_node_t *InsertBle2DFront(
  unsigned int msg_id, unsigned char cs, 
  ble_data_t *ble_data, int ble_data_len, 
  fsm_table_t *task_sm_table, int sm_table_len)
{
  return InsertDTaskFront(
    msg_id, cs, NULL, 0, ble_data, ble_data_len, task_sm_table, sm_table_len
  );
}

void DeleteDTask(task_node_t **ptn)
{
  task_node_t *tn = *ptn;
  _lockD();
  list_del(&tn->list);
  _unlockD();
  free(tn);
  *ptn = NULL;
}

void DTask2Waiting(task_node_t* tn) {
  
  _lockDW();
  list_move(&(tn->list), &waiting_task_head);
  _unlockDW();
}

// -------------- waiting list operation
int IsWEmpty()
{
  int result;
  _lockW();
  result = list_empty(&waiting_task_head);
  _unlockW();
  return result;
}

void WTask2Doing(task_node_t* tn) {
  _lockDW();
  list_move(&(tn->list), &doing_task_head);
  _unlockDW();
}

void DeleteWTask(task_node_t **ptn)
{
  task_node_t *tn = *ptn;
  _lockW();
  list_del(&tn->list);
  _unlockW();
  free(tn);
  *ptn = NULL;
}