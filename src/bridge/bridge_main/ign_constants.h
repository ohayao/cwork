#ifndef _IGN_CONSTANTS_H_
#define _IGN_CONSTANTS_H_



enum FSM_STATE
{
  BEGIN,
  DONE,
	CMD_INIT,
  INITED,
  GET_WIFI_USER,
	CMD_REQ_USERINFO,
  CMD_UPDATE_USERINFO,
  CMD_CONNECT_LOCK,
  CMD_UPDATE_LOCKSTATUS,
  CMD_UNLOCK
};

#endif