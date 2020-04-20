#include "bridge_function/test_bridge.h"

#include "lock/messages/PairingStep1.h"
#include "lock/messages/PairingStep2.h"
#include "lock/messages/PairingStep3.h"
#include "lock/messages/PairingStep4.h"
#include "lock/messages/PairingCommit.h"

static void on_user_abort(int arg) 
{
	g_main_loop_quit(m_main_loop);
}

void connection_init(connection_t *connection)
{
	memset((void *)(connection), 0, sizeof(connection_t));
	return;
}

void admin_connection_init(admin_connection_t *admin_connection)
{
	memset((void *)(admin_connection), 0, sizeof(admin_connection_t));
	return;
}

// ----------------------- common start ---------

size_t resolve_payload_len(size_t step_len)
{
	return step_len>254 ? (step_len+3) : (step_len+1);
}

bool build_msg_payload(uint8_t **p_payloadBytes, 
  size_t &payload_len, uint8_t *stepBytes, size_t step_len)
{
	payload_len = resolve_payload_len(step_len);
	size_t n_byte_for_len = payload_len - step_len;
	(*p_payloadBytes) = (uint8_t *)malloc(payload_len);
	if (n_byte_for_len == 1)
	{
		(*p_payloadBytes)[0] = step_len;
	}
	else if (n_byte_for_len == 3)
	{
		// 大端存储
		uint8_t fst, sec;
		sec = 254;
		fst = step_len - sec;
		(*p_payloadBytes)[0] = fst;
		(*p_payloadBytes)[1] = sec;
		(*p_payloadBytes)[3] = 0xff;  
	}
	else
	{
		cerr << "wrong n_byte_for_len, error" << endl;
		return false;
	}
	memcpy((*p_payloadBytes)+ n_byte_for_len, stepBytes, step_len);
	return true;
}

int write_char_by_uuid_multi_atts (
	gatt_connection_t* gatt_connection, uuid_t* uuid, const uint8_t* buffer, 
	size_t buffer_len, size_t BLE_ATT_MAX_BYTES)
{
	int ret = GATTLIB_SUCCESS;
	uint8_t *tmp_bytes[BLE_ATT_MAX_BYTES];
	size_t size_left;
	int i;
	for (i = 0; i < buffer_len; i += BLE_ATT_MAX_BYTES)
	{
		size_left = buffer_len-i>=BLE_ATT_MAX_BYTES?BLE_ATT_MAX_BYTES:buffer_len-i;
		memset(tmp_bytes, 0, BLE_ATT_MAX_BYTES);
		memcpy(tmp_bytes, buffer+i, size_left);
		ret = gattlib_write_char_by_uuid(
			gatt_connection, uuid, tmp_bytes, size_left);
		if (ret != GATTLIB_SUCCESS) {
			cerr << "write_char_by_uuid_multi_atts: gattlib_write_char_by_uuid failed in writint "<<  (i/BLE_ATT_MAX_BYTES) << "th packags" << endl;
			return ret;
		}
		else
		{
			cout << "write_char_by_uuid_multi_atts: gattlib_write_char_by_uuid success in writing " << (i/BLE_ATT_MAX_BYTES) << "th packages" << endl;
		}
	}
	return ret;
}


int save_message_data(
	const uint8_t* data, size_t data_length,
  size_t &step_max_size, size_t &step_cur_size, uint8_t *&step_data,
  size_t &n_size_byte)
{
	// 如果没有分配空间, 就分配一个大的空间
	if (step_max_size == 0)
	{
		cout << "step_max_size not set" << endl;
		if (data_length<3)
		{
			cerr << "data_length < 3, can't get len" << endl; 
			return 2;
		}
		else
		{
			if (data[2] == 0xff)
			{
				n_size_byte = 3;
				step_max_size = data[0] * (0xfe) + data[1] + 3;
				cout << "2 bytes lenth " << step_max_size << endl;
			}
			else
			{
				n_size_byte = 1;
				step_max_size = data[0] + 1; 
				cout << "1 bytes lenth " << step_max_size << endl;
			}
			step_cur_size = 0;
			step_data = (uint8_t *)malloc(step_max_size);
			if (!step_data)
			{
				cerr << "Message_handler malloc err" << endl;
				return 3;
			}
		}
	}

	// 判断还有多少的空间,要接收
	size_t size_left = step_max_size - step_cur_size;
	
	// 如果还剩下的空间,不足够庄下, 那么就重新申请一块大一点的.
	if (size_left < data_length)
	{
		cout << "size_left < data_length" << endl;
		step_max_size = step_max_size + DEFAULT_STEP2_DATA_SIZE;
		uint8_t *old_data = step_data;
		step_data = (uint8_t *)malloc(step_max_size);
		if (!step_data)
		{
			cerr << "Message_handler malloc err" << endl;
			return 4;
		}
		memcpy(step_data, old_data, step_cur_size);
		free(old_data);
	}

	// 空间足够, 直接放下空间里面
	for (int i = 0; i < data_length; ) {
		// printf("%02x ", data[i]);
		step_data[step_cur_size++] = data[i++];
	}
	// printf("\n");

	return 0;
}

