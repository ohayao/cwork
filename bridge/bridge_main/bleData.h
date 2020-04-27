#ifndef _BLE_DATA_H_
#define _BLE_DATA_H_

typedef struct BleData
{
  char *adapter_name; // 蓝牙接口的名字
  void *ble_param;  // 设置什么参数
  int  ble_param_len;    
  void *ble_result;   // 返回给逆什么结果
  int ble_result_len;
}ble_data_t;

int bleGetBleParamLen(ble_data_t *data, int *result_len);
int bleGetBleParam(ble_data_t *data, void *result);
int bleReleaseBleParam(ble_data_t *data);
int bleSetBleParam(ble_data_t *data, void *ble_param, int ble_param_len);

int bleGetBleResultLen(ble_data_t *data, int *result_len);
int bleGetBleResult(ble_data_t *data, void *result);
int bleReleaseBleResult(ble_data_t *data);
int bleSetBleResult(ble_data_t *data, void *ble_result, int ble_result_len);


#endif