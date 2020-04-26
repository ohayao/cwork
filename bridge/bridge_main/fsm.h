#ifndef _FSM_H_
#define _FSM_H_

typedef struct st_fsm_table {
	unsigned char cur_state;
	int (*eventActFun)(void*);
	unsigned char next_state;
}fsm_table_t;

#endif