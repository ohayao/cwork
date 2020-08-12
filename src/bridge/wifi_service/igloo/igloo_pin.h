#ifndef _IGLOO_PIN_H
#define _IGLOO_PIN_H

#include <stdbool.h>
#include <stdint.h>

#include "igloo.h"

#ifdef __cplusplus
extern "C" {
#endif

//////////////// PIN validation
typedef enum IgPinType {
	IG_PINTYPE_INVALID = 0,
	IG_PINTYPE_ONE_TIME = 1,
	IG_PINTYPE_PERMANENT = 2,
	IG_PINTYPE_WEEKLY = 3,
	IG_PINTYPE_DURATION = 4,
	IG_PINTYPE_MONTHLY = 5,
	IG_PINTYPE_WEEKDAY = 6,
	IG_PINTYPE_WEEKEND = 7,
} IgPinType;
typedef struct IgPinResult {
	bool valid;			// if PIN is valid, i.e., it has start_time within the last 24 hours to 1 week from now
	IgPinType type;		// type of PIN, if it is valid
	uint32_t start_time;	// start time of PIN, if it is valid
	uint32_t end_time;		// end time of PIN, if it is valid
} IgPinResult;
/*
 * Initialize the pin algorithm. This needs to be called only once upon booting.
 * key - pin key generated when pairing
 * key_len - length of the key in bytes
 * current_time - current time, in seconds since epoch
 * gmt_offset - GMT offset in seconds.
 */
bool ig_initialize_pin_validator(uint8_t *key, uint32_t key_len, uint32_t current_time, int32_t gmt_offset);
/*
 * Checks if the given PIN is valid
 * Parameters:
 * pin - ASCII string of the PIN entered by the user. e.g., [31, 32, 33, 34] for "1234"
 * pin_length - length of the above byte array. In the example above, it will be 4
 * current_time - current time, in seconds since epoch.
 * gmt_offset - GMT offset in seconds.
 * result - a struct representing the result of the calculation, i.e, whether the pin is valid, what type of pin it is etc. 
 *
 * returns false if there are errors in the parameters
 */
bool ig_validate_pin(uint8_t *pin, uint8_t pin_length, uint32_t current_time, int32_t gmt_offset, IgPinResult *result);	
/*
 * Checks if the given PIN is valid in the future (currently 2 weeks)
 * Parameters:
 * pin - ASCII string of the PIN entered by the user. e.g., [31, 32, 33, 34] for "1234"
 * pin_length - length of the above byte array. In the example above, it will be 4
 * current_time - current time, in seconds since epoch.
 * gmt_offset - GMT offset in seconds.
 * result - a struct representing the result of the calculation, i.e, whether the pin is valid, what type of pin it is etc. 
 *
 * returns false if there are errors in the parameters
 */
bool ig_validate_future_pin(uint8_t *pin, uint8_t pin_length, uint32_t current_time, int32_t gmt_offset, IgPinResult *result);

//////////////// end PIN validation

#ifdef __cplusplus
}
#endif

#endif // _IGLOO_PIN_H
