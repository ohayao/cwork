#include "bridge/wifi_service/fsm1.h"

// for pairing
#include "bridge/lock/connection/pairing_connection.h"
#include "bridge/ble/ble_operation.h"
#include "bridge/wifi_service/pairing.h"
#include "bridge/wifi_service/fsm1.h"
#include "bridge/bridge_main/log.h"
#include <stdint.h>

#define kNonceLength 12
uint8_t *fake_transmit_payloadBytes = NULL;
int fake_transmit_payloadBytes_len = 0;
#define ble_pkg_limit  (20)
uint8_t ble_pkg[ble_pkg_limit];

int generateStep1AndSend()
{
  int ret = 0;
  uint8_t *step1Bytes = NULL;
  uint32_t step1Bytes_len = 0;
  uint32_t payload_len = 0;
  uint8_t *payloadBytes = NULL;
  // 假设开始写 write step1, 这些是外部产生的
  // 生成一个step 1 所需要如下的步骤
  ret = igloohome_ble_lock_crypto_PairingConnection_beginConnection();
  if (!ret)
  {
    serverLog(LL_ERROR, "beginConnection fail");
    return 1;
  }
  // serverLog(LL_NOTICE, "beginConnection success");

  step1Bytes_len = 
		igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(
			                                                            &step1Bytes);
  if (step1Bytes == NULL)
  {
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native fail");
    return 1;
  }
  // serverLog(LL_NOTICE, "igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native success");

  if (!build_msg_payload(&payloadBytes, &payload_len, step1Bytes, step1Bytes_len))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		return 1;
	}

  // serverLog(LL_NOTICE, "success in build_msg_payload");
  fake_transmit_payloadBytes = malloc(payload_len);
  memcpy(fake_transmit_payloadBytes, payloadBytes, payload_len);
  fake_transmit_payloadBytes_len = payload_len;
  free(step1Bytes);
  step1Bytes = NULL;
  free(step1Bytes);
  step1Bytes = NULL;
  free(payloadBytes);
  payloadBytes = NULL;
  return 0;
}

int fakeRecvData(void *arg)
{
  RecvData *recv_pairing_data = (RecvData *)arg;
  if (recv_pairing_data == NULL)
  {
    serverLog(LL_ERROR, "fakeRecvData  recv_pairing_data null");
    return 1;
  }
  //
  // printRecvData(recv_pairing_data);

  // 接收数据部分
  for (int i = 0 ; i < fake_transmit_payloadBytes_len; )
  {
    int copy_len = (fake_transmit_payloadBytes_len-i-ble_pkg_limit>0?ble_pkg_limit:fake_transmit_payloadBytes_len-i);
    // serverLog(LL_NOTICE, "copy_len: %d", copy_len);
    memset(ble_pkg, 0, sizeof(ble_pkg));
    memcpy(ble_pkg, fake_transmit_payloadBytes+i, copy_len);
    // 输出每次的包, 用来验证是否全部输出
    // for (int j = 0; j < copy_len; j++)
    // {
    //   printf("%x ", ble_pkg[j]);
    // }
    // printf("\n");

    // 这里验证开始接收到的pkg
    // serverLog(LL_NOTICE, "before copy status: %d", recv_pairing_data->recv_status);
    recvData(recv_pairing_data, ble_pkg, copy_len);
    // serverLog(LL_NOTICE, "before copy status: %d", recv_pairing_data->recv_status);

    // 验证是否拷贝对了
    // for(int j = 0; j < copy_len; j++)
    // {
    //   if ((recv_pairing_data->data[i+j]) != ble_pkg[j])
    //   {
    //     serverLog(LL_ERROR, "recvData copy error");
    //     return 1;
    //   }
    // }

    if (copy_len != ble_pkg_limit)
    {
      printf("end receiving data %.3f\n", (double)(i+copy_len)/fake_transmit_payloadBytes_len);
    }
    i += copy_len;
  }
  printRecvData(recv_pairing_data);
}

