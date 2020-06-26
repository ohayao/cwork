#include "bridge/wifi_service/fsm1.h"

// 测试pairing 的状态机
// 发生的pairing事件
typedef enum {
	C_SUBSCRIBE,
  C_WRITE_STEP1,
  C_WRITE_STEP3,
  C_WRITE_COMMIT,
  ERR_EVENT,
} PAIRING_EVENT;

int main(int argc, char *argv[])
{

}