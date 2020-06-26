#ifndef _FSM1_H_
#define _FSM1_H_
#include <stdint.h>
#include <stddef.h>

#define ERROR_EVENT (255)

// 状态基转移表
typedef struct FSMTransform
{
    uint8_t event;                /* 触发事件 */
    uint8_t CurState;             /* 当前状态 */
    void (*eventActFun)(void *);  /* 动作函数 */
    uint8_t NextState;            /* 跳转状态 */
}FSMTransform;


typedef struct FSM
{
    FSMTransform *fsm_trans_table;         /* 状态迁移表 */
    uint8_t cur_state;                      /* 状态机当前状态 */
    uint8_t max_state_num;          
}FSM;

// 获得一个FSM
int getFSM(FSM **p_fsm);
int initFSM(FSM *fsm, FSMTransform *fsm_trans_table, uint8_t max_state_num, uint8_t cur_state);

// 获得一个FSMTransform table
// 因为一个事件只有一个处理方式, 而且每个事件, 都是不同的
int getFSMTransformTable(FSM *fsm, int max_trans_num);
int insertFSMTransItem(FSM *fsm, FSMTransform *trans_item);

// 处理函数
void FSM_EventHandle(FSM *pFsm, uint8_t event, void *parm);
#endif