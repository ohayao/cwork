#ifndef _BLE_OPERATION_H_
#define _BLE_OPERATION_H_
#include <stdint.h>
#include "bridge/gattlib/gattlib.h"
#include <stdbool.h>

int write_char_by_uuid_multi_atts (
	gatt_connection_t* gatt_connection, uuid_t* uuid, const uint8_t* buffer, 
	size_t buffer_len);
bool build_msg_payload(uint8_t **p_payloadBytes, 
                      size_t *payload_len, uint8_t *stepBytes, size_t step_len);
size_t resolve_payload_len(const size_t step_len);
// int x2d(int x);

#endif