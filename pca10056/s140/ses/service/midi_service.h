/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools|Templates
 * and open the template in the editor.
 */

/* 
 * File:   midi_service.h
 * Author: mafaneh
 */

#ifndef MIDI_SERVICE_H
#define MIDI_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_midi_service instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */

#ifndef BLE_MIDI_BLE_OBSERVER_PRIO
#define BLE_MIDI_BLE_OBSERVER_PRIO 2
#endif

#define BLE_MIDI_DEF(_name)                                                                         \
static ble_midi_service_t _name;                                                                    \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_MIDI_BLE_OBSERVER_PRIO,                                                    \
                     ble_midi_service_on_ble_evt, &_name)

// MIDI Service UUID: 03B80E5A-EDE8-4B33-A751-6CE34EC4C700
// MIDI Data I/O Characteristic UUID: 7772E5DB-3868-4112-A1A9-F2669D106BF3
// The bytes need to be in reverse order to match the UUID from the spec
// Note: They are stored in little endian, meaning that the Least Significant Byte
// is stored first.
    
#define BLE_UUID_MIDI_SERVICE_BASE_UUID  {0x00, 0xC7, 0xC4, 0x4E, 0xE3, 0x6C, 0x51, 0xA7, 0x33, 0x4B, 0xE8, 0xED, 0x5A, 0x0E, 0xB8, 0x03}
#define BLE_UUID_MIDI_SERVICE_UUID        0x0E5A
    
#define BLE_UUID_MIDI_DATA_IO_CHAR_BASE_UUID {0xF3, 0x6B, 0x10, 0x9D, 0x66, 0xF2, 0xA9, 0xA1, 0x12, 0x41, 0x68, 0x38, 0xDB, 0xE5, 0x72, 0x77}
#define BLE_UUID_MIDI_DATA_IO_CHAR_UUID       0xE5DB
    
/**@brief MIDI Service event type. */
typedef enum
{
    BLE_DATA_IO_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_DATA_IO_EVT_NOTIFICATION_DISABLED,                            /**< Custom value notification disabled event. */
} ble_midi_evt_type_t;

/**@brief MIDI Service event. */
typedef struct{
  ble_midi_evt_type_t evt_type;
} ble_midi_evt_t;

// Forward declaration of the ble_midi_service_t type. 
typedef struct ble_midi_service_s ble_midi_service_t;

typedef void (*ble_midi_evt_handler_t) (ble_midi_service_t * p_midi_service, ble_midi_evt_t * p_evt);

typedef struct
{
    ble_midi_evt_handler_t evt_handler;  /**< Event handler to be called when a MIDI event occurs. */
} ble_midi_service_init_t;

/**@brief MIDI Service structure. This contains various status information for the service. */
struct ble_midi_service_s
{
    uint16_t                    service_handle;
    ble_gatts_char_handles_t    data_io_char_handles;
    uint8_t                     uuid_type;
    uint16_t                    conn_handle;
    ble_midi_evt_handler_t      evt_handler;
};

/**@brief Function for initializing the MIDI Service.
 *
 * @param[out]  p_midi_service   MIDI Service structure. This structure will have to be supplied by
 *                               the application. It will be initialized by this function, and will later
 *                               be used to identify this particular service instance.
 * @param[in]   p_midi_service_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_midi_service_init(ble_midi_service_t * p_midi_service, const ble_midi_service_init_t * p_midi_service_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the MIDI Service.
 *
 *
 * @param[in]   p_midi_service      LED Button Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_midi_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

#endif /* MIDI_SERVICE_H */