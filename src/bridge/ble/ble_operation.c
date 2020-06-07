#include <bridge/ble/ble_operation.h>
#include <bridge/bridge_main/log.h>
#include <stdlib.h>

size_t resolve_payload_len(const size_t step_len)
{
	return step_len>254 ? (step_len+3) : (step_len+1);
}

// int x2d(int x)
// {
//   int ret = 0;
//   ret += (x %10);
//   x /= 10;
//   for (;x;){
//     ret += (x % 10) * 15;
//     x = x / 10;
//   }
//   printf("ret: %d", ret);
//   return ret;
// }



bool build_msg_payload(uint8_t **p_payloadBytes, size_t *payload_len, uint8_t *stepBytes, size_t step_len) {
	*payload_len = resolve_payload_len(step_len);
	size_t n_byte_for_len = *payload_len - step_len;
	(*p_payloadBytes) = (uint8_t *)malloc(*payload_len);
	if (n_byte_for_len == 1) {
		(*p_payloadBytes)[0] = step_len;
	}
	else if (n_byte_for_len == 3) {
		// 大端存储
		uint8_t fst, sec;
		sec = 254;
		fst = step_len - sec;
		(*p_payloadBytes)[0] = fst;
		(*p_payloadBytes)[1] = sec;
		(*p_payloadBytes)[3] = 0xff;  
	} else {
		serverLog(LL_ERROR, "wrong n_byte_for_len, error");
		return false;
	}
	memcpy((*p_payloadBytes) + n_byte_for_len, stepBytes, step_len);
	return true;
}


int write_char_by_uuid_multi_atts (
	gatt_connection_t* gatt_connection, uuid_t* uuid, const uint8_t* buffer, 
	size_t buffer_len)
{
  int BLE_ATT_MAX_BYTES = 20;
	int ret = GATTLIB_SUCCESS;
	uint8_t tmp_bytes[BLE_ATT_MAX_BYTES];
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
      serverLog(LL_ERROR, "write_char_by_uuid_multi_atts: gattlib_write_char_by_uuid failed in writint ");
			return ret;
		}
    // serverLog(LL_NOTICE, "write_char_by_uuid_multi_atts: gattlib_write_char_by_uuid success in writing");
	}
	return ret;
}

