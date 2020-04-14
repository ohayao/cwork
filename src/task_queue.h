#include "list.h"

typedef struct task_node {
    struct list_head list;
	//struct task_node *next;
	unsigned int msg_id;
    void * req;
    void * res;
    unsigned char current_state;
}task_node_t;

//LIST_HEAD(task_head);
//INIT_LIST_HEAD(task_head);


void InsertTask(struct list_head* th, unsigned int msg_id, void * req, void *rsp, unsigned char cs) {
	task_node_t *new_task = (task_node_t *)malloc(sizeof(struct task_node));
    new_task->msg_id = msg_id;
	new_task->req = req;
	new_task->res = rsp;
    new_task->current_state = cs;
    list_add(&new_task->list, th);
}

void DeleteTask(task_node_t* tn) {
    list_del(&tn->list);
    free(tn);
}

int DeleteTaskByMsgID(unsigned int msg_id, list_head_t* plh) {
    list_head_t *pos = NULL;
    task_node_t *pnode = NULL;
    list_for_each(pos, plh) {
        pnode = list_entry(pos, task_node_t, list);
        if (msg_id == pnode->msg_id) {
            list_del(&pnode->list);
            free(pnode);
            return 0;
        }
    }
    return -1;
}


task_node_t* NextTask(list_head_t* cur, list_head_t* lh) {
    if (list_is_last(cur, lh))
        return NULL;
    else
        return list_entry(cur->next, task_node_t, list);
}

task_node_t* FindTaskByMsgID(unsigned int msg_id, list_head_t* plh) {
    list_head_t *pos = NULL;
    task_node_t *pnode = NULL;
    list_for_each(pos, plh) {
        pnode = list_entry(pos, task_node_t, list);
        if (msg_id == pnode->msg_id) {
            return pnode;
        }
    }
    return NULL;
}