// 处理写的事件
int handleWriteStep1(void *arg)
{
  serverLog(LL_NOTICE, "handleWriteStep1  start -----------------------");
  int ret = 0;
  RecvData *recv_pairing_data = (RecvData *)arg;
  if (recv_pairing_data == NULL)
  {
    serverLog(LL_ERROR, "handleWriteStep1  recv_pairing_data null");
    return 1;
  }

  // test 
  printf("=============== handleWriteStep1 ==================\n");
  printf("fake_transmit_payloadBytes_len: %u\n", fake_transmit_payloadBytes_len);
  for (int i = 0; i < fake_transmit_payloadBytes_len; i += ble_pkg_limit)
  {
    for (int j = 0; j < ble_pkg_limit; ++j)
    {
      printf(" %x", fake_transmit_payloadBytes[i+j]);
    }
    printf("\n");
  }

  fakeRecvData(arg);  

  if(isRecvFullPkg(recv_pairing_data))
  {
    printf("-------handleWriteStep1 recv_pairing_data----------- \n");
    printf("recv_pairing_data->data_len: %u\n", recv_pairing_data->data_len);
    for (int i = 0; i < recv_pairing_data->data_len; i+=20)
    {
      for (int j = 0; j < 20; j++)
      {
        printf(" %x", recv_pairing_data->data[i+j]);
      }
      printf("\n");
    }

    // 收到的包, 是全部的,需要自己取出来
    serverLog(LL_NOTICE, "isRecvFullPkg");
    size_t step1_len = 0;
    uint8_t *step1_payload_bytes = NULL;
    if (getRecvPkgLen(recv_pairing_data, &step1_len))
    {
      serverLog(LL_ERROR, "getRecvPkgLen error");
      return 1;
    }
    // serverLog(LL_NOTICE, "getRecvPkgLen success, size %u", step1_len);

    step1_payload_bytes = malloc(step1_len);
    memset(step1_payload_bytes, 0, step1_len);

    if (getPkgFromRecvData(recv_pairing_data, step1_payload_bytes))
    {
      serverLog(LL_ERROR, "getPkgFromRecvData error");
      return 1;
    }

    uint8_t *step2_bytes = NULL;
    uint32_t step2_size = ig_pairing_step2_size();
    uint32_t step2_writen_len = 0;
    if (step2_size == 0)
    {
      serverLog(LL_ERROR, "ig_pairing_step2_size error");
      return 1;
    }

    step2_bytes = malloc(step2_size);
    memset(step2_bytes, 0, step2_size);

    // serverLog(LL_NOTICE, "getPkgFromRecvData success");
    if (server_gen_pairing_step2(
      step1_payload_bytes, step1_len, step2_bytes, step2_size, &step2_writen_len))
    {
      serverLog(LL_ERROR, "server_gen_pairing_step2 error");
      return 1;
    }

    serverLog(LL_NOTICE, "server_gen_pairing_step2 success");

    if (!ig_PairingStep2_is_valid((IgPairingStep2 *)step2_bytes))
    {
      serverLog(LL_ERROR, "step2 not valid error");
      return 1;
    }
    // serverLog(LL_NOTICE, "step 2 generate valid");

    uint8_t *payloadBytes = NULL;
    uint32_t payload_len = 0;

    if (!build_msg_payload(
      &payloadBytes, &payload_len, step2_bytes, step2_writen_len))
    {
      serverLog(LL_ERROR, "failed in build_msg_payload");
      return 1;
    }
    serverLog(LL_NOTICE, "success in build_msg_payload, size: %u", payload_len);

    if (fake_transmit_payloadBytes) free(fake_transmit_payloadBytes);
    fake_transmit_payloadBytes = NULL;
    fake_transmit_payloadBytes = (uint8_t *)malloc(payload_len);
    fake_transmit_payloadBytes_len = payload_len;
    memcpy(fake_transmit_payloadBytes, payloadBytes, payload_len);

    // fake_transmit_payloadBytes
    free(payloadBytes);
    payloadBytes = NULL;
    free(step2_bytes);
    step2_bytes = NULL;
    free(step1_payload_bytes);
    step1_payload_bytes = NULL;
  }
  resetRecvData(recv_pairing_data);
  return ret;
}

