/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @brief BLE LED Button Service central and client application main file.
 *
 * This example can be a central for up to 8 peripherals.
 * The peripheral is called ble_app_blinky and can be found in the ble_peripheral
 * folder.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "bsp_btn_ble.h"
#include "ble.h"

#include "ble_hci.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_db_discovery.h"
#include "ble_lbs_c.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_ble_scan.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#include "app_error.h"
#include "app_uart.h"
#include "app_util.h"
#include "ble_gap.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "ble_nus_c.h"

#define UART_TX_BUF_SIZE        256                                     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        256                                     /**< UART RX buffer size. */

#define NUS_SERVICE_UUID_TYPE   BLE_UUID_TYPE_VENDOR_BEGIN              /**< UUID type for the Nordic UART Service (vendor specific). */

#define ECHOBACK_BLE_UART_DATA  0                                       /**< Echo the UART data that is received over the Nordic UART Service (NUS) back to the sender. */



#define APP_BLE_CONN_CFG_TAG      1                                     /**< Tag that refers to the BLE stack configuration that is set with @ref sd_ble_cfg_set. The default tag is @ref APP_BLE_CONN_CFG_TAG. */
#define APP_BLE_OBSERVER_PRIO     3                                     /**< BLE observer priority of the application. There is no need to modify this value. */

#define CENTRAL_SCANNING_LED      BSP_BOARD_LED_0
#define CENTRAL_CONNECTED_LED     BSP_BOARD_LED_1
#define LEDBUTTON_LED             BSP_BOARD_LED_2                       /**< LED to indicate a change of state of the Button characteristic on the peer. */

#define LEDBUTTON_BUTTON          BSP_BUTTON_0                          /**< Button that writes to the LED characteristic of the peer. */
#define BUTTON_DETECTION_DELAY    APP_TIMER_TICKS(50)                   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */



#define NUM_ADC_CHANS 		(4) // AIN4-7 for resistance, connected to MUX
//#define NUM_ADC_CHANS_PER_SWEEP 1
#define SAMPLES_IN_BUFFER       (NUM_ADC_CHANS+1) // AIN4-7 for resistance + AIN0 for ECG
#define MUX_NUM_CHANNNELS 	(8)
#define MUX_LAST_CHANNEL (NUM_ADC_CHANS * MUX_NUM_CHANNNELS - 1)

#define AVG_NUM_SAMPLES			(2.0)
#define ADC_TRIGGER_INTERVAL_MS     	3//(3.0)
#define MAX_MSG_SIZE_BYTES     		(20)
#define MAX_MSG_PAYLOAD_SIZE_BYTES      (MAX_MSG_SIZE_BYTES - 2)
#define SER_STR_SIZE			(100)
#define Send_Pack_SIZE			(90)
char stringSerial[SER_STR_SIZE];
uint8_t Length_send;

BLE_NUS_C_ARRAY_DEF(m_ble_nus_c, NRF_SDH_BLE_CENTRAL_LINK_COUNT);       /**< BLE Nordic UART Service (NUS) client instance. */
NRF_BLE_GATT_DEF(m_gatt);                                               /**< GATT module instance. */
BLE_LBS_C_ARRAY_DEF(m_lbs_c, NRF_SDH_BLE_CENTRAL_LINK_COUNT);           /**< LED button client instances. */
BLE_DB_DISCOVERY_ARRAY_DEF(m_db_disc, NRF_SDH_BLE_CENTRAL_LINK_COUNT);  /**< Database discovery module instances. */
NRF_BLE_SCAN_DEF(m_scan);                                               /**< Scanning Module instance. */
NRF_BLE_GQ_DEF(m_ble_gatt_queue,                                        /**< BLE GATT Queue instance. */
               NRF_SDH_BLE_CENTRAL_LINK_COUNT,
               NRF_BLE_GQ_QUEUE_SIZE);