// ----------------------- common end ---------

// ----------------------- paired start ---------

void *ble_write_pairing_commit(void *arg)
{
	int ret, i;
	connection_t *connection = (connection_t *)arg;
	gatt_connection_t* gatt_connection = connection->gatt_connection;
	char* addr = (connection->lock).addr;
	PAIRING_STEP &pairing_step = connection->pairing_step;
	std::mutex &pairing_step_mutex = connection->pairing_step_mutex;
	size_t &step4_max_size = connection->step4_max_size;
	size_t &step4_cur_size = connection->step4_cur_size;
	uint8_t *&step4_data = connection->step4_data;
	size_t &n_size_byte = connection->n_size_byte;
	size_t payload_len = 0;
	uint8_t *payloadBytes = NULL;
	uint8_t *commitBytes;
	int commitBytes_len;
	uint8_t *step4;

	cout << "igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native" << endl;
	ret = igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native(
		(step4_max_size-n_size_byte), (step4_data+n_size_byte), &(step4)
	);
	if (!ret)
	{
		cerr << "igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native error" << endl;
	}
	cout << "igloohome_ble_lock_crypto_PairingConnection_recPairingStep4Native success" << endl;
	
	time_t current_time = time(NULL);
	commitBytes_len = igloohome_ble_lock_crypto_PairingConnection_genPairingCommitNative(
    current_time, &commitBytes);
	if (!commitBytes_len)
	{
		cerr << "igloohome_ble_lock_crypto_PairingConnection_genPairingCommitNative failed!" << endl;
	}
	cerr << "igloohome_ble_lock_crypto_PairingConnection_genPairingCommitNative success!" << endl;

	if (!build_msg_payload(
		&payloadBytes, payload_len, commitBytes, commitBytes_len))
	{
		cerr << "failed in build_msg_payload" << endl;
		goto COMMIT_EXIT;
	}

	ret = write_char_by_uuid_multi_atts(
		gatt_connection, &pairing_uuid, payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
		cerr << "write_char_by_uuid_multi_atts failed in writing th packags" << endl;
		goto COMMIT_EXIT;
	}
	else
	{
		cout << "write_char_by_uuid_multi_atts success in writing packages" << endl;
		// std::lock_guard<std::mutex> lock(pairing_step_mutex);
		pairing_step = PairingStepDone;
		// addr : D9:78:2F:E3:1A:5C
		// name : IGM303e31a5c
		// 4b 9d f ed 0 3 59 39 c0 c4 86 c5 fa 88 49 1b
		cout << "addr : " << (connection->lock).addr << endl;
		cout << "name : " << (connection->lock).name << endl;
		uint8_t *admin_key;
		int key_len = igloohome_ble_lock_crypto_PairingConnection_getAdminKeyNative(
			time(NULL), &admin_key);		
		sp_lock_list->push_back(
			paired_igm_lock_t(connection->lock, true, admin_key, key_len));
		cout << "admin key: " << endl;
		for (int j = 0; j < key_len; j++)
		{
			printf("%02x ", admin_key[j]);
		}
		printf("\n");
		if (admin_key)
		{
			free(admin_key);
		}
		cout << "g_main_loop_quit connection->properties_changes_loop" << endl;
		g_main_loop_quit(connection->properties_changes_loop);
		cout << "g_main_loop_quit connection->properties_changes_loop end" << endl;
	}

COMMIT_EXIT:
	free(commitBytes);
	free(payloadBytes);
	return NULL;
}