int handleReplyStep2(void *arg)
{
  serverLog(LL_NOTICE, "handleReplyStep2 ----------------");
  uint16_t step2_len = 0;
  uint16_t n_size_byte = 0;
  uint16_t pkg_len = 0;
  pkg_len = getDataLength(fake_transmit_payloadBytes, &n_size_byte, &step2_len);
  if (pkg_len == 0)
  {
    serverLog(LL_ERROR, "getRecvPkgLen error");
    return 1;
  }
  serverLog(LL_NOTICE, "handleReplyStep2 getDataLength success, pkg_len %u, n_size_byte %u, step2_len %u ", 
      pkg_len, n_size_byte, step2_len);
  
  uint8_t *step3Bytes = NULL;
  size_t step3Bytes_len = 0;

  step3Bytes_len = igloohome_ble_lock_crypto_PairingConnection_genPairingStep3Native(
    step2_len, fake_transmit_payloadBytes+n_size_byte, &step3Bytes);
  free(fake_transmit_payloadBytes);
  fake_transmit_payloadBytes = NULL;
  fake_transmit_payloadBytes_len = 0;

  if (!step3Bytes_len)
  {
    serverLog(LL_ERROR, "genPairingStep3Native error");
    return 1;
  }
  serverLog(LL_NOTICE, "genPairingStep3Native Success");

  uint8_t *payloadBytes = NULL;
  size_t payload_len = 0;

  if (!build_msg_payload(
		&payloadBytes, &payload_len, step3Bytes, step3Bytes_len))
	{
    serverLog(LL_ERROR, "build_msg_payload failed!");
		return 1;
	}
  // serverLog(LL_NOTICE, "build_msg_payload success!");

  // 在这儿, 成功生成了 step3
  // 我们 把它拷贝到 fake_transmit_payloadBytes, 假装发送
  if(fake_transmit_payloadBytes) free(fake_transmit_payloadBytes);
  fake_transmit_payloadBytes = NULL;
  fake_transmit_payloadBytes = malloc(payload_len);
  memcpy(fake_transmit_payloadBytes, payloadBytes, payload_len);
  fake_transmit_payloadBytes_len = payload_len;
  if (step3Bytes)
  {
    free(step3Bytes);
    step3Bytes = NULL;
  }
  if (payloadBytes)
  {
    free(payloadBytes);
    payloadBytes = NULL;
  }
  return 0;
}


int handleWriteStep3(void *arg)
{
  serverLog(LL_NOTICE, " handleWriteStep3 -----------------------");
  int ret = 0;
  RecvData *recv_pairing_data = (RecvData *)arg;
  if (recv_pairing_data == NULL)
  {
    serverLog(LL_ERROR, "handleWriteStep1  recv_pairing_data null");
    return 1;
  }

  fakeRecvData(arg); 

  // 开始处理 step3 =============================
  // 确定step3 有加密
  if(isRecvFullPkg(recv_pairing_data))
  {
    serverLog(LL_NOTICE, "handle write step3 isRecvFullPkg");

    size_t step3_len = 0;
    uint8_t *step3_payload_bytes = NULL;
    if (getRecvPkgLen(recv_pairing_data, &step3_len))
    {
      serverLog(LL_ERROR, "getRecvPkgLen error");
      return 1;
    }
    // serverLog(LL_NOTICE, "getRecvPkgLen success, size %u", step1_len);

    step3_payload_bytes = malloc(step3_len);
    memset(step3_payload_bytes, 0, step3_len);

    if (getPkgFromRecvData(recv_pairing_data, step3_payload_bytes))
    {
      serverLog(LL_ERROR, "getPkgFromRecvData error");
      return 1;
    }

    uint32_t encrypt_step4_bytes_len = ig_pairing_step4_size();
    uint8_t encrypt_step4_bytes[encrypt_step4_bytes_len];
    uint32_t encrypt_step4_writen_len = 0;
    ig_pairing_step4(
      step3_payload_bytes, step3_len, 
      encrypt_step4_bytes, encrypt_step4_bytes_len, &encrypt_step4_writen_len);
    if (encrypt_step4_writen_len == UINT32_MAX)
    {
      serverLog(LL_ERROR, "ig_pairing_step4 error");
      return 1;
    }
    // serverLog(LL_NOTICE, "ig_pairing_step4 success");

    // 建立step4 完成, 然后就要发送除去
    uint32_t payload_len = 0;
    uint8_t *payloadBytes = NULL;
    if (!build_msg_payload(
      &payloadBytes, &payload_len, 
      encrypt_step4_bytes, encrypt_step4_bytes_len))
    {
      serverLog(LL_ERROR, "failed in build_msg_payload");
      return 1;
    }
    if (fake_transmit_payloadBytes) free(fake_transmit_payloadBytes);
    fake_transmit_payloadBytes = malloc(payload_len);
    memcpy(fake_transmit_payloadBytes, encrypt_step4_bytes, payload_len);
    fake_transmit_payloadBytes_len = payload_len;

    if (step3_payload_bytes) free(step3_payload_bytes);
    step3_payload_bytes = NULL;
    if (payloadBytes) free(payloadBytes);
    payloadBytes = NULL;
  }
  return 0;
}

