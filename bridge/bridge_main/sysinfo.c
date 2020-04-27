#include <bridge/bridge_main/sysinfo.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t sysinfo_mutex = PTHREAD_MUTEX_INITIALIZER;

int _lock()
{
  return pthread_mutex_lock(&sysinfo_mutex);
}
int _unlock()
{
  return pthread_mutex_unlock(&sysinfo_mutex);
}

int sysinfoInit(sysinfo_t *sysinfo)
{
  _lock();
  memset(sysinfo, 0, sizeof(sysinfo_t));
  _unlock();
  return 0;
}

/// BLE param 区域 start 

int sysinfoGetBleParamLen(sysinfo_t *sysinfo, int *result_len)
{
  _lock();
  *result_len = sysinfo->ble_param_len;
  _unlock();
  return 0;
}

int sysinfoGetBleParam(sysinfo_t *sysinfo, void *result)
{
  _lock();
  memcpy(result, sysinfo->ble_param, sysinfo->ble_param_len);
  _unlock();
  return 0;
}


int sysinfoReleaseBleParam(sysinfo_t *sysinfo)
{
  if (sysinfo->ble_param)
  {
    _lock();
    sysinfo->ble_param_len = 0;
    free(sysinfo->ble_param);
    sysinfo->ble_param = NULL;
    _unlock();
  }
  return 0;
}

int sysinfoSetBleParam(sysinfo_t *sysinfo, void *ble_param, int ble_param_len)
{
  sysinfoReleaseBleParam(sysinfo);
  _lock();
  sysinfo->ble_param = calloc(ble_param_len, 1);
  // TODO 申请内存可能出错
  sysinfo->ble_param_len = ble_param_len;
  memcpy(sysinfo->ble_param, ble_param, sysinfo->ble_param_len);
  _unlock();
  return 0;
}

/// BLE result 区域 start 

int sysinfoGetBleResultLen(sysinfo_t *sysinfo, int *result_len)
{
  _lock();
  *result_len = sysinfo->ble_result_len;
  _unlock();
  return 0;
}


int sysinfoGetBleResult(sysinfo_t *sysinfo, void *result)
{
  _lock();
  memcpy(result, sysinfo->ble_result, sysinfo->ble_result_len);
  _unlock();
  return 0;
}

int sysinfoReleaseBleResult(sysinfo_t *sysinfo)
{
  if (sysinfo->ble_result)
  {
    _lock();
    sysinfo->ble_result_len = 0;
    free(sysinfo->ble_result);
    sysinfo->ble_result = NULL;
    _unlock();
  }
  return 0;
}

int sysinfoSetBleResult(sysinfo_t *sysinfo, void *ble_result, int ble_result_len)
{
  sysinfoReleaseBleResult(sysinfo);
  _lock();
  sysinfo->ble_param = calloc(ble_result_len, 1);
  // TODO 申请内存可能出错
  sysinfo->ble_result_len = ble_result_len;
  memcpy(sysinfo->ble_result, ble_result, sysinfo->ble_result_len);
  _unlock();
  return 0;
}
