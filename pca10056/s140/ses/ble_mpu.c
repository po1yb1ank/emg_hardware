 /* 
  * This code is not extensively tested and only 
  * meant as a simple explanation and for inspiration. 
  * NO WARRANTY of ANY KIND is provided. 
  */

#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "service/ble_mpu.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "service/app_mpu.h"

static void on_write(ble_mpu_t * p_mpu, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (   (p_evt_write->handle == p_mpu->accel_char_handles.cccd_handle)
        && (p_evt_write->len == 2))
    {
        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            p_mpu->is_notification_enabled = true;
        }
        else
        {
            p_mpu->is_notification_enabled = false;
        }
    }
}

void ble_mpu_on_ble_evt(ble_mpu_t * p_mpu, ble_evt_t const * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            p_mpu->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            p_mpu->conn_handle = BLE_CONN_HANDLE_INVALID;
        case BLE_GATTS_EVT_WRITE:
            on_write(p_mpu, p_ble_evt);
            break;
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial. 
 *
 * @param[in]   p_mpu        mpu structure.
 *
 */
static uint32_t ble_char_accel_add(ble_mpu_t * p_mpu)
{
    uint32_t   err_code = 0; // Variable to hold return codes from library and softdevice functions
    
    ble_uuid_t          char_uuid;   
    BLE_UUID_BLE_ASSIGN(char_uuid, BLE_UUID_ACCEL_CHARACTERISTC_UUID);
    
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;
    
    ble_gatts_attr_md_t cccd_md;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 1;
        
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc = BLE_GATTS_VLOC_STACK;    
    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    //attr_char_value.max_len     = sizeof(accel_values_t);
    //attr_char_value.init_len    = sizeof(accel_values_t);
    attr_char_value.max_len       =sizeof(complicated_value);
    attr_char_value.init_len       =sizeof(complicated_value);
    //uint8_t value[6]            = {0};
    uint8_t value[12]            = {0};
    attr_char_value.p_value     = value;

    err_code = sd_ble_gatts_characteristic_add(p_mpu->service_handle,
                                       &char_md,
                                       &attr_char_value,
                                       &p_mpu->accel_char_handles);
    APP_ERROR_CHECK(err_code);   

    return NRF_SUCCESS;
}


/**@brief Function for initiating our new service.
 *
 * @param[in]   p_mpu        Our Service structure.
 *
 */
void ble_mpu_service_init(ble_mpu_t * p_mpu)
{
    uint32_t   err_code; // Variable to hold return codes from library and softdevice functions

    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid = BLE_UUID_BASE_UUID;
    service_uuid.uuid = BLE_UUID_MPU_SERVICE_UUID;
    service_uuid.type = 0x01;
    //BLE_GATTS_SRVC_TYPE_PRIMARY
    err_code = sd_ble_uuid_vs_add(&base_uuid,&service_uuid.type );// &service_uuid.type);
   // APP_ERROR_CHECK(err_code);    

    p_mpu->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_mpu->is_notification_enabled = false;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_mpu->service_handle);
    
    APP_ERROR_CHECK(err_code);

    ble_char_accel_add(p_mpu);
}


//uint32_t ble_mpu_update(ble_mpu_t *p_mpu, accel_values_t * accel_values)
uint32_t ble_mpu_update_c(ble_mpu_t *p_mpu, complicated_value * mpu_compl)
{
    // Send value if connected and notifying
    if ((p_mpu->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_mpu->is_notification_enabled))
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    //uint16_t               len = sizeof(accel_values_t);
    uint16_t                 len = sizeof(complicated_value);
    ble_gatts_hvx_params_t hvx_params;
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = p_mpu->accel_char_handles.value_handle;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = 0;
    hvx_params.p_len  = &len;
    hvx_params.p_data = (uint8_t *) mpu_compl;  

    return sd_ble_gatts_hvx(p_mpu->conn_handle, &hvx_params);
}
