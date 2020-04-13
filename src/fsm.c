#include <stdio.h>
#include <unistd.h>  //linux

enum
{
    DONE,
	CMD_INIT,
    INITED,
	CMD_REQ_USERINFO,
    CMD_UPDATE_USERINFO,
    CMD_CONNECT_LOCK,
    CMD_UPDATE_LOCKSTATUS,
    CMD_UNLOCK,
};


FsmTable_t IgnSMTable[] =
{
	{  CMD_REQ_USERINFO,        GetUserInfo,        CMD_UPDATE_USERINFO},
	{  CMD_UPDATE_USERINFO,     DealUserInfo,       CMD_CONNECT_LOCK},
    {  CMD_CONNECT_LOCK,        ScanLock,           CMD_UPDATE_LOCKSTATUS},
    {  CMD_UPDATE_LOCKSTATUS,   UpdateLockState,    DONE},
	{  CMD_UNLOCK,              UnLock,             CMD_UPDATE_LOCKSTATUS},

};

typedef struct FsmTable_s
{
	//int event;
	int CurState;
	void (*eventActFun)();
	int NextState;
}FsmTable_t;


typedef struct FSM_s
{
	FsmTable_t* FsmTable;
	int curState;
}FSM_t;

int g_max_num;  //状态表里含有的状态个数


void GetUserInfo()
{
	//send request to server to get userinfo
	printf("send request to server to get userinfo!\n");
}

void DealUserInfo()
{
	//Recv UserInfo from server 
	printf("recv userinfo from server!\n");
}

void ScanLock()
{
	//Scan and connect with locks
	printf("scan & connect with locks!\n");
}

void UpdateLockState()
{
	//update lock status to server
	printf("update lock status to server!\n");
}

void UnLock()
{
	//unlock
	printf("unlock!\n");
}

/*状态机注册*/
void FSM_Regist(FSM_t* pFsm, FsmTable_t* pTable)
{
	pFsm->FsmTable = pTable;
}

/*状态迁移*/
void FSM_StateTransfer(FSM_t* pFsm, int state)
{
	pFsm->curState = state;
}


/*事件处理*/
void FSM_EventHandle(FSM_t* pFsm, int event)
{
	FsmTable_t* pActTable = pFsm->FsmTable;
	void (*eventActFun)() = NULL;  //函数指针初始化为空
	int NextState;
	int CurState = pFsm->curState;
	int flag = 0; //标识是否满足条件

	/*获取当前动作函数*/
	for (int i = 0; i<g_max_num; i++)
	{
		//当且仅当当前状态下来个指定的事件，我才执行它
		if (event == pActTable[i].event && CurState == pActTable[i].CurState)
		{
			flag = 1;
			eventActFun = pActTable[i].eventActFun;
			NextState = pActTable[i].NextState;
			break;
		}
	}


	if (flag) //如果满足条件了
	{
		/*动作执行*/
		if (eventActFun)
		{
			eventActFun();
		}

		//跳转到下一个状态
		FSM_StateTransfer(pFsm, NextState);
	}
	else
	{
		// do nothing
	}
}

//初始化FSM
void InitFsm(FSM_t* pFsm)
{
	g_max_num = sizeof(XiaoMingTable) / sizeof(FsmTable_t);
	pFsm->curState = SLEEP;
	FSM_Regist(pFsm, XiaoMingTable);
}


//测试用的
void test(int *event)
{
	if (*event == 3)
	{
		*event = 1;
	}
	else
	{
		(*event)++;
	}
	
}


int main()
{
	FSM_t fsm;
	InitFsm(&fsm);
	int event = EVENT1; 
	//小明的一天,周而复始的一天又一天，进行着相同的活动
	while (1)
	{
		printf("event %d is coming...\n", event);
		FSM_EventHandle(&fsm, event);
		printf("fsm current state %d\n", fsm.curState);
		test(&event); 
		sleep(1);  //休眠1秒，方便观察
	}

	return 0;
}
