#include "bridge/bridge_main/log.h"
#include "bridge/lock/messages/PairingStep1.h"
#include "bridge/lock/messages/PairingStep2.h"
#include "bridge/lock/messages/PairingStep3.h"
#include "bridge/lock/messages/PairingStep4.h"
#include "bridge/lock/messages/PairingCommit.h"
#include "bridge/lock/connection/pairing_connection.h"
#include "bridge/gattlib/gattlib.h"
#include "bridge/ble/ble_operation.h"
#include "bridge/wifi_service/pairing.h"
#include <glib.h>

static char wifi_pairing_str[] = "12345678-0000-1000-8000-00805f9b34fb";
uuid_t wifi_pairing_uuid = {};

gatt_connection_t* gatt_connection = NULL;
char *bridge_addr = "DC:A6:32:10:C7:DC";
bool is_registered = false;

GMainLoop *loop = NULL;

PAIRING_STATUS pairing_status;

void handleStep4(const uint8_t* data, size_t data_length)
{
  serverLog(LL_NOTICE, "---------------- handleStep4: ");
  int ret = 0;
  

  uint8_t *step4Bytes = NULL;
	size_t step4_size = 0;
  IgPairingStep4 *step4;

  size_t n_size_byte = 0;
  if (data[2] == 0xff)
  {
    n_size_byte = 3;
  }
  else
  {
    n_size_byte = 1;
  }

  step4_size = igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native(
		(data_length-n_size_byte), (uint8_t *)(data+n_size_byte), &(step4Bytes)
	);

  if (!step4_size)
	{
    serverLog(LL_ERROR, "igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native error");
    return;
  }

  step4 = (IgPairingStep4 *)step4Bytes;

  serverLog(LL_NOTICE, "debug show step4 password: ");

  if (step4->has_success && step4->success)
	{
    serverLog(LL_NOTICE, "set Pairing Result the Password");
    if (step4->has_password)
    {
        printf("step4.has_password: ");
        for (int j = 0; j < step4->password_size;j++)
        {
            printf("%02x ", (step4->password)[j]);
        }
        printf("\n");
    }
	}

  size_t commitBytes_len = 0;
	uint8_t *commitBytes = NULL;
  time_t current_time = time(NULL);

  commitBytes_len = igloohome_ble_lock_crypto_PairingConnection_genPairingCommitNative(
    current_time, &commitBytes);
  if (!commitBytes_len)
	{
    serverLog(LL_ERROR, "genPairingCommitNative failed!");
		return;
	}
  serverLog(LL_NOTICE, "genPairingCommitNative success");


  size_t payload_len = 0;
	uint8_t *payloadBytes = NULL;

  if (!build_msg_payload(
		&payloadBytes, &payload_len, commitBytes, commitBytes_len))
	{
    serverLog(LL_ERROR, "build_msg_payload failed!");
		return;
	}
  serverLog(LL_NOTICE, "build_msg_payload success!");

  ret = write_char_by_uuid_multi_atts(
		gatt_connection, &wifi_pairing_uuid, 
    payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed!");
		return;
	}
  serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts success!");

  if (step4Bytes)
  {
    free(step4Bytes);
    step4Bytes = NULL;
  }

  if (commitBytes)
  {
    free(commitBytes);
    commitBytes = NULL;
  }

  if (payloadBytes)
  {
    free(payloadBytes);
    payloadBytes = NULL;
  }
  pairing_status = PAIRING_COMMIT;
  return;
}

void handleStep2(const uint8_t* data, size_t data_length)
{
  serverLog(LL_NOTICE, "---------------- handleStep4: ");
  int ret = 0;
  size_t payload_len = 0;
	uint8_t *payloadBytes = NULL;
  uint8_t *step3Bytes = NULL;
  size_t step3Bytes_len = 0;

  size_t n_size_byte = 0;
  if (data[2] == 0xff)
  {
    n_size_byte = 3;
  }
  else
  {
    n_size_byte = 1;
  }

  step3Bytes_len = 
      igloohome_ble_lock_crypto_PairingConnection_genPairingStep3Native(
		        (data_length-n_size_byte), (uint8_t *)(data+n_size_byte), &step3Bytes);
  if (!step3Bytes_len)
  {
    serverLog(LL_NOTICE, "genPairingStep3Native error");
    return;
  }
  serverLog(LL_NOTICE, "genPairingStep3Native Success");

  if (!build_msg_payload(
		&payloadBytes, &payload_len, step3Bytes, step3Bytes_len))
	{
    serverLog(LL_ERROR, "failed in build_msg_payload");
		return;
	}
  serverLog(LL_NOTICE, "build_msg_payload Success");

  ret = write_char_by_uuid_multi_atts(
		gatt_connection, &wifi_pairing_uuid,payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, 
                "write_char_by_uuid_multi_atts failed in writing th packags");
		return;
	}
  serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts Success");

  pairing_status = PAIRING_STEP3;
  return;
}

