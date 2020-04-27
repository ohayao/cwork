#include <bridge/bridge_main/bleData.h>
#include <string.h>
#include <stdlib.h>

int bleGetBleParamLen(ble_data_t *data, int *result_len)
{
  *result_len = data->ble_param_len;
  return 0;
}

int bleGetBleParam(ble_data_t *data, void *result)
{
  memcpy(result, data->ble_param, data->ble_param_len);
  return 0;
}


int bleReleaseBleParam(ble_data_t *data)
{
  if (data->ble_param)
  {
    data->ble_param_len = 0;
    free(data->ble_param);
    data->ble_param = NULL;
  }
  return 0;
}

int bleSetBleParam(ble_data_t *data, void *ble_param, int ble_param_len)
{
  bleReleaseBleParam(data);
  data->ble_param = calloc(ble_param_len, 1);
  // TODO 申请内存可能出错
  data->ble_param_len = ble_param_len;
  memcpy(data->ble_param, ble_param, data->ble_param_len);
  return 0;
}

/// BLE result 区域 start 

int bleGetBleResultLen(ble_data_t *data, int *result_len)
{
  *result_len = data->ble_result_len;
  return 0;
}


int bleGetBleResult(ble_data_t *data, void *result)
{
  memcpy(result, data->ble_result, data->ble_result_len);
  return 0;
}

int bleReleaseBleResult(ble_data_t *data)
{
  if (data->ble_result)
  {
    data->ble_result_len = 0;
    free(data->ble_result);
    data->ble_result = NULL;
  }
  return 0;
}

int bleSetBleResult(ble_data_t *data, void *ble_result, int ble_result_len)
{
  bleReleaseBleResult(data);
  data->ble_param = calloc(ble_result_len, 1);
  // TODO 申请内存可能出错
  data->ble_result_len = ble_result_len;
  memcpy(data->ble_result, ble_result, data->ble_result_len);
  return 0;
}