void *ble_write_step3(void *arg)
{
	int ret, i;
	connection_t *connection = (connection_t *)arg;
	gatt_connection_t* gatt_connection = connection->gatt_connection;
	char* addr = (connection->lock).addr;
	PAIRING_STEP &pairing_step = connection->pairing_step;
	std::mutex &pair_complete_mutex = connection->pairing_step_mutex;
	size_t &step2_max_size = connection->step2_max_size;
	size_t &step2_cur_size = connection->step2_cur_size;
	uint8_t *&step2_data = connection->step2_data;
	size_t &n_size_byte = connection->n_size_byte;
	size_t payload_len = 0;
	uint8_t *payloadBytes = NULL;
	uint8_t *step3Bytes;
	int step3Bytes_len;
	cout << "------------------------------debug line-------------------" << endl;
	cout << "===========================ble_write_step3" << endl;
	step3Bytes_len = 
		igloohome_ble_lock_crypto_PairingConnection_genPairingStep3Native(
		(step2_max_size-n_size_byte), (step2_data+n_size_byte), &step3Bytes
	);
	if (!step3Bytes_len)
	{
		cout << "step3 generate ok error" << endl;
		return NULL;
	}
	cout << "step3 generate ok" << endl;

	if (!build_msg_payload(
		&payloadBytes, payload_len, step3Bytes, step3Bytes_len))
	{
		cerr << "failed in build_msg_payload" << endl;
		goto STEP3_EXIT;
	}
	ret = write_char_by_uuid_multi_atts(
		gatt_connection, &pairing_uuid, payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
		cerr << "write_char_by_uuid_multi_atts failed in writing th packags" << endl;
		goto STEP3_EXIT;
	}
	else
	{
		cout << "write_char_by_uuid_multi_atts success in writing packages" << endl;
		// std::lock_guard<std::mutex> lock(pairing_step_mutex);
		pairing_step = PairingStep3;
	}

STEP3_EXIT:
	free(step3Bytes);
	free(payloadBytes);
	return NULL;
}

int handle_step2_message(
	connection_t *connection, const uint8_t* data, size_t data_length,
  size_t &step2_max_size, size_t &step2_cur_size, uint8_t *&step2_data,
  size_t &n_size_byte, PAIRING_STEP &pairing_step, 
  std::mutex &pairing_step_mutex)
{
	cout << "------------------------------debug line-------------------" << endl;
	cout << "----------------handle_step2_message" << endl;
	if (pairing_step != PairingStep1)
	{
		cerr << "handle_step2_message pairing_step != PairingStep1, error" << endl;
		return 1;
	}
	// save message data
	save_message_data(
		data, data_length, step2_max_size, step2_cur_size, step2_data, 
		n_size_byte);
	if (step2_max_size == step2_cur_size)
	{
		int ret;
		cout << "RECV step2 data finished" << endl;
		cout << "pairing_step: " << pairing_step << endl;
		// cout << "reset step2 data" << endl;
		// step2_cur_size = 0;
		// step2_max_size = 0;
		// // std::lock_guard<std::mutex> lock(pairing_step_mutex);
		// if (step2_data)
		// {
		// 	free(step2_data);
		// }
		// n_size_byte = 0;
		cout << "set pairing_step to PairingStep2" << endl;
		pairing_step = PairingStep2;
		cout << "Create thread to write step3" << endl;
		ret = pthread_create(&connection->write_thread, NULL,	ble_write_step3, connection);
		if (ret != 0) {
			cerr <<  "Failt to create BLE write_thread thread." << endl;
			free(connection);
			return 5;
		}
		ret = pthread_detach(connection->write_thread);
		if (ret != 0) {
			cerr <<  "Failt to detach BLE write_thread thread." << endl;
			free(connection);
			return 6;
		}
	}
	return 0;
}