static void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data) {
  serverLog(LL_NOTICE, "----------- message_handler ---------------");
  printf("data_length: %u", data_length);
  for (int i = 0; i < data_length; ++i)
  {
    printf(" %x", data[i]);
  }
  printf("\n");
  switch (pairing_status)
  {
  case PAIRING_STEP1:
    handleStep2(data, data_length);
    break;
  case PAIRING_STEP3:
    handleStep4(data, data_length);
    break;
  case PAIRING_COMMIT:
    break;
  default:
    serverLog(LL_ERROR, "error pairing_status");
    break;
  }
  
}

int register_pairing_notfication()
{
  serverLog(LL_NOTICE, "-------- register_pairing_notfication start --------");
  int ret;
  gatt_connection = gattlib_connect(
    NULL, bridge_addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
  if (gatt_connection == NULL) {
		serverLog(LL_ERROR, "Fail to connect to the bluetooth device." );
		return 1;
	} 
  serverLog(LL_NOTICE, "Succeeded to connect to the bluetooth device." );

  if (
    gattlib_string_to_uuid(
      wifi_pairing_str, strlen(wifi_pairing_str), &(wifi_pairing_uuid))<0)
  {
    serverLog(LL_ERROR, "gattlib_string_to_uuid to pairing_uuid fail");
  }
  serverLog(LL_NOTICE, "gattlib_string_to_uuid to pairing_uuid success." );

  // 注册, 并且 start 了
  gattlib_register_notification(
    gatt_connection, message_handler, NULL);
  ret = gattlib_notification_start(
      gatt_connection, &wifi_pairing_uuid);
  if (ret) {
    serverLog(LL_ERROR, "Fail to start notification.");
		return 1;
	}
  is_registered = true;
  serverLog(LL_NOTICE, "success to start notification" );

} 

void endGattConnection()
{
  int ret= 0;
  if (is_registered && gatt_connection)
  {
    serverLog(LL_NOTICE, "endConnection gattlib_notification_stop");
    ret = gattlib_notification_stop(gatt_connection, &wifi_pairing_uuid);
    if (ret != GATTLIB_SUCCESS)
    {
      serverLog(LL_ERROR, "endConnection gattlib_notification_stop error");
      return;
    }
    serverLog(LL_NOTICE, "endConnection gattlib_disconnect");
  }

  if(gatt_connection)
  {
    ret = gattlib_disconnect(gatt_connection);
    if (ret != GATTLIB_SUCCESS)
    {
      serverLog(LL_ERROR, "endConnection gattlib_disconnect error");
      return;
    }
    gatt_connection = NULL;
  }

}

int write_pairing_step1()
{
  serverLog(LL_NOTICE, "write_pairing_step1 start --------");
  int ret;
  size_t step1Bytes_len, payload_len;
  uint8_t *step1Bytes = NULL;
  uint8_t *payloadBytes = NULL;
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

  ret = write_char_by_uuid_multi_atts(
    gatt_connection, &wifi_pairing_uuid, 
    payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
    serverLog(LL_ERROR, "write_char_by_uuid_multi_atts failed in writing th packags");
		return 1;
	}
  serverLog(LL_NOTICE, 
    "write_char_by_uuid_multi_atts success in writing packages, len %u ", payload_len);
  printf("-------------------- payloadBytes: \n");
  for (int i = 0; i < payload_len; ++i)
  {
    printf(" %x", payloadBytes[i]);
  }
  printf("\n");
  if (step1Bytes) 
  {
    free(step1Bytes);
    step1Bytes = NULL;
  }

  if (payloadBytes)
  {
    free(payloadBytes);
    payloadBytes =  NULL;
  }
  pairing_status = PAIRING_STEP1;
  return 0;
}

// 没有这一步, 就不会收到回复
int waiting_pairing_step2()
{
  serverLog(LL_NOTICE, "waiting_pairing_step2");
  // task_node_t *task_node = (task_node_t *)arg;

  // // 在这儿用g_main_loop_run等待, 用线程锁和睡眠的方法不行, 就像是bluez不会调用
  // // 我的回调函数, 在 rtos 应该会有相应的方法实现这样的等待事件到来的方法.
  // // 当前 Linux 下, 这样用, works 
  // serverLog(LL_NOTICE, "waiting_pairing_step2 new loop waiting");
  loop = g_main_loop_new(NULL, 0);
  // task_node->loop = g_main_loop_new(NULL, 0);
  g_main_loop_run(loop);
  // serverLog(LL_NOTICE, "waiting_pairing_step2 exit task_node->loop");
  // return 0;
}
// 对方一定要advertise, 否则不会收得到得.

int main(int argc, char *argv[])
{
  // 先注册
  if (register_pairing_notfication(NULL))
  {
    serverLog(LL_ERROR, "register_pairing_notfication error");

  }

  // 写第一步阿
  write_pairing_step1();
  waiting_pairing_step2();
  endGattConnection();
}