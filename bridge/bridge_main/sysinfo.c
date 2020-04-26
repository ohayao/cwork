#include <bridge/bridge_main/sysinfo.h>
#include <string.h>
#include <stdio.h>

SysInfo::SysInfo()
{
  inited = true;
  memset(wifi_ssid, 0, sizeof(wifi_ssid));
  memset(wifi_pswd, 0, sizeof(wifi_pswd));
  memset(user_id, 0, sizeof(user_id));
  memset(user_pswd, 0, sizeof(user_pswd));
  memset(lock1_id, 0, sizeof(lock1_id));
  memset(lock2_id, 0, sizeof(lock2_id));
  memset(lock3_id, 0, sizeof(lock3_id));
  memset(lock4_id, 0, sizeof(lock4_id));
  memset(lock5_id, 0, sizeof(lock5_id));
  userinfo = NULL;
  lock1info = NULL;
  lock2info = NULL;
  lock3info = NULL;
  lock4info = NULL;
  lock5info = NULL;
}