static char const m_target_periph_name[] = "Nordic_UART";             /**< Name of the device to try to connect to. This name is searched for in the scanning report data. */
static uint16_t m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH; /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */





typedef enum 
{
    RX_STANDBY,
    RX_TRANSMISSION
}RX_STATE;

static RX_STATE state;
static int msgIndex, numMsgs;
//static int numStringsPrinted = 0;
static char serString[250];


/**@brief NUS UUID. */
static ble_uuid_t const m_nus_uuid =
{
    .uuid = BLE_UUID_NUS_SERVICE,
    .type = NUS_SERVICE_UUID_TYPE
};

/**@brief Function for handling asserts in the SoftDevice.
 *
 * @details This function is called in case of an assert in the SoftDevice.
 *
 * @warning This handler is only an example and is not meant for the final product. You need to analyze
 *          how your product is supposed to react in case of an assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing assert call.
 * @param[in] p_file_name  File name of the failing assert call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}


/**@brief Function for handling characters received by the Nordic UART Service (NUS).
 *
 * @details This function takes a list of characters of length data_len and prints the characters out on UART.
 *          If @ref ECHOBACK_BLE_UART_DATA is set, the data is sent back to sender.
 */
static void ble_nus_chars_received_uart_print(uint8_t * p_data, uint16_t data_len)
{
    ret_code_t ret_val;

    NRF_LOG_DEBUG("Receiving data.\r\n");
    NRF_LOG_HEXDUMP_DEBUG(p_data, data_len);
    uint8_t index=0;
    /*for (uint32_t i = 0; i < data_len; i++)
    {
        while (app_uart_put(p_data[i]) != NRF_SUCCESS);
    }
    while (app_uart_put('\r') != NRF_SUCCESS);
    while (app_uart_put('\n') != NRF_SUCCESS);*/

    switch (state)
    {
        case RX_STANDBY: // Waiting for the first message chunk
        {
            if (p_data[0] == '?') // first symbol in the first message, marks the start of the sequence
            {
                strncpy(serString, "", sizeof(serString));
                state = RX_TRANSMISSION;
                numMsgs = (int)p_data[1];

                //strncpy(serString, p_data+2, data_len-2); // first three symbols are part of the header

                for(index=0;index<data_len-2;index++)
                {
                  serString[index]= p_data[index+2];
                }

                //printf("P%d,%d|%s",numMsgs,data_len, serString);
                //printf("%s",serString);
                for (uint32_t i = 0; i < data_len-2; i++)
                {
                    while (app_uart_put(serString[i]) != NRF_SUCCESS);
                }
		msgIndex = 2;
            }
            else
            {
                for (uint32_t i = 0; i < data_len; i++)
                {
                    while (app_uart_put(p_data[i]) != NRF_SUCCESS);
                }
                while (app_uart_put('\r') != NRF_SUCCESS);
                while (app_uart_put('\n') != NRF_SUCCESS);
            }
            break;
        }
        case RX_TRANSMISSION: // Collecting further message chunks
        {
            if (p_data[0] == '!') // first symbol in the last chunk, marks the end of the sequence
            {
                 // send the string to the UART, reset the state
                 strncpy(serString, "", sizeof(serString));
                 //strncpy(serString, p_data+2, data_len-2); // ignore 2 symbols in the header
                 for(index=0;index<data_len-2;index++)
                 {
                   serString[index]= p_data[index+2];
                 }
                 //printf("%s\r\n", serString);
                 for (uint32_t i = 0; i < data_len-2; i++)
                 {
                     while (app_uart_put(serString[i]) != NRF_SUCCESS);
                 }
                 while (app_uart_put('\r') != NRF_SUCCESS);
                 while (app_uart_put('\n') != NRF_SUCCESS);
                 state = RX_STANDBY;
            }
            else if (p_data[0] == ':') // marks the current message index
            {
                numMsgs = (int)p_data[1];
                if (numMsgs == msgIndex)
                {
                    strncpy(serString, "", sizeof(serString));
                    //strncpy(serString, p_data+2, data_len-2); // ignore 2 symbols in the header
                    for(index=0;index<data_len-2;index++)
                    {
                      serString[index]= p_data[index+2];
                    }
                    //printf("%s",serString);
                    for (uint32_t i = 0; i < data_len-2; i++)
                    {
                        while (app_uart_put(serString[i]) != NRF_SUCCESS);
                    }
                    //printf("P%d,%d|%s",numMsgs,data_len, serString);
                    msgIndex++;
                }
                else
                {
                  //printf("ERROR: Invalid packet received!\n");
                  return; // Invalid packet received. TODO: log the error.
                }
            }
            else
            {
                //printf("ERR:Expected:%d,received:%c%d\n",msgIndex,p_data[0],(int)p_data[1]);
                return;     // Corrupted/invalid packet received, TODO: log the error.
            }
            break;
        }
    }


    if (ECHOBACK_BLE_UART_DATA)
    {
        // Send data back to the peripheral.
        for (uint32_t i = 0; i< NRF_SDH_BLE_CENTRAL_LINK_COUNT; i++)
        {
          do
          {
              ret_val = ble_nus_c_string_send(&m_ble_nus_c[i], p_data, data_len);
              if ((ret_val != NRF_SUCCESS) && (ret_val != NRF_ERROR_BUSY))
              {
                  NRF_LOG_ERROR("Failed sending NUS message. Error 0x%x. \r\n", ret_val);
                  //printf("Failed sending NUS message. Error 0x%x. \r\n", ret_val);
                  APP_ERROR_CHECK(ret_val);
              }
          } while (ret_val == NRF_ERROR_BUSY);
        }
    }
}