int handle_step4_message(
	connection_t *connection, const uint8_t* data, size_t data_length,
  size_t &step4_max_size, size_t &step4_cur_size, uint8_t *&step4_data,
  size_t &n_size_byte, PAIRING_STEP &pairing_step, 
  std::mutex &pairing_step_mutex)
{
	cout << "------------------------------debug line-------------------" << endl;
	cout << "----------------handle_step4_message" << endl;
	if (pairing_step != PairingStep3)
	{
		cerr << "handle_step4_message pairing_step != PairingStep3, error" << endl;
		return 1;
	}

	save_message_data(
		data, data_length, step4_max_size, step4_cur_size, step4_data, 
		n_size_byte);

	if (step4_max_size == step4_cur_size)
	{
		int ret;
		cout << "RECV step4 data finished" << endl;
		cout << "pairing_step: " << pairing_step << endl;

		cout << "set pairing_step to PairingStep4" << endl;
		pairing_step = PairingStep4;

		cout << "Create thread to write step5, message E to commit pairing" << endl;
		ret = pthread_create(
			&connection->write_thread, NULL,	ble_write_pairing_commit, connection);
		if (ret != 0) {
			cerr <<  "Failt to create BLE ble_write_pairing_commit thread." << endl;
			free(connection);
			return 5;
		}
		cout << "Success to create BLE ble_write_pairing_commit thread." << endl;

		ret = pthread_detach(connection->write_thread);
		if (ret != 0) {
			cerr <<  "Failt to detach BLE ble_write_pairing_commit thread." << endl;
			free(connection);
			return 6;
		}
		cout << "Success to detach BLE ble_write_pairing_commit thread." << endl;
	}

	return 0;
}

void message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data) {
	
	int i;
	connection_t *connection = (connection_t *)user_data;
	char *addr = (connection->lock).addr;

	cout << "------------------------------debug line-------------------" << endl;
	cout << "------------message_handler" << endl;
	printf("message from ADDR %s\n", addr);
	printf("message Handler: data_length %d \n", data_length);
	printf("connection->pairing_step %d\n", connection->pairing_step);
	switch (connection->pairing_step)
	{
	case PairingStep1:
	{
		cout << "message_handler handle_step2_message " << endl;
		int ret = handle_step2_message(
			connection, data, data_length,
			connection->step2_max_size, connection->step2_cur_size, 
			connection->step2_data, connection->n_size_byte, 
			connection->pairing_step, connection->pairing_step_mutex);
		break;
	}
	case PairingStep3:
	{
		int ret = handle_step4_message(
			connection, data, data_length,
			connection->step4_max_size, connection->step4_cur_size, 
			connection->step4_data, connection->n_size_byte, 
			connection->pairing_step, connection->pairing_step_mutex);
		break;
	}
	default:
	{	
		cerr << "PairingStep3 wrong in pairing message handler" << endl;
		break;
	}
	}
	return;	
}

void *ble_write_step1(void *arg)
{
	int ret, i, size_left;
	connection_t *connection = (connection_t *)arg;
	gatt_connection_t* gatt_connection = connection->gatt_connection;
	char* addr = (connection->lock).addr;
	size_t step1Bytes_len, payload_len;
	PAIRING_STEP &pairing_step = connection->pairing_step;
	std::mutex &pairing_step_mutex = connection->pairing_step_mutex;
	size_t &step2_max_size = connection->step2_max_size;
	size_t &step2_cur_size = connection->step2_cur_size;
	uint8_t *&step2_data = connection->step2_data;

  uint8_t *step1Bytes = NULL;
	uint8_t *payloadBytes = NULL;

	cout << "------------------------------debug line-------------------" << endl;
	printf("ble_write_step1 to ADDR %s", addr);

	ret = igloohome_ble_lock_crypto_PairingConnection_beginConnection();
  if (ret)
  {
    printf("beginConnection success\n");
  }
  else
  {
    printf("beginConnection fail\n");
		return NULL;
  }

	step1Bytes_len = 
		igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native(
			&step1Bytes);
	
  if (step1Bytes == NULL)
  {
    printf("igloohome_ble_lock_crypto_PairingConnection_genPairingStep1Native fail\n");
    return NULL;
  }
	// failed in build_msg_payload
	if (!build_msg_payload(&payloadBytes, payload_len, step1Bytes, step1Bytes_len))
	{
		cerr << "failed in build_msg_payload" << endl;
		goto STEP1_EXIT;
	}
	ret = write_char_by_uuid_multi_atts(gatt_connection, &pairing_uuid, payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
		cerr << "write_char_by_uuid_multi_atts failed in writing th packags" << endl;
		goto STEP1_EXIT;
	}
	else
	{
		cout << "write_char_by_uuid_multi_atts success in writing packages" << endl;
		// std::lock_guard<std::mutex> lock(pairing_step_mutex);
		pairing_step = PairingStep1;
	}
	
STEP1_EXIT:
	free(step1Bytes);
	free(payloadBytes);
	return NULL;
}

