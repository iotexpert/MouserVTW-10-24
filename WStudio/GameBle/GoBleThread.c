
#include "../WStudio/GameBle/GameThread.h"
#include "../WStudio/GameBle/GoBle_db.h"
#include "../WStudio/GameBle/SystemGlobal.h"
#include "wiced.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_stack.h"


/*******************************************************************
 * Variable Definitions
 ******************************************************************/
extern const wiced_bt_cfg_settings_t wiced_bt_cfg_settings;
extern const wiced_bt_cfg_buf_pool_t wiced_bt_cfg_buf_pools[WICED_BT_CFG_NUM_BUF_POOLS];

/*******************************************************************
 * Function Prototypes
 ******************************************************************/
static wiced_bt_dev_status_t  goble_management_callback    ( wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data );
static void                   goble_set_advertisement_data ( void );
static wiced_bt_gatt_status_t goble_event_handler          ( wiced_bt_gatt_evt_t  event, wiced_bt_gatt_event_data_t *p_event_data );

/*******************************************************************
 * Function Definitions
 ******************************************************************/
void GoBleThread_start(void)
{
    wiced_bt_stack_init(goble_management_callback, &wiced_bt_cfg_settings, wiced_bt_cfg_buf_pools);
}


/* Set Advertisement Data */
void goble_set_advertisement_data( void )
{
    wiced_bt_ble_advert_elem_t adv_elem[3] = { 0 };
    uint8_t adv_flag = BTM_BLE_GENERAL_DISCOVERABLE_FLAG | BTM_BLE_BREDR_NOT_SUPPORTED;
    uint8_t num_elem = 0; 

    /* Advertisement Element for Flags */
    adv_elem[num_elem].advert_type = BTM_BLE_ADVERT_TYPE_FLAG;
    adv_elem[num_elem].len = sizeof(uint8_t);
    adv_elem[num_elem].p_data = &adv_flag;
    num_elem++;

    uint8_t gobleuuid[] = {__UUID_GOBLE};

    /* Advertisement Element for Name */
    adv_elem[num_elem].advert_type = BTM_BLE_ADVERT_TYPE_128SRV_COMPLETE;
    adv_elem[num_elem].len = 16;
    adv_elem[num_elem].p_data = gobleuuid;
    num_elem++;

    /* Set Raw Advertisement Data */
    wiced_bt_ble_set_raw_advertisement_data(num_elem, adv_elem);
}


/* Bluetooth Management Event Handler */
wiced_bt_dev_status_t goble_management_callback( wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data )
{
    wiced_bt_dev_status_t status = WICED_BT_SUCCESS;

    switch (event)
    {
    case BTM_ENABLED_EVT:
        goble_set_advertisement_data();
        wiced_bt_gatt_register( goble_event_handler );
        wiced_bt_gatt_db_init( gatt_database, gatt_database_len );
        wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);
        break;

    case BTM_SECURITY_REQUEST_EVT:
        wiced_bt_ble_security_grant(p_event_data->security_request.bd_addr, WICED_BT_SUCCESS);
        break;

    case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
        p_event_data->pairing_io_capabilities_ble_request.local_io_cap = BTM_IO_CAPABILITIES_NONE;
        p_event_data->pairing_io_capabilities_ble_request.oob_data = BTM_OOB_NONE;
        p_event_data->pairing_io_capabilities_ble_request.auth_req = BTM_LE_AUTH_REQ_NO_BOND;
        break;


    case BTM_USER_CONFIRMATION_REQUEST_EVT: // Just confirm
        wiced_bt_dev_confirm_req_reply( WICED_BT_SUCCESS , p_event_data->user_confirmation_request.bd_addr);
        break;

    case BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
        WPRINT_APP_INFO(("Paired linke keys\n"));
        status = WICED_BT_ERROR;
        break;

    case BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT:
    case BTM_PAIRING_COMPLETE_EVT:
    case BTM_ENCRYPTION_STATUS_EVT:
    case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
    case BTM_LPM_STATE_LOW_POWER:
        break;

    default:
        WPRINT_APP_INFO(("Unhandled Bluetooth Management Event: 0x%x (%d)\n", event, event));
        break;
    }

    return status;
}


/* GATT Event Handler */
wiced_bt_gatt_status_t goble_event_handler( wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_event_data )
{
    wiced_bt_gatt_status_t status = WICED_BT_GATT_ERROR;
    wiced_bt_gatt_attribute_request_t *p_attr_req = NULL;


    switch ( event )
    {
    case GATT_CONNECTION_STATUS_EVT:
        if(!p_event_data->connection_status.connected)
        {
            WPRINT_APP_INFO(("Disconnected\n"));
            wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);
        }
        else
            WPRINT_APP_INFO(("Connected\n"));
        break;
    case GATT_ATTRIBUTE_REQUEST_EVT:

        p_attr_req = &p_event_data->attribute_request;
        if( p_attr_req->request_type == GATTS_REQ_TYPE_WRITE  && p_attr_req->data.handle == HDLC_GOBLE_SERIALPORTID_VALUE)
        {
            uint32_t numButtons = p_attr_req->data.write_req.p_val[3];
            uint32_t sliderY = p_attr_req->data.write_req.p_val[5+numButtons];
            uint32_t sliderX = p_attr_req->data.write_req.p_val[6+numButtons];
            uint32_t buttonMask = 0x00;
            for(int i=0;i<numButtons;i++)
            {
                buttonMask |= (1<<p_attr_req->data.write_req.p_val[5+i]);
            }

            game_msg_t msg;
            msg.evt = MSG_POSITION;
            msg.val = 100 - (100*sliderY/255);
            wiced_rtos_push_to_queue(&paddleQueue,&msg,0);
            if(buttonMask & 0x10)
            {
                msg.evt = MSG_BUTTON0;
                msg.val = 1;
                wiced_result_t result=wiced_rtos_push_to_queue(&paddleQueue,&msg,0);
            }
            status = WICED_BT_GATT_SUCCESS;
        }
        break;
    default:
        status = WICED_BT_GATT_SUCCESS;
        break;
    }

    return status;
}
