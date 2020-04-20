#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <list>

#include <glib.h>
#include <regex>
#include <gattlib/include/gattlib.h>
#include <lock/connection/pairing_connection.h>
#include <ctime>
#include <lock/connection/admin_connection.h>

using namespace std;

// C6:9C:04:67:1B:A7 - 'IGM401671ba7'
#define MAX_DEVICE_NAME (12+1)
#define MAX_DEVICE_ADDR (17+1)
#define BLE_SCAN_TIMEOUT   4
#define DEFAULT_STEP2_DATA_SIZE (85)
#define DEFAULT_PAIR_TIMEOUT (5)
#define MAX_DEVICE_DISCOVER_NUM (100)

enum PAIRING_STEP {
	PairingStepNone = 0,
  PairingStep1,
  PairingStep2,
  PairingStep3,
  PairingStep4,
  PairingStepDone
};

typedef void (*ble_discovered_device_t)(const char* addr, const char* name);
uuid_t pairing_uuid, admin_uuid, guest_uuid;
char pairing_str[] = "5c3a659e-897e-45e1-b016-007107c96df6";
char admin_str[] = "5c3a659f-897e-45e1-b016-007107c96df6";
char guest_str[] = "5c3a65a0-897e-45e1-b016-007107c96df6";
GMainLoop *m_main_loop;

// resource
char* adapter_name;
void* adapter;

// static uint8_t privateKey_[kPrivateKeyLength] = {0x00};
// static uint8_t publicKey_[kPublicKeyLength] = {0x00};
// static uint8_t otherPublicKey_[kPublicKeyLength] = {0x00};
// static uint8_t txNonce_[kNonceLength] = {0x00};
// static uint8_t rxNonce_[kNonceLength] = {0x00};
// static uint8_t sharedKey_[kConnectionKeyLength] = {0x00};
struct igm_lock_t {
  char addr[MAX_DEVICE_ADDR];
  int addr_len;
  char name[MAX_DEVICE_NAME];
  int name_len;
  igm_lock_t ():addr_len(0), name_len(0)
  {
    memset(addr, 0, MAX_DEVICE_ADDR);
    memset(name, 0, MAX_DEVICE_NAME);
  }
  igm_lock_t(const int &addr_len_, const char addr_[], const int &name_len_, const char name_[])
  {
    addr_len_>=MAX_DEVICE_ADDR?addr_len=MAX_DEVICE_ADDR-1:addr_len=addr_len_;
    memcpy(addr, addr_, addr_len);
    addr[addr_len] = '\0';
    name_len_>=MAX_DEVICE_NAME?name_len=MAX_DEVICE_ADDR-1:name_len=name_len_;
    memcpy(name, name_, name_len);
    name[name_len] = '\0';
  }
  igm_lock_t& operator=(igm_lock_t o)
  {
    memcpy(addr, o.addr, o.addr_len);
    memcpy(name, o.name, o.name_len);
    addr_len = o.addr_len;
    name_len = o.name_len;
  }
};

struct paired_igm_lock_t {
  igm_lock_t lock;
  bool paired;
  uint8_t *admin_key;
  size_t admin_key_len;
  paired_igm_lock_t()
  {
    paired = false;
  }
  paired_igm_lock_t(
    igm_lock_t lock_, bool paired_, uint8_t *admin_key_, size_t admin_key_len_)
  {
    lock = lock_;
    paired = paired_;
    admin_key = admin_key_;
    admin_key_len = admin_key_len_;
  }
};

typedef std::list<igm_lock_t> igm_nearby_lock_list;
typedef std::list<paired_igm_lock_t> igm_lock_list;

std::shared_ptr<igm_nearby_lock_list> sp_near_list;
std::shared_ptr<igm_lock_list> sp_lock_list;

// igm_lock_t nearby_locks[MAX_DEVICE_DISCOVER_NUM];
// size_t begin_lock_index
// size_t cur_lock_index;
// size_t end_lock_index;


struct connection_t {
	pthread_t thread;
	pthread_t write_thread;
	gatt_connection_t* gatt_connection;
  std::mutex pair_complete_mutex;
  std::condition_variable pair_complete;
  igm_lock_t lock;
  std::mutex pairing_step_mutex;
	PAIRING_STEP pairing_step;
	size_t step2_max_size;
	size_t  step2_cur_size;
	size_t n_size_byte;
	uint8_t *step2_data;
  size_t step4_max_size;
  size_t  step4_cur_size;
  uint8_t *step4_data;
  GMainLoop *properties_changes_loop;
};

enum ADMIN_STEP {
  ConnectionStepNone=0,
  ConnectionStep1,
  ConnectionStep2,
  ConnectionEstablished
};


struct admin_connection_t {
	pthread_t thread;
	pthread_t write_thread;
	gatt_connection_t* gatt_connection;
  std::mutex admin_complete_mutex;
  std::condition_variable admin_complete;
  paired_igm_lock_t lock;
  std::mutex admin_step_mutex;
	ADMIN_STEP admin_step;
  GMainLoop *properties_changes_loop;
  size_t step_max_size, step_cur_size, n_size_byte;
  uint8_t *step_data;
};

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

std::vector<connection_t *> g_ble_connections;
std::vector<admin_connection_t *> g_admin_connections;

// helper functions
int write_char_by_uuid_multi_atts (
	gatt_connection_t* gatt_connection, uuid_t* uuid, const uint8_t* buffer, 
	size_t buffer_len, size_t BLE_ATT_MAX_BYTES=20);
bool build_msg_payload(uint8_t **p_payloadBytes, 
  size_t &payload_len, uint8_t *stepBytes, size_t step_len);
int save_message_data(
  const uint8_t* data, size_t data_length,
  size_t &step_max_size, size_t &step_cur_size, uint8_t *&step_data,
  size_t &n_size_byte);
void connection_init(connection_t *connection);

// 锁返回的是2和4的消息
int handle_step2_message(
  connection_t *connection, const uint8_t* data, size_t data_length,
  size_t &step2_max_size, size_t &step2_cur_size, uint8_t *&step2_data,
  size_t &n_size_byte, PAIRING_STEP &pairing_step, 
  std::mutex &pairing_step_mutex
);
int handle_step4_message(
  connection_t *connection, const uint8_t* data, size_t data_length,
  size_t &step3_max_size, size_t &step3_cur_size, uint8_t *&step3_data,
  size_t &n_size_byte, PAIRING_STEP &pairing_step, 
  std::mutex &pairing_step_mutex);