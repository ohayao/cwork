#include <stdlib.h>

#include "bridge/wifi_service/fsm1.h"
#include "bridge/bridge_main/log.h"


static inline void transState(FSM *fsm, uint8_t state)
{
    fsm->cur_state = state;
}

int freeFSM(FSM **p_fsm)
{
    if (!p_fsm)
    {
        serverLog(LL_ERROR, "freeFSM p_fsm null");
        return 1;
    }
    if (!*p_fsm)
    {
        serverLog(LL_ERROR, "freeFSM *p_fsm is null");
        return 1;
    }
    free(*p_fsm);
    *p_fsm = NULL;
    return 0;
}                                                        

int getFSM(FSM **p_fsm)
{
    if (!p_fsm) 
    {
        serverLog(LL_ERROR, "getFSM p_fsm null");
        return 1;
    }
    if ((*p_fsm))
    {
        serverLog(LL_ERROR, "getFSM *p_fsm not null");
        return 1;
    } 
    *p_fsm = malloc(sizeof(FSM));
    return 0;
}

int initFSMCurState(FSM *fsm, uint8_t cur_state)
{
    if (!fsm) return 1;
    fsm->cur_state = cur_state;;
    return 0;
}

int handleEvent(FSM *fsm, uint8_t event, void *arg)
{
    serverLog(LL_NOTICE, "------------------- handleEvent");
    if (event == ERROR_EVENT)
    {
        serverLog(LL_ERROR, "event is ERROR_EVENT");
        return 1;
    }
    int ret = 1;
    FSMTransform *trans_table = fsm->fsm_trans_table;
    ActionFunc event_act_func = NULL;
    uint8_t next_state;
    uint8_t cur_state = fsm->cur_state;
    uint8_t flag = 0;
    serverLog(LL_NOTICE, "cur_state %d event %d", cur_state, event);
    serverLog(LL_NOTICE, "max_state_num %d", fsm->max_state_num);
    for (uint8_t i = 0; i < fsm->max_state_num; i++)// 遍历状态表
    {
        serverLog(LL_NOTICE, "event %d cur_state %d", event, cur_state);
        if (event == trans_table[i].event && cur_state == trans_table[i].cur_state)
        {
            flag = 1;
            event_act_func = trans_table[i].event_act_func;
            next_state = trans_table[i].next_state;
            break;
        }
    }
    if (flag)
    {
        if (event_act_func != NULL)
        {
            ret = event_act_func(arg);  // 执行相应动作
        }
        transState(fsm, next_state); // 状态转换
    }
    else
    {
        serverLog(LL_ERROR, "handleEvent wrong");
    }
    return ret;
}

int getFSMTransTable(FSM *fsm, uint8_t max_trans_num)
{
    if (!fsm)
    {
        serverLog(LL_ERROR, "getFSMTransTable fsm null");
        return 1;
    }
    if (fsm->fsm_trans_table)
    {
        serverLog(LL_ERROR, "getFSMTransTable fsm_trans_table is not NULL");
        return 1;
    }
    // 分配 max_trans_num 个 转换状态
    fsm->fsm_trans_table = malloc(sizeof(FSMTransform) * max_trans_num);
    fsm->max_state_num = max_trans_num;
    return 0;
}

int freeFSMTransTable(FSM *fsm)
{
    if (!fsm)
    {
        serverLog(LL_ERROR, "freeFSMTransTable fsm null");
        return 1;
    }
    if (!fsm->fsm_trans_table)
    {
        serverLog(LL_ERROR, "freeFSMTransTable fsm->fsm_trans_table is null");
        return 1;
    }
    free(fsm->fsm_trans_table);
    fsm->fsm_trans_table = NULL;
    return 0;
}

int fillFSMTransItem(FSM *fsm, FSMTransform *trans_item)
{
    if (!fsm)
    {
        serverLog(LL_ERROR, "fillFSMTransItem fsm is null");
        return 1;
    }
    if (!fsm->fsm_trans_table)
    {
        serverLog(LL_ERROR, "fillFSMTransItem fsm->fsm_trans_table is null");
        return 1;
    }
    if (!trans_item)
    {
        serverLog(LL_ERROR, "fillFSMTransItem trans_item is null");
        return 1;
    }
    memcpy(
        fsm->fsm_trans_table + trans_item->event,
        trans_item,
        sizeof(FSMTransform)
    );

    return 0;
}

int fillTransItem(FSMTransform *trans_item, 
    uint8_t event, uint8_t cur_state, ActionFunc event_act_func, 
    uint8_t next_state)
{
    if (!trans_item)
    {
        serverLog(LL_ERROR, "fillTransItem trans_item is null");
        return 1;
    }
    trans_item->cur_state = cur_state;
    trans_item->next_state = next_state;
    // 确定这个地址是不变的哦
    trans_item->event_act_func = event_act_func;
    trans_item->event = event;
    return 0;
}