/**@brief Function for handling the LED Button Service Client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void lbs_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling the Nordic UART Service Client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nus_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    bsp_board_init(BSP_INIT_LEDS);
}


void Uart_Data_Dummy(void)
{
int16_t r[8] ;
int16_t ab[4] ;
char BNO[100];
char StrBNO[100];

float avgSensRes = 0.0;
uint32_t avgSensRes_int;
float ecgBuf[MUX_NUM_CHANNNELS] = {0}; // for ecg buffer
char strTemp1[35],strTemp2[35],strTemp[35];

uint16_t Numb= 17772;//0x456c
uint8_t Index_Str=0;
uint8_t i;

  for (i = 0; i < NUM_ADC_CHANS * MUX_NUM_CHANNNELS; i++)
  {             
  //avgSensRes = (float)(sensResBuf[i] / AVG_NUM_SAMPLES);
  //avgSensRes_int=(int)(avgSensRes*10);
  avgSensRes_int=Numb;
  Index_Str=i*2;
  stringSerial[Index_Str]=avgSensRes_int & 0xFF;
  stringSerial[Index_Str+1]=((avgSensRes_int>>8) & 0xFF);
  }

  i = NUM_ADC_CHANS * MUX_NUM_CHANNNELS;
  avgSensRes_int=65556;  //number should be more than 65536 and less than 2^32
  Index_Str=i*2;
  stringSerial[Index_Str]=avgSensRes_int & 0xFF;
  stringSerial[Index_Str+1]=((avgSensRes_int>>8) & 0xFF);
  stringSerial[Index_Str+2]=((avgSensRes_int>>16) & 0xFF);
  stringSerial[Index_Str+3]=((avgSensRes_int>>24) & 0xFF);

  ab[0]= 1234;
  ab[1]= 5678;
  ab[2]= 9123;
  r[0]= 1234;
  r[1]= 4567;
  r[2]= 7895;
  r[3]= 4563;
  r[4]= 2156;
  r[5]= 5463;
  r[6]= 7852;
  r[7]= 1596;

  stringSerial[Index_Str+4]=ab[0] & 0xFF;
  stringSerial[Index_Str+5]=((ab[0]>>8) & 0xFF);
  stringSerial[Index_Str+6]=ab[1] & 0xFF;
  stringSerial[Index_Str+7]=((ab[1]>>8) & 0xFF);
  stringSerial[Index_Str+8]=ab[2] & 0xFF;
  stringSerial[Index_Str+9]=((ab[2]>>8) & 0xFF);
  stringSerial[Index_Str+10]=r[0] & 0xFF;
  stringSerial[Index_Str+11]=((r[0]>>8) & 0xFF);
  stringSerial[Index_Str+12]=r[1] & 0xFF;
  stringSerial[Index_Str+13]=((r[1]>>8) & 0xFF);
  stringSerial[Index_Str+14]=r[2] & 0xFF;
  stringSerial[Index_Str+15]=((r[2]>>8) & 0xFF);
  stringSerial[Index_Str+16]=r[3] & 0xFF;
  stringSerial[Index_Str+17]=((r[3]>>8) & 0xFF);
  stringSerial[Index_Str+18]=r[4] & 0xFF;
  stringSerial[Index_Str+19]=((r[4]>>8) & 0xFF);
  stringSerial[Index_Str+20]=r[5] & 0xFF;
  stringSerial[Index_Str+21]=((r[5]>>8) & 0xFF);
  stringSerial[Index_Str+22]=r[6] & 0xFF;
  stringSerial[Index_Str+23]=((r[6]>>8) & 0xFF);
  stringSerial[Index_Str+24]=r[7] & 0xFF;
  stringSerial[Index_Str+25]=((r[7]>>8) & 0xFF);

  Length_send=Index_Str+26;
}
void Uart_Send_Dummy(void)
{
  for (uint32_t i = 0; i < Length_send; i++)
  {
    while (app_uart_put(stringSerial[i]) != NRF_SUCCESS);
  }
  while (app_uart_put('\r') != NRF_SUCCESS);
  while (app_uart_put('\n') != NRF_SUCCESS);
}
/**@brief   Function for handling app_uart events.
 *
 * @details This function receives a single character from the app_uart module and appends it to
 *          a string. The string is sent over BLE when the last character received is a
 *          'new line' '\n' (hex 0x0A) or if the string reaches the maximum data length.
 */
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint16_t index = 0;
    uint32_t ret_val;

    switch (p_event->evt_type)
    {
        /**@snippet [Handling data from UART] */
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;

            if ((data_array[index - 1] == '\n') ||
                (data_array[index - 1] == '\r') ||
                (index >= (m_ble_nus_max_data_len)))
            {
              if(data_array[0]=='*' && data_array[1]==';')
              {
                Uart_Send_Dummy();
              }
              else
              {
                NRF_LOG_DEBUG("Ready to send data over BLE NUS");

                for (uint32_t i = 0; i< NRF_SDH_BLE_CENTRAL_LINK_COUNT; i++)
                {
                    do
                    {
                        ret_val = ble_nus_c_string_send(&m_ble_nus_c[i], data_array, index);
                        if ( (ret_val != NRF_ERROR_INVALID_STATE) && (ret_val != NRF_ERROR_RESOURCES) )
                        {
                            APP_ERROR_CHECK(ret_val);
                        }
                    } while (ret_val == NRF_ERROR_RESOURCES);
                }
              }
              index = 0;

              /*NRF_LOG_DEBUG("Ready to send data over BLE NUS");

              for (uint32_t i = 0; i< NRF_SDH_BLE_CENTRAL_LINK_COUNT; i++)
              {
                  do
                  {
                      ret_val = ble_nus_c_string_send(&m_ble_nus_c[i], data_array, index);
                      if ( (ret_val != NRF_ERROR_INVALID_STATE) && (ret_val != NRF_ERROR_RESOURCES) )
                      {
                          APP_ERROR_CHECK(ret_val);
                      }
                  } while (ret_val == NRF_ERROR_RESOURCES);
              }
              index = 0;*/
            }
            break;

        /**@snippet [Handling data from UART] */
        case APP_UART_COMMUNICATION_ERROR:
            NRF_LOG_ERROR("Communication error occurred while handling UART.\r\n");
            //printf("Communication error occurred while handling UART.\r\n");
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            NRF_LOG_ERROR("Error occurred in FIFO module used by UART.\r\n");
            //printf("Error occurred in FIFO module used by UART.\r\n");
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}

