#ifndef _FSM1_H_
#define _FSM1_H_
#include <stdint.h>
#include <stddef.h>

#define ERROR_EVENT (255)


typedef int (*ActionFunc)(void *);
// 状态基转移表
typedef struct FSMTransform
{
    uint8_t event;                /* 触发事件 */
    uint8_t cur_state;             /* 当前状态 */
    ActionFunc event_act_func;  /* 动作函数 */
    uint8_t next_state;            /* 跳转状态 */
}FSMTransform;


typedef struct FSM
{
    FSMTransform *fsm_trans_table;         /* 状态迁移表 */
    uint8_t cur_state;                      /* 状态机当前状态 */
    uint8_t max_state_num;          
}FSM;

// 获得一个FSM
int getFSM(FSM **p_fsm);
int freeFSM(FSM **p_fsm);
int initFSMCurState(FSM *fsm, uint8_t cur_state);
// 获得一个FSMTransform table
// 因为一个事件只有一个处理方式, 而且每个事件, 都是不同的
int getFSMTransTable(FSM *fsm, uint8_t max_trans_num);
int freeFSMTransTable(FSM *fsm);
// 拷贝复制进取, 根据的是事件, 因为事件,是唯一的
int fillFSMTransItem(FSM *fsm, FSMTransform *trans_item);
int fillTransItem(FSMTransform *trans_item, 
    uint8_t event, uint8_t CurState, ActionFunc eventActFun, 
    uint8_t NextState);

// 处理函数
int handleEvent(FSM *fsm, uint8_t event, void *arg);
#endif