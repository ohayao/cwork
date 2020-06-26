#include "bridge/wifi_service/fsm1.h"
#include "bridge/bridge_main/log.h"

// for pairing
#include "bridge/lock/messages/PairingStep1.h"
#include "bridge/lock/messages/PairingStep2.h"
#include "bridge/lock/messages/PairingStep3.h"
#include "bridge/lock/messages/PairingStep4.h"
#include "bridge/lock/messages/PairingCommit.h"
#include "bridge/lock/connection/pairing_connection.h"
#include "bridge/ble/ble_operation.h"
#include "bridge/wifi_service/pairing.h"




// 处理写的事件
int handleWriteStep1(void *arg)
{
  uint8_t *step1 = (uint8_t *)arg;
}

int handleReplyStep2(void *arg)
{

}

int handleWriteStep3(void *arg)
{
  uint8_t *step1 = (uint8_t *)arg;
}

int handleReplyStep4(void *arg)
{
  
}

// 主要用来展示状态机的用法
// 和测试状态机的正确性
int main(int argc, char *argv[])
{
  // 所需变量初始化
  FSM *fsm = NULL;
  int ret = 0;
  uint8_t max_trans_num;
  FSMTransform trans_item;
  size_t step1Bytes_len = 0;
  size_t payload_len = 0;
  uint8_t *step1Bytes = NULL;
  uint8_t *payloadBytes = NULL;
  size_t ble_pkg_limit = 20;
  size_t ble_pkg[ble_pkg_limit];
  RecvData *recv_pairing_data = NULL;

  // 获取
  if(getFSM(&fsm))
  {
    serverLog(LL_ERROR, "getFSM err");
    return 1;
  }
  serverLog(LL_NOTICE, "getFSM success");

  // 生成你自己的状态转换表
  // 我直接拿pairing 的状态转换来进行举例, 也好让我理清这些东西
  max_trans_num = 7;
  if (getFSMTransTable(fsm, max_trans_num))
  {
    serverLog(LL_ERROR, "getFSMTransTable err");
    return 1;
  }
  serverLog(LL_NOTICE, "getFSMTransTable success");

  // 1
  // C_WRITE_STEP1, from PAIRING_BEGIN to  PAIRING_STEP1
  if (fillTransItem(&trans_item, 
    PAIRING_STEP1, PAIRING_BEGIN, handleWriteStep1, PAIRING_STEP1))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillTransItem event C_WRITE_STEP1");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillFSMTransItem event C_WRITE_STEP1");

  // 2
  // S_REPLY_STEP2, from PAIRING_STEP1 to PAIRING_STEP2
  if (fillTransItem(&trans_item, 
    S_REPLY_STEP2, PAIRING_STEP1, handleReplyStep2, PAIRING_STEP2))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillTransItem event S_REPLY_STEP2");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillFSMTransItem event S_REPLY_STEP2");

  // 3
  // C_WRITE_STEP3, from PAIRING_STEP2 to PAIRING_STEP3
  if (fillTransItem(&trans_item, 
    C_WRITE_STEP3, PAIRING_STEP2, handleWriteStep3, PAIRING_STEP3))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillTransItem event C_WRITE_STEP3");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillFSMTransItem event C_WRITE_STEP3");

  // 4
  // S_REPLY_STEP4, from PAIRING_STEP3 to PAIRING_STEP4
  if (fillTransItem(&trans_item, 
    S_REPLY_STEP4, PAIRING_STEP3, handleWriteStep3, PAIRING_STEP4))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillTransItem event S_REPLY_STEP4");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillFSMTransItem event S_REPLY_STEP4");

  // 5
  // C_WRITE_COMMIT, from PAIRING_STEP4 to PAIRING_COMPLETE
  if (fillTransItem(&trans_item, 
    C_WRITE_COMMIT, PAIRING_STEP4, handleWriteStep3, PAIRING_COMPLETE))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillTransItem event C_WRITE_COMMIT");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }
  serverLog(LL_NOTICE, "fillFSMTransItem event C_WRITE_COMMIT");

  // 假设开始写 write step1
  // 生成一个step 1 所需要如下的步骤
  ret = igloohome_ble_lock_crypto_PairingConnection_beginConnection();
  if (!ret)
  {
    serverLog(LL_ERROR, "beginConnection fail");
    return 1;
  }
  serverLog(LL_NOTICE, "beginConnection success");

  step1Bytes_len = 
		igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(
			                                                            &step1Bytes);
  if (step1Bytes == NULL)
  {
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native fail");
    return 1;
  }
  serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native success");

  if (!build_msg_payload(&payloadBytes, &payload_len, step1Bytes, step1Bytes_len))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		return 1;
	}
  serverLog(LL_NOTICE, "success in build_msg_payload");

  // 打印生成的包
  for (int i = 0; i < payload_len; i++)
  {
    printf("%x", payloadBytes[i]);
    if (i%ble_pkg_limit==0 && i != 0)
    {
      printf("  i: %d \n", i);
    }
  }
  printf("\n");

  // 到这里, 成功生成了 step1 的 步骤, 也就是 payloadBytes 
  // 会被成功传输到 brdige, 这就相当于, 我要开始接收这个字符串
  
  printf("%d\n", payload_len);
  for (int i = 0 ; 
    i < payload_len; 
    i += (payload_len-i-ble_pkg_limit>0?ble_pkg_limit:payload_len-i) )
  {
    printf("%d\n", i);

  }
  printf("%d\n", payload_len % ble_pkg_limit);

  // 释放 转换表
  if (freeFSMTransTable(fsm))
  {
    serverLog(LL_ERROR, "freeFSMTransTable err");
    return 1;
  }
  serverLog(LL_NOTICE, "freeFSMTransTable success");

  // 释放
  if (freeFSM(&fsm))
  {
    serverLog(LL_ERROR, "freeFSM err");
    return 1;
  }
  serverLog(LL_NOTICE, "freeFSM success");
  
  

  return 0;
}