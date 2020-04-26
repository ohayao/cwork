#ifndef _MQTT_DATA_H_
#define _MQTT_DATA_H_

typedef struct MqttData
{
  void *data;
  // debug, 自己调试用, 假设上面法来一个操作
  int op;
}mqtt_data_t;

#endif