/**@brief Function for initializing the UART. */
static void uart_init(void)
{
    ret_code_t err_code;

    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud1M
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);

    APP_ERROR_CHECK(err_code);
}


static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
    ret_code_t err_code;

    switch(p_scan_evt->scan_evt_id)
    {
        case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
        {
            err_code = p_scan_evt->params.connecting_err.err_code;
            APP_ERROR_CHECK(err_code);
        } break;

        case NRF_BLE_SCAN_EVT_CONNECTED:
         {
              ble_gap_evt_connected_t const * p_connected =
                               p_scan_evt->params.connected.p_connected;
             // Scan is automatically stopped by the connection.
             NRF_LOG_INFO("Connecting to target %02x%02x%02x%02x%02x%02x\r\n",
                      p_connected->peer_addr.addr[0],
                      p_connected->peer_addr.addr[1],
                      p_connected->peer_addr.addr[2],
                      p_connected->peer_addr.addr[3],
                      p_connected->peer_addr.addr[4],
                      p_connected->peer_addr.addr[5]
                      );
              /*printf("Connecting to target %02x%02x%02x%02x%02x%02x\r\n",
                      p_connected->peer_addr.addr[0],
                      p_connected->peer_addr.addr[1],
                      p_connected->peer_addr.addr[2],
                      p_connected->peer_addr.addr[3],
                      p_connected->peer_addr.addr[4],
                      p_connected->peer_addr.addr[5]
                      );*/
         } break;

        default:
            break;
    }
}


