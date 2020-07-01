#ifndef _IGLOO_H
#define _IGLOO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IG_KEY_EXCHANGE_PUBLIC_LENGTH 64
#define IG_KEY_EXCHANGE_PRIVATE_LENGTH 32
#define IG_KEY_LENGTH 16
#define IG_PIN_KEY_LENGTH 16
#define IG_PASSWORD_LENGTH 8
#define IG_MASTER_PIN_MIN_LENGTH 4
#define IG_MASTER_PIN_DEFAULT_LENGTH 8
#define IG_MASTER_PIN_MAX_LENGTH 10
#define IG_PIN_INPUT_MAX_LENGTH 16
#define IG_PIN_MAX_LENGTH 9
#define IG_PIN_MIN_LENGTH 4
#define IG_ACCESS_RIGHTS_SIZE 8
#define IG_ERROR_MAX_CONNECTIONS 1001

typedef enum IgConnectionCharacteristicType {
	IG_CONNECTION_TYPE_ADMIN,
	IG_CONNECTION_TYPE_GUEST,
	IG_CONNECTION_TYPE_PAIRING,
} IgConnectionCharacteristicType;

typedef enum IgErrorCode {
	IG_ERROR_NONE = 0,
	IG_ERROR_GENERIC_FAIL = 1,
	IG_ERROR_INVALID_CONN_ID = 2,
	IG_ERROR_DATA_TOO_SHORT = 3,
	IG_ERROR_INVALID_MESSAGE = 4,
	IG_ERROR_INVALID_PARAMS = 5,
	IG_ERROR_INTERNAL_ERROR = 6,
} IgErrorCode;

typedef enum IgLogType {
	IG_LOG_TYPE_ADMIN_UNLOCK = 11,
	IG_LOG_TYPE_EKEY_UNLOCK,
	IG_LOG_TYPE_SET_TIME,
	IG_LOG_TYPE_KEYPAD_VOLUME_CHANGE,
	IG_LOG_TYPE_BT_VOLUME_CHANGE,
	IG_LOG_TYPE_WRONG_PIN,
	IG_LOG_TYPE_PIN_TOO_LONG,
	IG_LOG_TYPE_MASTER_PIN_UNLOCK,
	IG_LOG_TYPE_STORED_PIN_UNLOCK,
	IG_LOG_TYPE_OTP_PIN,

	IG_LOG_TYPE_PERMANENT_PIN = 21,
	IG_LOG_TYPE_DURATION_PIN,
	IG_LOG_TYPE_WEEKLY_PIN,
	IG_LOG_TYPE_MONTHLY_PIN,
	IG_LOG_TYPE_WEEKDAY_PIN,
	IG_LOG_TYPE_WEEKEND_PIN,
	IG_LOG_TYPE_BT_NEW_PIN,
	IG_LOG_TYPE_KEYPAD_EDIT_PIN,
	IG_LOG_TYPE_BT_EDIT_PIN,
	IG_LOG_TYPE_KEYPAD_DELETE_PIN,

	IG_LOG_TYPE_BT_DELETE_PIN = 31,
	IG_LOG_TYPE_CHANGE_MASTER_PIN,
	IG_LOG_TYPE_KEYPAD_SET_AUTORELOCK,
	IG_LOG_TYPE_BT_SET_AUTORELOCK,
	IG_LOG_TYPE_KEYPAD_SET_WRONG_PIN_LOCKOUT_COUNT,
	IG_LOG_TYPE_SET_WRONG_PIN_LOCKOUT_COUNT,
	IG_LOG_TYPE_ADMIN_LOCK,
	IG_LOG_TYPE_ENABLE_AUTOUNLOCK,
	IG_LOG_TYPE_BLACKLIST_GUEST_KEY,
	IG_LOG_TYPE_UNBLACKLIST_GUEST_KEY,

	IG_LOG_TYPE_SET_DAYLIGHT_SAVING = 41,
	IG_LOG_TYPE_ADD_KEYCARD,
	IG_LOG_TYPE_KEYCARD_UNLOCK,
	IG_LOG_TYPE_DELETE_KEYCARD,
	IG_LOG_TYPE_SET_BRIGHTNESS,
	IG_LOG_TYPE_BT_SET_RELOCKALARM,
	IG_LOG_TYPE_AUTO_RELOCK,
	IG_LOG_TYPE_AUTO_RELOCK_WITHOUT_OPEN,
	IG_LOG_TYPE_LOCK_WITH_KEYPAD_OR_BUTTON,
	IG_LOG_TYPE_UNLOCK_WITH_BUTTON,

	IG_LOG_TYPE_LOCK_WITH_PHYSICAL_ACTION = 51,
	IG_LOG_TYPE_UNLOCK_WITH_PHYSICAL_ACTION,
	IG_LOG_TYPE_BREAKIN_ATTEMPT,
	IG_LOG_TYPE_ADD_FINGERPRINT,
	IG_LOG_TYPE_DELETE_FINGERPRINT,
	IG_LOG_TYPE_FINGERPRINT_UNLOCK,
	IG_LOG_TYPE_,
} IgLogType;

