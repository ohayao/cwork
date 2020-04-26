#ifndef _MQTT_DATA_H_
#define _MQTT_DATA_H_
#include <memory>
using std::weak_ptr;

class MqttData
{
public:
  weak_ptr<void> *data;
  // debug, 自己调试用, 假设上面法来一个操作
  int op;
  MqttData() = delete;
  MqttData(const int op_);
};

#endif