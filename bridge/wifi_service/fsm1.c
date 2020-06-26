#include <stdlib.h>

#include "bridge/wifi_service/fsm1.h"
#include "bridge/bridge_main/log.h"


static inline void transState(FSM *fsm, uint8_t state)
{
    fsm->cur_state = state;
}                                                        

int getFSM(FSM **p_fsm)
{
    if (!p_fsm) 
    {
        serverLog(LL_ERROR, "getFSM p_fsm null");
        return 1;
    }
    if (!*p_fsm)
    {
        serverLog(LL_ERROR, "getFSM *p_fsm not null");
        return 1;
    } 
    p_fsm = malloc(sizeof(FSM));
    return 0;
}

int initFSM(FSM *fsm, FSMTransform *fsm_trans_table, uint8_t max_state_num, uint8_t cur_state)
{
    if (!fsm) return 1;
    fsm->fsm_trans_table = fsm_trans_table;
    fsm->cur_state = cur_state;
    fsm->max_state_num = max_state_num;
    return 0;
}

void handleEvent(FSM *fsm, uint8_t event, void *parm)
{
    FSMTransform *trans_table = fsm->fsm_trans_table;
    void (*eventActFun)(void *) = NULL;
    uint8_t NextState;
    uint8_t CurState = fsm->cur_state;
    uint8_t flag = 0;
      
    for (uint8_t i = 0; i < fsm->max_state_num; i++)// 遍历状态表
    {
        if (event == trans_table[i].event && CurState == trans_table[i].CurState)
        {
            flag = 1;
            eventActFun = trans_table[i].eventActFun;
            NextState = trans_table[i].NextState;
            break;
        }
    }
    if (flag)
    {
        if (eventActFun != NULL)
        {
            eventActFun(parm);  // 执行相应动作
        }
        transState(fsm, NextState); // 状态转换
    }
    else
    {
        serverLog(LL_ERROR, "handleEvent wrong");
    }
}