// client 接收到了 step4
int handleReplyStep4(void *arg)
{
  serverLog(LL_NOTICE, "handleReplyStep4 ----------------");
  int ret = 0;
  uint16_t step4_len = 0;
  uint16_t n_size_byte = 0;
  uint16_t pkg_len = 0;
  pkg_len = getDataLength(
    fake_transmit_payloadBytes, &n_size_byte, &step4_len);
  if (pkg_len == 0)
  {
    serverLog(LL_ERROR, "getRecvPkgLen error");
    return 1;
  }
  serverLog(LL_NOTICE, "step4_len: %u, n_size_byte %u, pkg_len %u", step4_len, n_size_byte, pkg_len);

  uint8_t *encrypt_step4_bytes = NULL;
  size_t encrypt_step4_bytes_len = 0;

  encrypt_step4_bytes_len = igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native(
		step4_len, 
    fake_transmit_payloadBytes+n_size_byte, 
    &encrypt_step4_bytes
	);
  if (!encrypt_step4_bytes_len)
	{
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native error");
    return 1;
  }

  

  if (encrypt_step4_bytes) free(encrypt_step4_bytes);
  encrypt_step4_bytes = NULL;
}

int handleWriteCommit(void *arg)
{
  
}

int handlePairingComplete(void *arg)
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

  //这儿只显示 服务器的状态, 那么, 这儿, 就只会有

  // 1 事件, 从 PAIRING_BEGIN -> PAIRING_STEP2
  // C_WRITE_STEP1, from PAIRING_BEGIN to  PAIRING_STEP2
  // handleWriteStep1: client 写入一个 step1, 那么就会有把状态从 PAIRING_BEGIN -> PAIRING_STEP2,
  // 因为接收到了之后, 就会直接返回一个pairing step2, 所以是到pairing step2
  if (fillTransItem(&trans_item, 
    PAIRING_STEP1, PAIRING_BEGIN, handleWriteStep1, PAIRING_STEP2))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  // serverLog(LL_NOTICE, "fillTransItem event C_WRITE_STEP1");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }
  // serverLog(LL_NOTICE, "fillFSMTransItem event C_WRITE_STEP1");

  // 2 事件, 从 PAIRING_STEP2 到 PAIRING_STEP4  
  // 因为会我们会返回 step3 给客户端
  // C_WRITE_STEP3, from PAIRING_STEP2 to PAIRING_STEP3
  // handleWriteStep3: client 写入一个 step3, 那么就会有把状态从 PAIRING_STEP2 -> PAIRING_STEP4,
  // 因为服务器会发送一个step3, 返回给客户一个step4,然后等待commit
  if (fillTransItem(&trans_item, 
    C_WRITE_STEP3, PAIRING_STEP2, handleWriteStep3, PAIRING_COMMIT))
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

  // 3
  // S_REPLY_STEP4, from PAIRING_STEP4 to PAIRING_STEP4
  // handleWriteCommit: 客户接收到 到一个 Commit 消息, 然后会返回一个
  if (fillTransItem(&trans_item, 
    S_REPLY_STEP4, PAIRING_COMMIT, handleWriteCommit, PAIRING_COMPLETE))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  // serverLog(LL_NOTICE, "fillTransItem event S_REPLY_STEP4");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }
  // serverLog(LL_NOTICE, "fillFSMTransItem event S_REPLY_STEP4");

  // 4
  // S_PAIRING_COMPLETE, from PAIRING_COMPLETE to PAIRING_BEGIN
  if (fillTransItem(&trans_item, 
    S_PAIRING_COMPLETE, PAIRING_COMPLETE, handlePairingComplete, PAIRING_BEGIN))
  {
    serverLog(LL_ERROR, "fillTransItem err");
    return 1;
  }
  // serverLog(LL_NOTICE, "fillTransItem event C_WRITE_COMMIT");

  if (fillFSMTransItem(fsm, &trans_item))
  {
    serverLog(LL_ERROR, "fillFSMTransItem err");
    return 1;
  }
  // serverLog(LL_NOTICE, "fillFSMTransItem event C_WRITE_COMMIT");

  // 初始化fsm
  initFSMCurState(fsm, PAIRING_BEGIN);

  serverLog(LL_NOTICE, "fsm state : %d", fsm->cur_state);

  if (getRecvData(&recv_pairing_data))
  {
    serverLog(LL_ERROR, "getRecvData err");
    return 1;
  }
  // serverLog(LL_NOTICE, "getRecvData success, need to free");
  if (initRecvData(recv_pairing_data))
  {
    serverLog(LL_ERROR, "initRecvData err");
    return 1;
  }
  serverLog(LL_NOTICE, "initRecvData success");


  generateStep1AndSend();

  // client 开始写数据进来, 我需要用 handle write step1 来接收和处理
  event = C_WRITE_STEP1;
  handleEvent(fsm, event, recv_pairing_data);
  // 证明是 调到了状态2
  serverLog(LL_NOTICE, "fsm state: %d", fsm->cur_state);

  // client 处理
  handleReplyStep2(recv_pairing_data);
  
  // client 会发送step3, 
  event = C_WRITE_STEP3;
  handleEvent(fsm, event, recv_pairing_data);
  serverLog(LL_NOTICE, "fsm state: %d", fsm->cur_state);
  // serverLog(LL_NOTICE, "recv_pairing_data->n_size_byte: %u", recv_pairing_data->n_size_byte);

  // 到这儿, server生成了 step4 给client, 然后client要接收
  handleReplyStep4(argv);

  // // 打印复制的包, 肉眼可以看到是拷贝对了
  // serverLog(LL_NOTICE, "printf copy data");
  // for (int i = 0; i < payload_len; i++)
  // {
  //   if (i%ble_pkg_limit==0 && i != 0)
  //   {
  //     printf("  i: %d \n", i);
  //   }
  //   printf("%x ", recv_pairing_data->data[i]);
    
  // }
  // printf("\n");
  // serverLog(LL_NOTICE, "before : %d", fsm->cur_state);
  // transState(fsm, PAIRING_STEP1);
  // serverLog(LL_NOTICE, "after state : %d", fsm->cur_state );
  // // ------------------- 上面是接收 step1 的包的函数的验证 ----------------------

  // // ------------------- 下面是接收 服务端开始处理 发送的step1 的包的函数的验证 ----------------------
  // // 需要解开第一个包, 然后
  // size_t step1_len = 0;
  // if (getRecvPkgLen(recv_pairing_data, &step1_len))
  // {
  //   serverLog(LL_ERROR, "getRecvPkgLen error");
  //   return 1;
  // }
  // serverLog(LL_NOTICE, "getRecvPkgLen success, size %u", step1_len);

  // uint8_t *step1_payload_bytes = malloc(step1_len);
  // memset(step1_payload_bytes, 0, step1_len);
  // if (getPkgFromRecvData(recv_pairing_data, step1_payload_bytes))
  // {
  //   serverLog(LL_ERROR, "getPkgFromRecvData error");
  //   return 1;
  // }
  // serverLog(LL_NOTICE, "getPkgFromRecvData success");

  // // 打印一下拷贝, 确定确实拷贝对了:
  // for (int i = 0; i < step1_len; i++)
  // {
  //   if (i%ble_pkg_limit==0 && i != 0)
  //   {
  //     printf("  i: %d \n", i);
  //   }
  //   printf("%x ", step1_payload_bytes[i]);
  // }
  // printf("\n");

  // // 解压数据包传过来的
  // // 假设write_pairing_step1过来了
  // event = C_WRITE_STEP1;
  // handleEvent(fsm, event, recv_pairing_data);
  // freeRecvData(&recv_pairing_data);

  // transState(fsm, );
  // // ------------------- 上面是接收 服务端开始处理 收到的step1 的包的函数的验证 ----------------------

  // // ------------------- 下面是客户 接收到服务器的payload 收到的step2 的包的函数的验证 ----------------------
  // // 假设已经接收到所有数据, 因为 已经验证过接收数据了
  // handleReplyStep2(NULL);
  // handleEvent(fsm, event, recv_pairing_data);
  // // ------------------- 下面是服务器 接收到客户端的 step3Bytes 函数的验证 ----------------------
  // // 客户端 设置成为 step3 

  // if (getRecvData(&recv_pairing_data))
  // {
  //   serverLog(LL_ERROR, "step3 getRecvData err");
  //   return 1;
  // }
  // serverLog(LL_NOTICE, "getRecvData success, need to free");
  // if (initRecvData(recv_pairing_data))
  // {
  //   serverLog(LL_ERROR, "initRecvData err");
  //   return 1;
  // }
  // serverLog(LL_NOTICE, "initRecvData success");

  // for (int i = 0 ; i < payload_len; )
  // {
  //   printf("receiving data %.3f\n", (double)i/payload_len);
  //   int copy_len = (payload_len-i-ble_pkg_limit>0?ble_pkg_limit:payload_len-i);
  //   memset(ble_pkg, 0, sizeof(ble_pkg));
  //   memcpy(ble_pkg, payloadBytes+i, copy_len);
  //   // 输出每次的包, 用来验证是否全部输出
  //   for (int j = 0; j < copy_len; j++)
  //   {
  //     printf("%x ", ble_pkg[j]);
  //   }
  //   printf("\n");

  //   // 这里验证开始接收到的pkg
  //   serverLog(LL_NOTICE, "before copy status: %d", recv_pairing_data->recv_status);
  //   recvData(recv_pairing_data, ble_pkg, copy_len);
  //   serverLog(LL_NOTICE, "before copy status: %d", recv_pairing_data->recv_status);

  //   // 验证是否拷贝对了
  //   for(int j = 0; j < copy_len; j++)
  //   {
  //     if ((recv_pairing_data->data[i+j]) != ble_pkg[j])
  //     {
  //       serverLog(LL_ERROR, "recvData copy error");
  //       return 1;
  //     }
  //   }

  //   if (copy_len != ble_pkg_limit)
  //   {
  //     printf("end receiving data %.3f\n", (double)(i+copy_len)/payload_len);
  //   }
  //   i += copy_len;
  // }
  // serverLog(LL_NOTICE, "----------- test step3 -----------------");
  // // size_t step3_len = 0;
  // // if (getRecvPkgLen(recv_pairing_data, &step3_len))
  // // {
  // //   serverLog(LL_ERROR, "getRecvPkgLen error");
  // //   return 1;
  // // }
  // // serverLog(LL_NOTICE, "getRecvPkgLen success, step3_len size %u", step3_len);
  // // uint8_t *step3_payload_bytes = malloc(step3_len);
  // // memset(step3_payload_bytes, 0, step3_len);
  // // if (getPkgFromRecvData(recv_pairing_data, step3_payload_bytes))
  // // {
  // //   serverLog(LL_ERROR, "step3_payload_bytes getPkgFromRecvData error");
  // //   return 1;
  // // }
  // // serverLog(LL_NOTICE, "step3_payload_bytes getPkgFromRecvData success");


  // event = C_WRITE_STEP3;
  // handleEvent(fsm, event, recv_pairing_data);
  // freeRecvData(&recv_pairing_data);

  // fake_transmit_payloadBytes
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