#include <bridge/bridge_main/task_queue.h>
#include <bridge/bridge_main/list.h>

list_head_t waiting_task_head = {&waiting_task_head, &waiting_task_head};
list_head_t doing_task_head = {&doing_task_head, &doing_task_head};
int waiting_task_inited = 0;
int doing_task_inited = 0;
pthread_mutex_t doing_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waiting_mutex = PTHREAD_MUTEX_INITIALIZER;

// -------------- empty list operation

int IsDEmpty()
{
  int result;
  pthread_mutex_lock(&doing_mutex);
  result = list_empty(&doing_task_head);
  pthread_mutex_unlock(&doing_mutex);
  return result;
}

task_node_t *GetDHeadNode()
{
  task_node_t *result;
  if (IsDoingEmpty())
  {
    return NULL;
  }
  pthread_mutex_lock(&doing_mutex);
  result = list_entry(doing_task_head.next, task_node_t, list);
  pthread_mutex_unlock(&doing_mutex);
  return result;
  
}

task_node_t *NextDTask(task_node_t *task_node)
{
  task_node_t *result;
  list_head_t* cur = &(task_node->list);
  pthread_mutex_lock(&doing_mutex);
  if (list_is_last(cur, &doing_task_head))
  {
    return NULL;
  }
  result = list_entry(cur->next, task_node_t, list);
  pthread_mutex_unlock(&doing_mutex);
}

task_node_t *InsertDTaskFront(
  unsigned int msg_id, unsigned char cs, mqtt_data_t *p_dataMQTT, ble_data_t *p_dataBLE)
{
  task_node_t *new_task = (task_node_t *)malloc(sizeof(task_node_t));
  new_task->msg_id = msg_id;
  new_task->dataMQTT = p_dataMQTT;
  new_task->dataBLE = p_dataBLE;
  new_task->cur_state = cs;
  pthread_mutex_lock(&doing_mutex);
  list_add(&(new_task->list), &doing_task_head);
  pthread_mutex_unlock(&doing_mutex);
  return new_task;
}

void DeleteDTask(task_node_t* tn)
{
  pthread_mutex_lock(&doing_mutex);
  list_del(&tn->list);
  pthread_mutex_unlock(&doing_mutex);
  free(tn);
}

void DTask2Waiting(task_node_t* tn) {
  pthread_mutex_lock(&doing_mutex);
  pthread_mutex_lock(&waiting_mutex);
  list_move(&(tn->list), &waiting_task_head);
  pthread_mutex_unlock(&waiting_mutex);
  pthread_mutex_unlock(&doing_mutex);
}

// -------------- empty list operation
int IsWEmpty()
{
  int result;
  pthread_mutex_lock(&waiting_mutex);
  result = list_empty(&waiting_task_head);
  pthread_mutex_unlock(&waiting_mutex);
  return result;
}

void WTask2Doing(task_node_t* tn) {
  pthread_mutex_lock(&doing_mutex);
  pthread_mutex_lock(&waiting_mutex);
  list_move(&(tn->list), &doing_task_head);
  pthread_mutex_unlock(&waiting_mutex);
  pthread_mutex_unlock(&doing_mutex);
}

void DeleteWTask(task_node_t* tn)
{
  pthread_mutex_lock(&waiting_mutex);
  list_del(&tn->list);
  pthread_mutex_unlock(&waiting_mutex);
  free(tn);
}