// 由 discovered 来启动, 暂时
void *ble_pair_device(void *arg) {
	int step1Bytes_len;
  uint8_t *step1Bytes = NULL;
	int size_left;
	connection_t *connection = (connection_t *)arg;
	char* addr = (connection->lock).addr;
	PAIRING_STEP &pairing_step = connection->pairing_step;
	std::mutex &pairing_step_mutex = connection->pairing_step_mutex;
	std::mutex &pair_complete_mutex = connection->pair_complete_mutex;
	std::condition_variable &pair_complete = connection->pair_complete;
	gatt_connection_t* gatt_connection;
	gattlib_primary_service_t* services;
	gattlib_characteristic_t* characteristics;
	int services_count, characteristics_count;
	char uuid_str[MAX_LEN_UUID_STR + 1];
	int ret, i;
	GMainLoop *loop;

	gatt_connection = gattlib_connect(NULL, addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
	connection->gatt_connection = gatt_connection;
	if (gatt_connection == NULL) {
		cerr << "Fail to connect to the bluetooth device." << endl;
		goto CONNECTION_EXIT;
	} else {
		cout << "Succeeded to connect to the bluetooth device." << endl;
		// std::lock_guard<std::mutex> lock(pairing_step_mutex);
	}

	if (
    gattlib_string_to_uuid(pairing_str, strlen(pairing_str), &pairing_uuid)<0)
  {
    printf("gattlib_string_to_uuid to pairing_uuid fail\n");
  }
  else
  {
    printf("gattlib_string_to_uuid to pairing_uuid success\n");
  }

	gattlib_register_notification(gatt_connection, message_handler, arg);

	ret = gattlib_notification_start(gatt_connection, &pairing_uuid);
	if (ret) {
		cerr <<  "Fail to start notification." << endl;
		goto DISCONNECT_EXIT;
	}
	else
	{
		cout <<"success to start notification" << endl;
	}

	ret = pthread_create(
		&connection->write_thread, NULL,	ble_write_step1, connection);
	if (ret != 0) {
		fprintf(stderr, "Failt to create BLE connection thread.\n");
		free(connection);
		return NULL;
	}
	cout << "Success to create BLE connection thread " << endl;
	ret = pthread_detach(connection->write_thread);
	if (ret != 0) {
		cerr <<  "Failt to detach BLE write_thread thread." << endl;
		free(connection);
		return NULL;
	}
	cout << "Success to detach BLE connection thread " << endl;
	loop = g_main_loop_new(NULL, 0);
	connection->properties_changes_loop = loop;
	g_main_loop_run(loop);

NOTIFICATION_EXIT:
	gattlib_notification_stop(gatt_connection, &pairing_uuid);

DISCONNECT_EXIT:
	gattlib_disconnect(gatt_connection);

CONNECTION_EXIT:
	pthread_mutex_unlock(&g_mutex);
	return NULL;
}

// ----------------------- paired end ---------

// ----------------------- admin start ---------

void *ble_admin_write_step2(void *arg)
{
	int ret, i;
	admin_connection_t *connection = (admin_connection_t *)arg;
	gatt_connection_t* gatt_connection = connection->gatt_connection;
	char *addr = (connection->lock).lock.addr;
	ADMIN_STEP &admin_step = connection->admin_step;
	std::mutex &admin_step_mutex = connection->admin_step_mutex;
	size_t &step_max_size = connection->step_max_size;
	uint8_t *&step_data = connection->step_data;
	size_t &n_size_byte = connection->n_size_byte;
	uint8_t *&admin_key = (connection->lock).admin_key;
	size_t &admin_key_len = (connection->lock).admin_key_len;
	int &connectionID = connection->connectionID;
	uint8_t *step2Bytes;
	int step2Len;
	size_t payload_len = 0;
	uint8_t *payloadBytes = NULL;
	
	cout << "------------------------------debug line-------------------" << endl;
	cout << "===========================ble_admin_write_step2" << endl;
	for (int j = 0; j < admin_key_len; j++)
	{
		printf("%02x ", admin_key[j]);
	}
	printf("\n");
	ret = igloohome_ble_lock_crypto_AdminConnection_beginConnection(admin_key, admin_key_len);
	if (ret == ERROR_CONNECTION_ID)
	{
		cout << "igloohome_ble_lock_crypto_AdminConnection_beginConnection error" << endl;
		goto ADMIN_STEP2_EXIT;
	}
	cout << "igloohome_ble_lock_crypto_AdminConnection_beginConnection success" << endl;

	connectionID = ret;

	step2Len = igloohome_ble_lock_crypto_AdminConnection_genConnStep2Native(
		connectionID, step_data+n_size_byte, step_max_size-n_size_byte, &step2Bytes);
	if (!step2Len)
	{
		cerr << "igloohome_ble_lock_crypto_AdminConnection_genConnStep2Native err" << endl;
		goto ADMIN_STEP2_EXIT;
	}
	cout << "igloohome_ble_lock_crypto_AdminConnection_genConnStep2Native success" << endl;

	if (!build_msg_payload(
		&payloadBytes, payload_len, step2Bytes, step2Len))
	{
		cerr << "failed in build_msg_payload" << endl;
		goto ADMIN_STEP2_EXIT;
	}
	cout << "build_msg_payload success" << endl;
	for (int j = 0; j < payload_len; j++)
	{
		printf("%02x ", payloadBytes[j]);
	}
	printf("\n");
	ret = write_char_by_uuid_multi_atts(
		gatt_connection, &admin_uuid, payloadBytes, payload_len);
	if (ret != GATTLIB_SUCCESS) {
		cerr << "write_char_by_uuid_multi_atts failed in writing packags" << endl;
		goto ADMIN_FREE_PAYLOAD;
	}
	else
	{
		cout << "admin write step2 write_char_by_uuid_multi_atts success in writing packages" << endl;
		admin_step = ConnectionStep2;
		step_max_size = 0;
		n_size_byte = 0;
		free(step_data);
		step_data = NULL;
	}

ADMIN_FREE_PAYLOAD:
	free(payloadBytes);

ADMIN_FREE_STEP2:
	free(step2Bytes);

ADMIN_STEP2_EXIT:
	return NULL;
}

int handle_admin_step3_message(
	admin_connection_t *connection, const uint8_t* data, size_t data_length,
	size_t &step_max_size, size_t &step_cur_size, uint8_t *&step_data,
  size_t &n_size_byte, ADMIN_STEP &admin_step, 
  std::mutex &admin_step_mutex
)
{
	cout << "------------------------------debug line-------------------" << endl;
	cout << "----------------handle_admin_step3_message" << endl;
	if (admin_step != ConnectionStep2)
	{
		cerr << "handle_admin_step1_message admin_step != ConnectionStepNone, error" << endl;
		return 1;
	}

	save_message_data(
		data, data_length, step_max_size, step_cur_size, step_data, 
		n_size_byte);

	if (step_max_size == step_cur_size)
	{
		int ret;
		cout << "RECV admin_step3 data finished" << endl;
		// debug
		for(int j = 0; j < step_max_size; j++)
		{
			printf("%02x ", step_data[j]);
		}
		printf("\n");
		admin_step = ConnectionEstablished;

		bool rec_ret = igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native(
			connection->connectionID, step_data+n_size_byte, step_max_size-n_size_byte
		);
		if (!rec_ret){
			cerr << "igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native err" << endl;
		}
		cout << "igloohome_ble_lock_crypto_AdminConnection_recConnStep3Native success" << endl;
	}
}

int handle_admin_step1_message(
	admin_connection_t *connection, const uint8_t* data, size_t data_length,
	size_t &step_max_size, size_t &step_cur_size, uint8_t *&step_data,
  size_t &n_size_byte, ADMIN_STEP &admin_step, 
  std::mutex &admin_step_mutex
)
{
	cout << "------------------------------debug line-------------------" << endl;
	cout << "----------------handle_admin_step1_message" << endl;
	if (admin_step != ConnectionStepNone)
	{
		cerr << "handle_admin_step1_message admin_step != ConnectionStepNone, error" << endl;
		return 1;
	}

	save_message_data(
		data, data_length, step_max_size, step_cur_size, step_data, 
		n_size_byte);

	if (step_max_size == step_cur_size)
	{
		int ret;
		cout << "RECV admin_step1 data finished" << endl;
		admin_step = ConnectionStep1;
		ret = pthread_create(&connection->write_thread, NULL,	ble_admin_write_step2, connection);
		if (ret != 0) {
			cerr <<  "Failt to create BLE admin write_thread thread." << endl;
			free(connection);
			return 5;
		}
		ret = pthread_detach(connection->write_thread);
		if (ret != 0) {
			cerr <<  "Failt to detach BLE admin write_thread thread." << endl;
			free(connection);
			return 6;
		}
		// 清空之前的数据, 表明当前数据已经处理完,不再需要
		// 不能在这处理, 因为数据还要给write step2用
		// step_max_size = 0;
		// step_cur_size = 0;
		// n_size_byte = 0;
		// free(step_data);
		// step_data = NULL;
	}
	return 0;
}

void admin_message_handler(
	const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data)
{
	int i;
	admin_connection_t *connection = (admin_connection_t *)user_data;
	char *addr = (connection->lock).lock.addr;
	size_t &step_max_size = connection->step_max_size;
	size_t &step_cur_size = connection->step_cur_size;
	uint8_t *&step_data = connection->step_data;
  size_t &n_size_byte = connection->n_size_byte;
	ADMIN_STEP &admin_step = connection->admin_step;
  std::mutex &admin_step_mutex = connection->admin_step_mutex;
	cout << "------------------------------debug line-------------------" << endl;
	cout << "------------admin_message_handler " << endl;
	printf("admin message from ADDR %s\n", addr);
	printf("admin message Handler: data_length %d \n", data_length);
	printf("connection->admin_step %d\n", connection->admin_step);
	switch (connection->admin_step)
	{
	case ConnectionStepNone:
	{
		handle_admin_step1_message(
			connection, data, data_length, step_max_size, step_cur_size, step_data,
  		n_size_byte, admin_step, admin_step_mutex);
		break;
	}
	case ConnectionStep2:
	{
		handle_admin_step3_message(
			connection, data, data_length, step_max_size, step_cur_size, step_data,
  		n_size_byte, admin_step, admin_step_mutex);
		break;
	}
	case ConnectionEstablished:
	{
		cout << "admin connection Established" << endl;
		for (int j = 0; j < data_length; j++)
		{
			printf("%02x ", data[j]);
		}
		printf("\n");
		break;
	}
	default:
	{	
		cerr << "admin connection step wrong in admin message handle" << endl;
		break;
	}
	}
	return;
}

void *ble_admin_connection(void *arg)
{
	admin_connection_t *connection = (admin_connection_t *)arg;
	char* addr = (connection->lock).lock.addr;
	ADMIN_STEP &admin_step = connection->admin_step;
	int ret, i;
	GMainLoop *loop;
	gatt_connection_t* gatt_connection;

	cout << "------------------------------debug line-------------------" << endl;
	printf("ble_admin_connection to ADDR %s\n", addr);

	cout << "ble_admin_connection gattlib_connect to device" << endl;
	gatt_connection = gattlib_connect(NULL, addr, GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
	if (gatt_connection == NULL) {
		cerr << "ble_admin_connection Fail to connect to the bluetooth device." << endl;
		goto ADMIN_EXIT;
	} else {
		cout << "ble_admin_connection Succeeded to connect to the bluetooth device." << endl;
		// std::lock_guard<std::mutex> lock(pairing_step_mutex);
	}
	connection->gatt_connection = gatt_connection;

	if (
    gattlib_string_to_uuid(admin_str, strlen(admin_str), &admin_uuid)<0)
  {
    printf("ble_admin_connection gattlib_string_to_uuid to admin_uuid fail\n");
  }
  else
  {
    printf("ble_admin_connection gattlib_string_to_uuid to admin_uuid success\n");
  }

	gattlib_register_notification(gatt_connection, admin_message_handler, arg);

	ret = gattlib_notification_start(gatt_connection, &admin_uuid);
	if (ret) {
		cerr <<  "ble_admin_connection Fail to start notification." << endl;
		goto ADMIN_EXIT;
	}
	else
	{
		cout <<"ble_admin_connection success to start notification" << endl;
	}

	// ret = pthread_create(
	// 	&connection->write_thread, NULL,	ble_write_step1, connection);
	// if (ret != 0) {
	// 	fprintf(stderr, "Failt to create BLE connection thread.\n");
	// 	free(connection);
	// 	return NULL;
	// }
	loop = g_main_loop_new(NULL, 0);
	g_main_loop_run(loop);

ADMIN_NOTIFICATION_EXIT:
	gattlib_notification_stop(gatt_connection, &admin_uuid);


ADMIN_EXIT:
	return NULL;
}

// ----------------------- admin end ---------

// ----------------------- discovered started ---------

void ble_discovered_device(void *adapter, const char* addr, const char* name, void *user_data) {
	int ret;
	int addr_size;
	char *up_addr;
	int name_size;

	if (name) {
    regex IGM_regex("^IGM.*?",  std::regex_constants::icase);
		
		// regex IGM_regex("IGM303e31a5c",  std::regex_constants::icase);
    if (regex_match(name, IGM_regex))
    {
			up_addr = strdup(addr);
			addr_size = strlen(up_addr);
			name_size = strlen(name);
      // printf("result Discovered %s - '%s '\n", addr, name);
    }
    else
    {
      return;
    }
	} else {
		// printf("Discovered %s, don't have name, return\n", addr);
		return;
  }

	sp_near_list->push_front(igm_lock_t(addr_size, up_addr, name_size, name));
	return;
}
// ----------------------- discovered end ---------


int main(int argc, char *argv[])
{
  int ret;
  // use default device;
  adapter_name = NULL;
	sp_near_list.reset(new igm_nearby_lock_list);
	sp_lock_list.reset(new igm_lock_list);

  ret = gattlib_adapter_open(adapter_name, &adapter);
  if (ret) {
		fprintf(stderr, "ERROR: Failed to open adapter.\n");
		return 1;
	}
	
	pthread_mutex_lock(&g_mutex);
	ret = gattlib_adapter_scan_enable(
		adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, NULL /* user_data */);
	if (ret) {
		fprintf(stderr, "ERROR: Failed to scan.\n");
		goto EXIT;
	}

	// paired 每一个 设备, 测试
  for (auto &nearby_lock: *(sp_near_list))
	{
		printf("name %s addr %s\n", nearby_lock.name, nearby_lock.addr);
		connection_t *connection = (connection_t *)malloc(sizeof(connection_t));
		if (connection == NULL) {
			fprintf(stderr, "Failt to allocate connection.\n");
			continue;
		}
		// 初始化 connection
		connection_init(connection);
		connection->pairing_step = PairingStepNone;
		connection->lock = nearby_lock;
		ret = pthread_create(&connection->thread, NULL,	ble_pair_device, connection);
		if (ret != 0) {
			fprintf(stderr, "Failt to create BLE connection thread.\n");
			free(connection);
			continue;
		}
		g_ble_connections.push_back(connection);
	}
	
	for (int i = 0; i < g_ble_connections.size(); i++) {
		struct connection_t *connection = g_ble_connections[i];
		pthread_join(connection->thread, NULL);
		free(connection);
	}

	// 配对完成的锁, 进行一个admin connection 的连接
	for (auto &paired_lock: *(sp_lock_list))
	{
		printf("name %s addr %s\n", (paired_lock.lock).name, (paired_lock.lock).addr);
		admin_connection_t *admin_connection = (admin_connection_t *)malloc(sizeof(admin_connection_t));
		if (admin_connection == NULL)
		{
			cerr << "Failt to allocate connection." << endl;
			continue;
		}
		admin_connection_init(admin_connection);
		admin_connection->admin_step = ConnectionStepNone;
		admin_connection->lock = paired_lock;
		ret = pthread_create(&admin_connection->thread, NULL,	ble_admin_connection, admin_connection);
		if (ret != 0) {
			fprintf(stderr, "Failt to create BLE connection thread.\n");
			free(admin_connection);
			continue;
		}
		g_admin_connections.push_back(admin_connection);
	}

	for (int i = 0; i < g_admin_connections.size(); i++) {
		admin_connection_t *admin_connection = g_admin_connections[i];
		pthread_join(admin_connection->thread, NULL);
		free(admin_connection);
	}
EXIT:
	gattlib_adapter_close(adapter);
	g_ble_connections.clear();
	return ret;
}