/**@brief Function for initializing the scanning and setting the filters.
 */
static void scan_init(void)
{
    ret_code_t          err_code;
    nrf_ble_scan_init_t init_scan;

    memset(&init_scan, 0, sizeof(init_scan));

    init_scan.connect_if_match = true;
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_NAME_FILTER, m_target_periph_name);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_NAME_FILTER, false);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting scanning. */
static void scan_start(void)
{
    ret_code_t ret;

    NRF_LOG_INFO("Start scanning for device name %s.\r\n", (uint32_t)m_target_periph_name);
    //printf("Start scanning for device name %s.\r\n", (uint32_t)m_target_periph_name);
    ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);
    // Turn on the LED to signal scanning.
    bsp_board_led_on(CENTRAL_SCANNING_LED);
}


/**@brief Handles events coming from the LED Button central module.
 *
 * @param[in] p_lbs_c     The instance of LBS_C that triggered the event.
 * @param[in] p_lbs_c_evt The LBS_C event.
 */
static void lbs_c_evt_handler(ble_lbs_c_t * p_lbs_c, ble_lbs_c_evt_t * p_lbs_c_evt)
{
    switch (p_lbs_c_evt->evt_type)
    {
        case BLE_LBS_C_EVT_DISCOVERY_COMPLETE:
        {
            ret_code_t err_code;

            NRF_LOG_INFO("LED Button Service discovered on conn_handle 0x%x\r\n",
                         p_lbs_c_evt->conn_handle);
            //printf("LED Button Service discovered on conn_handle 0x%x\r\n",p_lbs_c_evt->conn_handle);
            err_code = app_button_enable();
            APP_ERROR_CHECK(err_code);

            // LED Button Service discovered. Enable notification of Button.
            err_code = ble_lbs_c_button_notif_enable(p_lbs_c);
            APP_ERROR_CHECK(err_code);
        } break; // BLE_LBS_C_EVT_DISCOVERY_COMPLETE

        case BLE_LBS_C_EVT_BUTTON_NOTIFICATION:
        {
            NRF_LOG_INFO("Link 0x%x, Button state changed on peer to 0x%x\r\n",
                         p_lbs_c_evt->conn_handle,
                         p_lbs_c_evt->params.button.button_state);
            //printf("Link 0x%x, Button state changed on peer to 0x%x\r\n",p_lbs_c_evt->conn_handle,p_lbs_c_evt->params.button.button_state);
            if (p_lbs_c_evt->params.button.button_state)
            {
                bsp_board_led_on(LEDBUTTON_LED);
            }
            else
            {
                bsp_board_led_off(LEDBUTTON_LED);
            }
        } break; // BLE_LBS_C_EVT_BUTTON_NOTIFICATION

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code;

    // For readability.
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        // Upon connection, check which peripheral is connected, initiate DB
        // discovery, update LEDs status, and resume scanning, if necessary.
        case BLE_GAP_EVT_CONNECTED:
        {
            NRF_LOG_INFO("Connection 0x%x established, starting DB discovery.\r\n",
                         p_gap_evt->conn_handle);
            //printf("Connection 0x%x established, starting DB discovery.\r\n",p_gap_evt->conn_handle);

            APP_ERROR_CHECK_BOOL(p_gap_evt->conn_handle < NRF_SDH_BLE_CENTRAL_LINK_COUNT);

            /*err_code = ble_lbs_c_handles_assign(&m_lbs_c[p_gap_evt->conn_handle],
                                                p_gap_evt->conn_handle,
                                                NULL);*/

            err_code = ble_nus_c_handles_assign(&m_ble_nus_c[p_gap_evt->conn_handle],
                                                p_gap_evt->conn_handle,
                                                NULL);


            APP_ERROR_CHECK(err_code);

            err_code = ble_db_discovery_start(&m_db_disc[p_gap_evt->conn_handle],
                                              p_gap_evt->conn_handle);
            APP_ERROR_CHECK(err_code);

            // Update LEDs status and check whether it is needed to look for more
            // peripherals to connect to.
            bsp_board_led_on(CENTRAL_CONNECTED_LED);
            if (ble_conn_state_central_conn_count() == NRF_SDH_BLE_CENTRAL_LINK_COUNT)
            {
                bsp_board_led_off(CENTRAL_SCANNING_LED);
            }
            else
            {
                // Resume scanning.
                bsp_board_led_on(CENTRAL_SCANNING_LED);
                scan_start();
            }
        } break; // BLE_GAP_EVT_CONNECTED

        // Upon disconnection, reset the connection handle of the peer that disconnected, update
        // the LEDs status and start scanning again.
        case BLE_GAP_EVT_DISCONNECTED:
        {
            NRF_LOG_INFO("LBS central link 0x%x disconnected (reason: 0x%x)\r\n",
                         p_gap_evt->conn_handle,
                         p_gap_evt->params.disconnected.reason);
            //printf("LBS central link 0x%x disconnected (reason: 0x%x)\r\n",p_gap_evt->conn_handle,p_gap_evt->params.disconnected.reason);
            if (ble_conn_state_central_conn_count() == 0)
            {
                err_code = app_button_disable();
                APP_ERROR_CHECK(err_code);

                // Turn off the LED that indicates the connection.
                bsp_board_led_off(CENTRAL_CONNECTED_LED);
            }

            // Start scanning.
            scan_start();

            // Turn on the LED for indicating scanning.
            bsp_board_led_on(CENTRAL_SCANNING_LED);

        } break;

        case BLE_GAP_EVT_TIMEOUT:
        {
            // Timeout for scanning is not specified, so only the connection requests can time out.
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
                NRF_LOG_DEBUG("Connection request timed out.\r\n");
                //printf("Connection request timed out.\r\n");
            }
        } break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST.\r\n");
            //printf("BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST.\r\n");
            // Accept parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                        &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.\r\n");
            //printf("PHY update request.\r\n");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
        {
            // Disconnect on GATT client timeout event.
            NRF_LOG_DEBUG("GATT client timeout.\r\n");
            //printf("GATT client timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_TIMEOUT:
        {
            // Disconnect on GATT server timeout event.
            NRF_LOG_DEBUG("GATT server timeout.\r\n");
            //printf("GATT server timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
        } break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief LED Button collector initialization. */
static void lbs_c_init(void)
{
    ret_code_t       err_code;
    ble_lbs_c_init_t lbs_c_init_obj;

    lbs_c_init_obj.evt_handler   = lbs_c_evt_handler;
    lbs_c_init_obj.p_gatt_queue  = &m_ble_gatt_queue;
    lbs_c_init_obj.error_handler = lbs_error_handler;

    for (uint32_t i = 0; i < NRF_SDH_BLE_CENTRAL_LINK_COUNT; i++)
    {
        err_code = ble_lbs_c_init(&m_lbs_c[i], &lbs_c_init_obj);
        APP_ERROR_CHECK(err_code);
    }
}



/**@brief Callback handling Nordic UART Service (NUS) client events.
 *
 * @details This function is called to notify the application of NUS client events.
 *
 * @param[in]   p_ble_nus_c   NUS client handle. This identifies the NUS client.
 * @param[in]   p_ble_nus_evt Pointer to the NUS client event.
 */

/**@snippet [Handling events from the ble_nus_c module] */
static void ble_nus_c_evt_handler(ble_nus_c_t * p_ble_nus_c, ble_nus_c_evt_t const * p_ble_nus_evt)
{
    ret_code_t err_code;

    switch (p_ble_nus_evt->evt_type)
    {
        case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
            NRF_LOG_INFO("Discovery complete.\r\n");
            //printf("Discovery complete.\r\n");

            //err_code = ble_nus_c_handles_assign(p_ble_nus_c, p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
            //APP_ERROR_CHECK(err_code);

            //err_code = ble_nus_c_handles_assign(&m_ble_nus_c[p_ble_nus_evt->conn_handle], p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
            err_code = ble_nus_c_handles_assign(&m_ble_nus_c[p_ble_nus_evt->conn_handle], p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
            APP_ERROR_CHECK(err_code);

            err_code = ble_nus_c_tx_notif_enable(p_ble_nus_c);
            APP_ERROR_CHECK(err_code);
            NRF_LOG_INFO("Connected to device with Nordic UART Service. 0x%x \r\n",p_ble_nus_evt->conn_handle);
            //printf("Connected to device with Nordic UART Service. 0x%x \r\n",p_ble_nus_evt->conn_handle);
            break;

        case BLE_NUS_C_EVT_NUS_TX_EVT:
            //printf("Data recieved\r\n");
            ble_nus_chars_received_uart_print(p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);
            break;

        case BLE_NUS_C_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected.\r\n");
            //printf("Disconnected.\r\n");
            scan_start();
            break;
    }
}

/**@brief Function for initializing the Nordic UART Service (NUS) client. */
static void nus_c_init(void)
{
    ret_code_t       err_code;
    ble_nus_c_init_t init;

    init.evt_handler   = ble_nus_c_evt_handler;
    init.error_handler = nus_error_handler;
    init.p_gatt_queue  = &m_ble_gatt_queue;

    for (uint32_t i = 0; i < NRF_SDH_BLE_CENTRAL_LINK_COUNT; i++)
    {
      err_code = ble_nus_c_init(&m_ble_nus_c[i], &init);
      APP_ERROR_CHECK(err_code);
    }
}



/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupts.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for writing to the LED characteristic of all connected clients.
 *
 * @details Based on whether the button is pressed or released, this function writes a high or low
 *          LED status to the server.
 *
 * @param[in] button_action The button action (press or release).
 *            Determines whether the LEDs of the servers are ON or OFF.
 *
 * @return If successful, NRF_SUCCESS is returned. Otherwise, returns the error code from @ref ble_lbs_led_status_send.
 */
static ret_code_t led_status_send_to_all(uint8_t button_action)
{
    ret_code_t err_code;

    for (uint32_t i = 0; i< NRF_SDH_BLE_CENTRAL_LINK_COUNT; i++)
    {
        err_code = ble_lbs_led_status_send(&m_lbs_c[i], button_action);
        if (err_code != NRF_SUCCESS &&
            err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
            err_code != NRF_ERROR_INVALID_STATE)
        {
            return err_code;
        }
    }
        return NRF_SUCCESS;
}

/**
 * @brief Function for handling shutdown events.
 *
 * @param[in]   event       Shutdown type.
 */
/*static bool shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            // Prepare wakeup buttons.
            err_code = bsp_btn_ble_sleep_mode_prepare();
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }

    return true;
}

NRF_PWR_MGMT_HANDLER_REGISTER(shutdown_handler, APP_SHUTDOWN_HANDLER_PRIORITY);*/


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press or release).
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

    switch (pin_no)
    {
        case LEDBUTTON_BUTTON:
            err_code = led_status_send_to_all(button_action);
            if (err_code == NRF_SUCCESS)
            {
                NRF_LOG_INFO("LBS write LED state %d\r\n", button_action);
                //printf("LBS write LED state %d\r\n", button_action);
            }
            break;

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}





/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    ret_code_t err_code;

   // The array must be static because a pointer to it is saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {LEDBUTTON_BUTTON, false, BUTTON_PULL, button_event_handler}
    };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons), BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling database discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function forwards the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    NRF_LOG_DEBUG("call to ble_nus_on_db_disc_evt for instance %d and link 0x%x!\r\n",
                  p_evt->conn_handle,
                  p_evt->conn_handle);
    //printf("call to ble_nus_on_db_disc_evt for instance %d and link 0x%x!\r\n",p_evt->conn_handle,p_evt->conn_handle);

    ble_nus_c_on_db_disc_evt(&m_ble_nus_c[p_evt->conn_handle], p_evt);
}