void ig_init(void);

///////////////// pairing state machine
typedef struct IgPairingResult {
	uint8_t admin_key[IG_KEY_LENGTH];
	uint8_t pin_key[IG_PIN_KEY_LENGTH];
	uint8_t password[IG_PASSWORD_LENGTH];
	uint32_t master_pin_length;
	uint8_t master_pin[IG_MASTER_PIN_MAX_LENGTH];
	uint32_t current_time;
	int32_t gmt_offset;
	uint32_t dst_length;
	uint8_t *dst;	// do not free
} IgPairingResult;
void ig_begin_pairing(void);
uint32_t ig_pairing_step2_size(void);
IgErrorCode ig_pairing_step2(uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written);
uint32_t ig_pairing_step4_size(void);
IgErrorCode ig_pairing_step4(uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written);
IgErrorCode ig_commit_pairing(uint8_t *data_in, uint32_t data_in_len, IgPairingResult *data_out);
void ig_end_pairing(void);
int ig_debug_get_pairing_state(void);
//////////////// end pairing state machine

//////////////// connection
typedef struct IgConnectionScratchpad {
	uint8_t scratchpad[32];
} IgConnectionScratchpad;

typedef struct IgGuestKeyParams {
	uint32_t key_id;
	uint32_t start_time;		// start time of key
	uint32_t end_time;			// end time of key
	// A bitset representing access rights for this guest key.
	// We label the least significant bit of access_rights[0] as bit 0. Bit number increases
	//  as we go towards more significant bits. And then increases with increasing index of the
	//  array. Until the most significant bit of access_rights[3] as bit 31
	// Each bit marks the permissions for a particular command. 1 means permitted and
	//  0 means not allowed. The correspondence is as follows:
	// bit 0 - message id 21
	// bit 1 - message id 23
	// ...
	// bit 30 - message id 81
	// bit 31 - message id 83
	uint8_t access_rights[IG_ACCESS_RIGHTS_SIZE];
} IgGuestKeyParams;
// Call at startup to pass in the admin key
void ig_set_admin_key(uint8_t key[IG_KEY_LENGTH]);

/*
 * Initializes an admin connection. Returns connection id if successful, else returns IG_ERROR_MAX_CONNECTIONS
 * scratchpad (unused) - memory for storing connection parameters
 */
uint32_t ig_admin_begin_connection(IgConnectionScratchpad *scratchpad);
uint32_t ig_admin_connect_step1_size(void);
IgErrorCode ig_admin_connect_step1(uint32_t conn_id, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written);
uint32_t ig_admin_connect_step3_size(void);
IgErrorCode ig_admin_connect_step3(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written);
bool ig_admin_end_connection(uint32_t conn_id);

/*
 * Initializes an guest connection. Returns connection id if successful, else returns IG_ERROR_MAX_CONNECTIONS
 * scratchpad (unused) - memory for storing connection parameters
 */
uint32_t ig_guest_begin_connection(IgConnectionScratchpad *scratchpad);
uint32_t ig_guest_connect_step2_size(void);
IgErrorCode ig_guest_connect_step2(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written, IgGuestKeyParams *guest_key_params);
uint32_t ig_guest_connect_step4_size(void);
IgErrorCode ig_guest_connect_step4(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written);
bool ig_guest_end_connection(uint32_t conn_id);

// pair of functions to encrypt and decrypt data for a specific connection id
uint32_t ig_encrypt_data_size(uint32_t data_len);
IgErrorCode ig_encrypt_data(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written);
uint32_t ig_decrypt_data_size(uint32_t data_len);
IgErrorCode ig_decrypt_data(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written);

// Pair of functions used to encrypt log data. The byte array input should be the output of ig_encode_objects
uint32_t ig_encrypt_log_data_size(uint32_t data_len);
IgErrorCode ig_encrypt_log_data(uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint32_t *bytes_written);
//////////////// end connection

//void decrypt_data(uint32_t conn_id, uint8_t *data_in, uint32_t data_in_len, uint8_t *data_out, uint32_t data_out_len, uint8_t *nonce, uint32_t *bytes_written);

#ifdef __cplusplus
}
#endif

#endif // _IGLOO_H
