#ifndef BLE_ADVDATA_H__
#define BLE_ADVDATA_H__
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <bridge/lock/messages/BleTypes.h>
#include <bridge/lock/messages/BleGap.h>

/**@brief Security Manager TK value. */
typedef struct
{
  uint8_t tk[BLE_GAP_SEC_KEY_LEN];      /**< Array containing TK value in little-endian format. */
} ble_advdata_tk_value_t;

/**@brief Advertising data LE Role types. This enumeration contains the options available for the LE role inside
 *        the advertising data. */
typedef enum
{
    BLE_ADVDATA_ROLE_NOT_PRESENT = 0,                                 /**< LE Role AD structure not present. */
    BLE_ADVDATA_ROLE_ONLY_PERIPH,                                     /**< Only Peripheral Role supported. */
    BLE_ADVDATA_ROLE_ONLY_CENTRAL,                                    /**< Only Central Role supported. */
    BLE_ADVDATA_ROLE_BOTH_PERIPH_PREFERRED,                           /**< Peripheral and Central Role supported. Peripheral Role preferred for connection establishment. */
    BLE_ADVDATA_ROLE_BOTH_CENTRAL_PREFERRED                           /**< Peripheral and Central Role supported. Central Role preferred for connection establishment */
} ble_advdata_le_role_t;

/**@brief Advertising data name type. This enumeration contains the options available for the device name inside
 *        the advertising data. */
typedef enum
{
    BLE_ADVDATA_NO_NAME,                                              /**< Include no device name in advertising data. */
    BLE_ADVDATA_SHORT_NAME,                                           /**< Include short device name in advertising data. */
    BLE_ADVDATA_FULL_NAME                                             /**< Include full device name in advertising data. */
} ble_advdata_name_type_t;

/**@brief UUID list type. */
typedef struct
{
    uint16_t                     uuid_cnt;                            /**< Number of UUID entries. */
    ble_uuid_t *                 p_uuids;                             /**< Pointer to UUID array entries. */
} ble_advdata_uuid_list_t;

/**@brief Connection interval range structure. */
typedef struct
{
    uint16_t                     min_conn_interval;                   /**< Minimum connection interval, in units of 1.25 ms, range 6 to 3200 (7.5 ms to 4 s). */
    uint16_t                     max_conn_interval;                   /**< Maximum connection interval, in units of 1.25 ms, range 6 to 3200 (7.5 ms to 4 s). The value 0xFFFF indicates no specific maximum. */
} ble_advdata_conn_int_t;

/**@brief Manufacturer specific data structure. */
typedef struct
{
    uint16_t                     company_identifier;                  /**< Company identifier code. */
    uint8_array_t                data;                                /**< Additional manufacturer specific data. */
} ble_advdata_manuf_data_t;

/**@brief Service data structure. */
typedef struct
{
    uint16_t                     service_uuid;                        /**< Service UUID. */
    uint8_array_t                data;                                /**< Additional service data. */
} ble_advdata_service_data_t;

typedef struct
{
    ble_advdata_name_type_t      name_type;                           /**< Type of device name. */
    uint8_t                      short_name_len;                      /**< Length of short device name (if short type is specified). */
    bool                         include_appearance;                  /**< Determines if Appearance shall be included. */
    uint8_t                      flags;                               /**< Advertising data Flags field. */
    int8_t *                     p_tx_power_level;                    /**< TX Power Level field. */
    ble_advdata_uuid_list_t      uuids_more_available;                /**< List of UUIDs in the 'More Available' list. */
    ble_advdata_uuid_list_t      uuids_complete;                      /**< List of UUIDs in the 'Complete' list. */
    ble_advdata_uuid_list_t      uuids_solicited;                     /**< List of solicited UUIDs. */
    ble_advdata_conn_int_t *     p_slave_conn_int;                    /**< Slave Connection Interval Range. */
    ble_advdata_manuf_data_t *   p_manuf_specific_data;               /**< Manufacturer specific data. */
    ble_advdata_service_data_t * p_service_data_array;                /**< Array of Service data structures. */
    uint8_t                      service_data_count;                  /**< Number of Service data structures. */
    bool                         include_ble_device_addr;             /**< Determines if LE Bluetooth Device Address shall be included. */
    ble_advdata_le_role_t        le_role;                             /**< LE Role field. Included when different from @ref BLE_ADVDATA_ROLE_NOT_PRESENT. @warning This field can be used only for NFC. For BLE advertising, set it to NULL. */
    ble_advdata_tk_value_t *     p_tk_value;                          /**< Security Manager TK value field. Included when different from NULL. @warning This field can be used only for NFC. For BLE advertising, set it to NULL.*/
    uint8_t *                    p_sec_mgr_oob_flags;                 /**< Security Manager Out Of Band Flags field. Included when different from NULL. @warning This field can be used only for NFC. For BLE advertising, set it to NULL.*/
#if (NRF_SD_BLE_API_VERSION > 1)
    ble_gap_lesc_oob_data_t *    p_lesc_data;                         /**< LE Secure Connections OOB data. Included when different from NULL. @warning This field can be used only for NFC. For BLE advertising, set it to NULL.*/
#endif
} ble_advdata_t;
#endif