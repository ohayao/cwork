#ifndef _MQTT_CONSTANT_H_
#define _MQTT_CONSTANT_H_

//临时定义发布和订阅的TOPIC
#define PUB_TOPIC "igh/dev/abcdef"
#define SUB_TOPIC "igh/srv/abcdef"

//定义订阅和发布web端TOPIC
#define SUB_WEBDEMO "/WEBSOCKET_DEMO_SUB"
#define PUB_WEBDEMO "/WEBSOCKET_DEMO_PUB"

#define HOST "ssl://aa85fsnk5qn58-ats.iot.ap-southeast-1.amazonaws.com:8883"
#define SUBSCRIBE_CLIENT_ID "JasonSubscribeID"
#define CA_PATH "/tmp/igkey/"
#define TRUST_STORE "/tmp/igkey/ca.pem"
#define PRIVATE_KEY "/tmp/igkey/key.pem"
#define KEY_STORE "/tmp/igkey/cert.pem"

#endif