#ifndef _MQTT_CONSTANT_H_
#define _MQTT_CONSTANT_H_

//临时定义发布和订阅的TOPIC
//#define PUB_TOPIC "igh/dev/abcdef"
//#define SUB_TOPIC "igh/srv/abcdef"

//定义订阅和发布web端TOPIC
//#define SUB_WEBDEMO "/WEBSOCKET_DEMO_SUB"
//#define PUB_WEBDEMO "/WEBSOCKET_DEMO_PUB"

//临时定义发布和订阅的TOPIC   
#define PUB_TOPIC "igh/dev/IGN123456789"
#define SUB_TOPIC "igh/srv/IGN123456789"

//定义订阅和发布web端TOPIC    
#define SUB_WEBDEMO "/WEBSOCKET_DEMO_SUB"
#define PUB_WEBDEMO "/WEBSOCKET_DEMO_PUB"


#define HOST "ssl://aa85fsnk5qn58-ats.iot.ap-southeast-1.amazonaws.com:8883"
#define SUBSCRIBE_CLIENT_ID "JasonSubscribeID_0001"

#define CA_PATH "../../../igkey/"
#define TRUST_STORE "../../../igkey/ca.pem"
#define PRIVATE_KEY "../../../igkey/key.pem"
#define KEY_STORE "../../../igkey/cert.pem"

#endif