/** @brief Database discovery initialization.
 */
static void db_discovery_init(void)
{
    ble_db_discovery_init_t db_init;

    memset(&db_init, 0, sizeof(ble_db_discovery_init_t));

    db_init.evt_handler  = db_disc_handler;
    db_init.p_gatt_queue = &m_ble_gatt_queue;

    ret_code_t err_code = ble_db_discovery_init(&db_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)
    {
        NRF_LOG_INFO("ATT MTU exchange completed.\r\n");
        //printf("ATT MTU exchange completed.\r\n");

        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Ble NUS max data length set to 0x%X(%d)\r\n", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
        //printf("Ble NUS max data length set to 0x%X(%d)\r\n", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details This function handles any pending log operations, then sleeps until the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/** @brief Function for initializing the log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/** @brief Function for initializing the timer.
 */
static void timer_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);//Maybe to gatt_evt_handler
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_central_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}


int main(void)
{
    // Initialize.
    //log_init();
    state = RX_STANDBY;

    Uart_Data_Dummy();

    timer_init();
    uart_init();
    

    //printf("started.\r\n");

    leds_init();
    buttons_init();
    power_management_init();

    ble_stack_init();
    gatt_init();
    db_discovery_init();
    //lbs_c_init();
    nus_c_init();
    ble_conn_state_init();
    scan_init();

    // Start execution.
    printf("Multilink example started.\r\n");
    //NRF_LOG_INFO("Multilink example started.");
    
    scan_start();
    

    for (;;)
    {
        idle_state_handle();
    }
}
