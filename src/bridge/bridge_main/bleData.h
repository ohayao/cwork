#ifndef _BLE_DATA_H_
#define _BLE_DATA_H_

// 所有的数据, 在这里都有一份
// 所以使用者需要自己释放
typedef struct BleData {
	//char *adapter_name; // 蓝牙接口的名字
	//void *adapter;
	void *ble_param;  // 设置什么参数
	int  ble_param_len;    
	void *ble_result;   // 返回给你什么结果
	int ble_result_len;
	int n_of_result;
	void *ble_connection; // 为了能够线程安全, 所以不要用全局变量 
	int lock_status;
	int battery_level;
}ble_data_t;

int bleInitData(ble_data_t *data);
int bleReleaseData(ble_data_t **pp_data);


// 参数 param部分
int bleGetBleParamLen(ble_data_t *data, int *result_len);
int bleGetBleParam(ble_data_t *data, void *result);
int bleReleaseBleParam(ble_data_t *data);
int bleSetBleParam(ble_data_t *data, void *ble_param, int ble_param_len);

// 返回的 结果 result 部分
int bleGetBleResultLen(ble_data_t *data, int *result_len);
int bleGetBleResult(ble_data_t *data, void *result);

// 设置单个结果可以,问题是有些是多次多个结果
int bleReleaseBleResult(ble_data_t *data);
int bleSetBleResult(ble_data_t *data, void *ble_result);

// 所以有这个每次放入一个,但是要首先设置一个长度, 然后再一个一个放进去
int bleInitResults(ble_data_t *data, int n_results, int size_of_result);
int blePutResults(ble_data_t *data, void *ble_result, int size_of_result);
int bleGetNumsOfResult(ble_data_t *data);
void *bleGetNResult(ble_data_t *data, int n, int size_of_result);

// connection 部分

#endif
