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
  serverLog(LL_NOTICE, "handleWriteStep1  start -----------------------");
  int ret = 0;
  RecvData *recv_pairing_data = (RecvData *)arg;
  size_t step1_len = 0;
  uint8_t *step1_payload_bytes = NULL;

  if (getRecvPkgLen(recv_pairing_data, &step1_len))
  {
    serverLog(LL_ERROR, "getRecvPkgLen error");
    return 1;
  }
  serverLog(LL_NOTICE, "getRecvPkgLen success, size %u", step1_len);

  step1_payload_bytes = malloc(step1_len);
  memset(step1_payload_bytes, 0, step1_len);

  if (getPkgFromRecvData(recv_pairing_data, step1_payload_bytes))
  {
    serverLog(LL_ERROR, "getPkgFromRecvData error");
    return 1;
  }
  serverLog(LL_NOTICE, "getPkgFromRecvData success");
  
  IgPairingStep1 step1;
  ig_PairingStep1_init(&step1);
  IgSerializerError step1_err = ig_PairingStep1_decode(step1_payload_bytes, step1_len, &step1, 0);
  if (step1_err )
  {
    serverLog(LL_ERROR, "ig_PairingStep1_decode error");
    return 1;
  }
  serverLog(LL_NOTICE, "ig_PairingStep1_decode success");

  if (!ig_PairingStep1_is_valid(&step1))
  {
    serverLog(LL_ERROR, "ig_PairingStep1_is_valid, not valid");
    return 1;
  }
  serverLog(LL_NOTICE, "ig_PairingStep1_is_valid, valid");


  serverLog(LL_NOTICE, "handleWriteStep1  end -----------------------");
  return ret;
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
  int step1Bytes_len = 0;
  int payload_len = 0;
  uint8_t *step1Bytes = NULL;
  uint8_t *payloadBytes = NULL;
  int ble_pkg_limit = 20;
  uint8_t ble_pkg[ble_pkg_limit];
  RecvData *recv_pairing_data = NULL;
  uint8_t event = ERROR_EVENT;

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

  // 初始化fsm
  initFSMCurState(fsm, PAIRING_BEGIN);

  // 假设开始写 write step1, 这些是外部产生的
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
    if (i%ble_pkg_limit==0 && i != 0)
    {
      printf("  i: %d \n", i);
    }
    printf("%x ", payloadBytes[i]);
    
  }
  printf("\n");

  // 到这里, 成功生成了 step1 的 步骤, 也就是 payloadBytes 
  // 会被成功传输到 brdige, 这就相当于, 我要开始接收这个字符串
  if (getRecvData(&recv_pairing_data))
  {
    serverLog(LL_ERROR, "getRecvData err");
    return 1;
  }
  serverLog(LL_NOTICE, "getRecvData success, need to free");

  if (initRecvData(recv_pairing_data))
  {
    serverLog(LL_ERROR, "initRecvData err");
    return 1;
  }
  serverLog(LL_NOTICE, "initRecvData success");


  printf("%d\n", payload_len);
  for (int i = 0 ; 
    i < payload_len; )
  {
    printf("receiving data %.3f\n", (double)i/payload_len);
    int copy_len = (payload_len-i-ble_pkg_limit>0?ble_pkg_limit:payload_len-i);
    memset(ble_pkg, 0, sizeof(ble_pkg));
    memcpy(ble_pkg, payloadBytes+i, copy_len);
    // 输出每次的包, 用来验证是否全部输出
    for (int j = 0; j < copy_len; j++)
    {
      printf("%x ", ble_pkg[j]);
    }
    printf("\n");

    // 这里验证开始接收到的pkg
    serverLog(LL_NOTICE, "before copy status: %d", recv_pairing_data->recv_status);
    recvData(recv_pairing_data, ble_pkg, copy_len);
    serverLog(LL_NOTICE, "before copy status: %d", recv_pairing_data->recv_status);

    // 验证是否拷贝对了
    for(int j = 0; j < copy_len; j++)
    {
      if ((recv_pairing_data->data[i+j]) != ble_pkg[j])
      {
        serverLog(LL_ERROR, "recvData copy error");
        return 1;
      }
    }

    if (copy_len != ble_pkg_limit)
    {
      printf("end receiving data %.3f\n", (double)(i+copy_len)/payload_len);
    }
    i += copy_len;
  }

  serverLog(LL_NOTICE, "recv_pairing_data->n_size_byte: %u", recv_pairing_data->n_size_byte);

  // 打印复制的包, 肉眼可以看到是拷贝对了
  serverLog(LL_NOTICE, "printf copy data");
  for (int i = 0; i < payload_len; i++)
  {
    if (i%ble_pkg_limit==0 && i != 0)
    {
      printf("  i: %d \n", i);
    }
    printf("%x ", recv_pairing_data->data[i]);
    
  }
  printf("\n");

  // 需要解开第一个包, 然后
  size_t step1_len = 0;
  if (getRecvPkgLen(recv_pairing_data, &step1_len))
  {
    serverLog(LL_ERROR, "getRecvPkgLen error");
    return 1;
  }
  serverLog(LL_NOTICE, "getRecvPkgLen success, size %u", step1_len);

  uint8_t *step1_payload_bytes = malloc(step1_len);
  memset(step1_payload_bytes, 0, step1_len);
  if (getPkgFromRecvData(recv_pairing_data, step1_payload_bytes))
  {
    serverLog(LL_ERROR, "getPkgFromRecvData error");
    return 1;
  }
  serverLog(LL_NOTICE, "getPkgFromRecvData success");

  // 打印一下拷贝, 确定确实拷贝对了:
  for (int i = 0; i < step1_len; i++)
  {
    if (i%ble_pkg_limit==0 && i != 0)
    {
      printf("  i: %d \n", i);
    }
    printf("%x ", step1_payload_bytes[i]);
  }
  printf("\n");

  // 解压数据包传过来的
  // 假设write_pairing_step1过来了
  event = C_WRITE_STEP1;
  handleEvent(fsm, event, recv_pairing_data);

  // getPkgFromRecvData()
  
  
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