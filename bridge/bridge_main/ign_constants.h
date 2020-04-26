#ifndef _IGN_CONSTANTS_H_
#define _IGN_CONSTANTS_H_

#define HOST "ssl://aa85fsnk5qn58-ats.iot.ap-southeast-1.amazonaws.com:8883"
#define SUBSCRIBE_CLIENT_ID "JasonSubscribeID"
#define CA_PATH "/tmp/igkey/"
#define TRUST_STORE "/tmp/igkey/ca.pem"
#define PRIVATE_KEY "/tmp/igkey/key.pem"
#define KEY_STORE "/tmp/igkey/cert.pem"

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