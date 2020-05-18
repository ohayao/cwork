#include <bridge/bridge_main/bleData.h>
#include <string.h>
#include <stdlib.h>
#include <bridge/bridge_main/log.h>

int bleInitData(ble_data_t *data)
{
  memset(data, 0, sizeof(ble_data_t));
  return 0;
}

int bleReleaseData(ble_data_t **pp_data)
{
  bleReleaseBleResult(*pp_data);
  bleReleaseBleParam(*pp_data);
  free(*pp_data);
  *pp_data = NULL;
}


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
  if (!data) return 1;
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
  data->ble_param_len = ble_param_len;;
  // 只会浅复制
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
  if (!data) return 1;
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
  data->ble_result = calloc(ble_result_len, 1);
  // TODO 申请内存可能出错
  data->ble_result_len = ble_result_len;
  memcpy(data->ble_result, ble_result, data->ble_result_len);
  return 0;
}

int bleInitResults(ble_data_t *data, int n_results, int size_of_result)
{
  bleReleaseBleResult(data);
  data->ble_result = calloc(n_results, size_of_result);
  data->ble_result_len = n_results;
  data->n_of_result = 0;
  return 0;
}

int blePutResults(ble_data_t *data, void *ble_result, int size_of_result)
{
  void *result_start = (data->ble_result) + ((data->n_of_result) * size_of_result);
  memcpy(result_start, ble_result, size_of_result);
  (data->n_of_result) = (data->n_of_result) + 1;
  return 0;
}

int bleGetNumsOfResult(ble_data_t *data)
{
  return data->n_of_result;
}

void *bleGetNResult(ble_data_t *data, int n, int size_of_result)
{
  return (data->ble_result + (n) * size_of_result);
}