#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nordic_common.h"
#include "app_error.h"
#include "app_uart.h"
#include "ble_db_discovery.h"
#include "app_timer.h"
#include "app_util.h"
#include "bsp_btn_ble.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_hci.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_power.h"
#include "ble_advdata.h"
#include "ble_nus_c.h"
#include "nrf_ble_gatt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

/**************** BLE DEFINES ***************/
#define APP_BLE_CONN_CFG_TAG    1                                       /**< A tag that refers to the BLE stack configuration we set with @ref sd_ble_cfg_set. Default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */
#define APP_BLE_OBSERVER_PRIO   1                                       /**< Application's BLE observer priority. You shoulnd't need to modify this value. */

#define UART_TX_BUF_SIZE        256                                     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        256                                     /**< UART RX buffer size. */

#define NUS_SERVICE_UUID_TYPE   BLE_UUID_TYPE_VENDOR_BEGIN              /**< UUID type for the Nordic UART Service (vendor specific). */

#define SCAN_INTERVAL           0x00A0                                  /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW             0x0050                                  /**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_TIMEOUT            0x0000                                  /**< Timout when scanning. 0x0000 disables timeout. */

#define MIN_CONNECTION_INTERVAL MSEC_TO_UNITS(20, UNIT_1_25_MS)         /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL MSEC_TO_UNITS(75, UNIT_1_25_MS)         /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY           0                                       /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT     MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Determines supervision time-out in units of 10 millisecond. */

#define UUID16_SIZE             2                                       /**< Size of 16 bit UUID */
#define UUID32_SIZE             4                                       /**< Size of 32 bit UUID */
#define UUID128_SIZE            16                                      /**< Size of 128 bit UUID */

#define ECHOBACK_BLE_UART_DATA  0                                       /**< Echo the UART data that is received over the Nordic UART Service back to the sender. */

/**************** SAADC DEFINES ***************/
#define NUM_MILLIVOLT_PER_LSB (3.6 / 4096)
#define R_BIAS 								1355
#define V_BIAS 								1.25
#define GAIN 									8.925

#define NUM_ADC_CHANS 				3
//#define NUM_ADC_CHANS_PER_SWEEP 1
#define SAMPLES_IN_BUFFER NUM_ADC_CHANS
#define MUX_NUM_CHANNNELS 		8
#define MUX_LAST_CHANNEL (SAMPLES_IN_BUFFER * MUX_NUM_CHANNNELS - 1)
#define PIN_MUX_A 						11
#define PIN_MUX_B 						12
#define PIN_MUX_C 						13
#define PIN_MUX_EN 						14

#define AVG_NUM_SAMPLES			 		(3)
#define MAX_MSG_SIZE_BYTES     			(20)
#define MAX_MSG_PAYLOAD_SIZE_BYTES      (MAX_MSG_SIZE_BYTES - 2)
#define NUM_BYTES_TO_SEND		 	 	(7)
#define SER_STR_SIZE					(200)

/**************** BLE Private Variables ***************/
BLE_NUS_C_DEF(m_ble_nus_c);                                             /**< BLE NUS service client instance. */
NRF_BLE_GATT_DEF(m_gatt);                                               /**< GATT module instance. */
BLE_DB_DISCOVERY_DEF(m_db_disc);                                        /**< DB discovery module instance. */

static uint16_t m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH; /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
//const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;

/**************** SAADC Private Variables ***************/
static float ConvertAdcReadingToVoltage (uint32_t adcReading);
static float ConvertVoltageToResistance (float adcChanVoltage);
static void ConfigureMuxGpio(uint32_t muxSweepCount);

volatile uint8_t state = 1;
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(3);
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              muxSweepCount = 0;
static uint32_t				 avgCount = 0;
uint32_t sensResBuf[NUM_ADC_CHANS * MUX_NUM_CHANNNELS] = {0};


void saadc_sampling_event_enable(void)
{
	ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

	APP_ERROR_CHECK(err_code);
}

/**@brief Connection parameters requested for connection. */
static ble_gap_conn_params_t const m_connection_param =
{
	(uint16_t)MIN_CONNECTION_INTERVAL,  // Minimum connection
	(uint16_t)MAX_CONNECTION_INTERVAL,  // Maximum connection
	(uint16_t)SLAVE_LATENCY,            // Slave latency
	(uint16_t)SUPERVISION_TIMEOUT       // Supervision time-out
};

/** @brief Parameters used when scanning. */
static ble_gap_scan_params_t const m_scan_params =
{
	.active   = 1,
	.interval = SCAN_INTERVAL,
	.window   = SCAN_WINDOW,
	.timeout  = SCAN_TIMEOUT,
	#if (NRF_SD_BLE_API_VERSION <= 2)
		.selective   = 0,
		.p_whitelist = NULL,
	#endif
	#if (NRF_SD_BLE_API_VERSION >= 3)
		.use_whitelist = 0,
	#endif
};

/**@brief NUS uuid. */
static ble_uuid_t const m_nus_uuid =
{
	.uuid = BLE_UUID_NUS_SERVICE,
	.type = NUS_SERVICE_UUID_TYPE
};


/**@brief Function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
	app_error_handler(0xDEADBEEF, line_num, p_file_name);
}


/**@brief Function to start scanning. */
static void scan_start(void)
{
	ret_code_t ret;

	ret = sd_ble_gap_scan_start(&m_scan_params);
	APP_ERROR_CHECK(ret);

	ret = bsp_indication_set(BSP_INDICATE_SCANNING);
	APP_ERROR_CHECK(ret);
}


/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
	ble_nus_c_on_db_disc_evt(&m_ble_nus_c, p_evt);
}


/**@brief Function for handling characters received by the Nordic UART Service.
 *
 * @details This function takes a list of characters of length data_len and prints the characters out on UART.
 *          If @ref ECHOBACK_BLE_UART_DATA is set, the data is sent back to sender.
 */
static void ble_nus_chars_received_uart_print(uint8_t * p_data, uint16_t data_len)
{
	ret_code_t ret_val;

	NRF_LOG_DEBUG("Receiving data.");
	NRF_LOG_HEXDUMP_DEBUG(p_data, data_len);

	for (uint32_t i = 0; i < data_len; i++)
	{
		do
		{
			ret_val = app_uart_put(p_data[i]);
			if ((ret_val != NRF_SUCCESS) && (ret_val != NRF_ERROR_BUSY))
			{
				NRF_LOG_ERROR("app_uart_put failed for index 0x%04x.", i);
				APP_ERROR_CHECK(ret_val);
			}
		} while (ret_val == NRF_ERROR_BUSY);
	}
	if (p_data[data_len-1] == '\r')
	{
		while (app_uart_put('\n') == NRF_ERROR_BUSY);
	}
	if (ECHOBACK_BLE_UART_DATA)
	{
		// Send data back to peripheral.
		do
		{
			ret_val = ble_nus_c_string_send(&m_ble_nus_c, p_data, data_len);
			if ((ret_val != NRF_SUCCESS) && (ret_val != NRF_ERROR_BUSY))
			{
				NRF_LOG_ERROR("Failed sending NUS message. Error 0x%x. ", ret_val);
				APP_ERROR_CHECK(ret_val);
			}
		} while (ret_val == NRF_ERROR_BUSY);
	}
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
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

			if ((data_array[index - 1] == '\n') || (index >= (m_ble_nus_max_data_len)))
			{
				NRF_LOG_DEBUG("Ready to send data over BLE NUS");
				NRF_LOG_HEXDUMP_DEBUG(data_array, index);

				do
				{
					ret_val = ble_nus_c_string_send(&m_ble_nus_c, data_array, index);
					if ( (ret_val != NRF_ERROR_INVALID_STATE) && (ret_val != NRF_ERROR_BUSY) )
					{
						APP_ERROR_CHECK(ret_val);
					}
				} while (ret_val == NRF_ERROR_BUSY);

				index = 0;
			}
			break;

		/**@snippet [Handling data from UART] */
		case APP_UART_COMMUNICATION_ERROR:
			NRF_LOG_ERROR("Communication error occurred while handling UART.");
			APP_ERROR_HANDLER(p_event->data.error_communication);
			break;

		case APP_UART_FIFO_ERROR:
			NRF_LOG_ERROR("Error occurred in FIFO module used by UART.");
			APP_ERROR_HANDLER(p_event->data.error_code);
			break;

		default:
			break;
	}
}


/**@brief Callback handling NUS Client events.
 *
 * @details This function is called to notify the application of NUS client events.
 *
 * @param[in]   p_ble_nus_c   NUS Client Handle. This identifies the NUS client
 * @param[in]   p_ble_nus_evt Pointer to the NUS Client event.
 */

/**@snippet [Handling events from the ble_nus_c module] */
static void ble_nus_c_evt_handler(ble_nus_c_t * p_ble_nus_c, ble_nus_c_evt_t const * p_ble_nus_evt)
{
	ret_code_t err_code;

	switch (p_ble_nus_evt->evt_type)
	{
		case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
			NRF_LOG_INFO("Discovery complete.");
			err_code = ble_nus_c_handles_assign(p_ble_nus_c, p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
			APP_ERROR_CHECK(err_code);

			err_code = ble_nus_c_tx_notif_enable(p_ble_nus_c);
			APP_ERROR_CHECK(err_code);
			NRF_LOG_INFO("Connected to device with Nordic UART Service.");
		
			// Connection to peripheral should be fully established by now, can start the ADC finally
			saadc_sampling_event_enable();
		  printf("\r\nConnection established! ADC started.\r\n");
			break;

		case BLE_NUS_C_EVT_NUS_TX_EVT:
			ble_nus_chars_received_uart_print(p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);
			break;

		case BLE_NUS_C_EVT_DISCONNECTED:
			NRF_LOG_INFO("Disconnected.");
			scan_start();
			break;
	}
}
/**@snippet [Handling events from the ble_nus_c module] */


/**
 * @brief Function for shutdown events.
 *
 * @param[in]   event       Shutdown type.
 */
static bool shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
	ret_code_t err_code;

	err_code = bsp_indication_set(BSP_INDICATE_IDLE);
	APP_ERROR_CHECK(err_code);

	switch (event)
	{
		case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
			// Prepare wakeup buttons.
			//err_code = bsp_btn_ble_sleep_mode_prepare(); UNCOMMENTED THIS *******
			//APP_ERROR_CHECK(err_code); 									 UNCOMMENTED THIS *******
			break;

		default:
			break;
	}

	return true;
}

NRF_PWR_MGMT_HANDLER_REGISTER(shutdown_handler, APP_SHUTDOWN_HANDLER_PRIORITY);

/**@brief Reads an advertising report and checks if a UUID is present in the service list.
 *
 * @details The function is able to search for 16-bit, 32-bit and 128-bit service UUIDs.
 *          To see the format of a advertisement packet, see
 *          https://www.bluetooth.org/Technical/AssignedNumbers/generic_access_profile.htm
 *
 * @param[in]   p_target_uuid The UUID to search for.
 * @param[in]   p_adv_report  Pointer to the advertisement report.
 *
 * @retval      true if the UUID is present in the advertisement report. Otherwise false
 */
static bool is_uuid_present(ble_uuid_t               const * p_target_uuid,
							ble_gap_evt_adv_report_t const * p_adv_report)
{
	ret_code_t   err_code;
	ble_uuid_t   extracted_uuid;
	uint16_t     index  = 0;
	uint8_t    * p_data = (uint8_t *)p_adv_report->data;

	while (index < p_adv_report->dlen)
	{
		uint8_t field_length = p_data[index];
		uint8_t field_type   = p_data[index + 1];

		if (   (field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE)
			|| (field_type == BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE))
		{
			for (uint32_t i = 0; i < (field_length / UUID16_SIZE); i++)
			{
				err_code = sd_ble_uuid_decode(UUID16_SIZE,
											  &p_data[i * UUID16_SIZE + index + 2],
											  &extracted_uuid);

				if (err_code == NRF_SUCCESS)
				{
					if (extracted_uuid.uuid == p_target_uuid->uuid)
					{
						return true;
					}
				}
			}
		}
		else if (   (field_type == BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_MORE_AVAILABLE)
				 || (field_type == BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_COMPLETE))
		{
			for (uint32_t i = 0; i < (field_length / UUID32_SIZE); i++)
			{
				err_code = sd_ble_uuid_decode(UUID32_SIZE,
											  &p_data[i * UUID32_SIZE + index + 2],
											  &extracted_uuid);

				if (err_code == NRF_SUCCESS)
				{
					if (   (extracted_uuid.uuid == p_target_uuid->uuid)
						&& (extracted_uuid.type == p_target_uuid->type))
					{
						return true;
					}
				}
			}
		}

		else if (   (field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE)
				 || (field_type == BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE))
		{
			err_code = sd_ble_uuid_decode(UUID128_SIZE, &p_data[index + 2], &extracted_uuid);
			if (err_code == NRF_SUCCESS)
			{
				if (   (extracted_uuid.uuid == p_target_uuid->uuid)
					&& (extracted_uuid.type == p_target_uuid->type))
				{
					return true;
				}
			}
		}
		index += field_length + 1;
	}
	return false;
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
	ret_code_t            err_code;
	ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

	switch (p_ble_evt->header.evt_id)
	{
		case BLE_GAP_EVT_ADV_REPORT:
		{
			ble_gap_evt_adv_report_t const * p_adv_report = &p_gap_evt->params.adv_report;

			if (is_uuid_present(&m_nus_uuid, p_adv_report))
			{

				err_code = sd_ble_gap_connect(&p_adv_report->peer_addr,
											  &m_scan_params,
											  &m_connection_param,
											  APP_BLE_CONN_CFG_TAG);

				if (err_code == NRF_SUCCESS)
				{
					// scan is automatically stopped by the connect
					err_code = bsp_indication_set(BSP_INDICATE_IDLE);
					APP_ERROR_CHECK(err_code);
					NRF_LOG_INFO("Connecting to target %02x%02x%02x%02x%02x%02x",
							 p_adv_report->peer_addr.addr[0],
							 p_adv_report->peer_addr.addr[1],
							 p_adv_report->peer_addr.addr[2],
							 p_adv_report->peer_addr.addr[3],
							 p_adv_report->peer_addr.addr[4],
							 p_adv_report->peer_addr.addr[5]
							 );
				}
			}
		}break; // BLE_GAP_EVT_ADV_REPORT

		case BLE_GAP_EVT_CONNECTED:
			NRF_LOG_INFO("Connected to target");
			err_code = ble_nus_c_handles_assign(&m_ble_nus_c, p_ble_evt->evt.gap_evt.conn_handle, NULL);
			APP_ERROR_CHECK(err_code);

			err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
			APP_ERROR_CHECK(err_code);

			// start discovery of services. The NUS Client waits for a discovery result
			err_code = ble_db_discovery_start(&m_db_disc, p_ble_evt->evt.gap_evt.conn_handle);
			APP_ERROR_CHECK(err_code);
			break;

		case BLE_GAP_EVT_TIMEOUT:
			if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
			{
				NRF_LOG_INFO("Scan timed out.");
				scan_start();
			}
			else if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
			{
				NRF_LOG_INFO("Connection Request timed out.");
			}
			break;

		case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
			// Pairing not supported
			err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
			APP_ERROR_CHECK(err_code);
			break;

		case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
			// Accepting parameters requested by peer.
			err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
													&p_gap_evt->params.conn_param_update_request.conn_params);
			APP_ERROR_CHECK(err_code);
			break;

#if defined(S132)
		case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
		{
			NRF_LOG_DEBUG("PHY update request.");
			ble_gap_phys_t const phys =
			{
				.rx_phys = BLE_GAP_PHY_AUTO,
				.tx_phys = BLE_GAP_PHY_AUTO,
			};
			err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
			APP_ERROR_CHECK(err_code);
		} break;
#endif

		case BLE_GATTC_EVT_TIMEOUT:
			// Disconnect on GATT Client timeout event.
			NRF_LOG_DEBUG("GATT Client Timeout.");
			err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
											 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
			APP_ERROR_CHECK(err_code);
			break;

		case BLE_GATTS_EVT_TIMEOUT:
			// Disconnect on GATT Server timeout event.
			NRF_LOG_DEBUG("GATT Server Timeout.");
			err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
											 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
			APP_ERROR_CHECK(err_code);
			break;

		default:
			break;
	}
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
	ret_code_t err_code;

	err_code = nrf_sdh_enable_request();
	NRF_LOG_INFO("err_code is %d", err_code);
	APP_ERROR_CHECK(err_code);

	// Configure the BLE stack using the default settings.
	// Fetch the start address of the application RAM.
	uint32_t ram_start = 0;
	err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
	NRF_LOG_INFO("BLE init 2");
	APP_ERROR_CHECK(err_code);

	// Enable BLE stack.
	err_code = nrf_sdh_ble_enable(&ram_start);
	NRF_LOG_INFO("BLE init 3");
	APP_ERROR_CHECK(err_code);

	// Register a handler for BLE events.
	NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
	if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)
	{
		NRF_LOG_INFO("ATT MTU exchange completed.");

		m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
		NRF_LOG_INFO("Ble NUS max data length set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
	}
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
	ret_code_t err_code;

	err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_gatt_att_mtu_central_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
	APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in] event  Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
	ret_code_t err_code;

	switch (event)
	{
		case BSP_EVENT_SLEEP:
			nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
			break;

		case BSP_EVENT_DISCONNECT:
			err_code = sd_ble_gap_disconnect(m_ble_nus_c.conn_handle,
											 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
			if (err_code != NRF_ERROR_INVALID_STATE)
			{
				APP_ERROR_CHECK(err_code);
			}
			break;

		default:
			break;
	}
}

/**@brief Function for initializing the UART. */
static void uart_initt(void)
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

/**@brief Function for initializing the NUS Client. */
static void nus_c_init(void)
{
	ret_code_t       err_code;
	ble_nus_c_init_t init;

	init.evt_handler = ble_nus_c_evt_handler;

	err_code = ble_nus_c_init(&m_ble_nus_c, &init);
	APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds. */
static void buttons_leds_init(void)
{
	ret_code_t err_code;
	bsp_event_t startup_event;

	err_code = bsp_init(BSP_INIT_LED, bsp_event_handler);
	APP_ERROR_CHECK(err_code);

	//err_code = bsp_btn_ble_init(NULL, &startup_event);    UNCOMMENTED THIS ******
	//APP_ERROR_CHECK(err_code);														UNCOMMENTED_THIS ******
}


/**@brief Function for initializing the timer. */
static void timer_init(void)
{
	ret_code_t err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the nrf log module. */
static void log_init(void)
{
	ret_code_t err_code = NRF_LOG_INIT(NULL);
	APP_ERROR_CHECK(err_code);

	NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing the Power manager. */
static void power_init(void)
{
	ret_code_t err_code = nrf_pwr_mgmt_init();
	APP_ERROR_CHECK(err_code);
}


/** @brief Function for initializing the Database Discovery Module. */
static void db_discovery_init(void)
{
	ret_code_t err_code = ble_db_discovery_init(db_disc_handler);
	APP_ERROR_CHECK(err_code);
}

/**************** SAADC PRIVATE FUNCTIONS ***************/
void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
}


void saadc_sampling_event_init(void)
{
	ret_code_t err_code;

	err_code = nrf_drv_ppi_init();
	APP_ERROR_CHECK(err_code);

	nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
	timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
	err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
	APP_ERROR_CHECK(err_code);

	/* setup m_timer for compare event every 50ms */
	uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 6);
	nrf_drv_timer_extended_compare(&m_timer,
								   NRF_TIMER_CC_CHANNEL0,
								   ticks,
								   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
								   false);
	nrf_drv_timer_enable(&m_timer);

	uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
																				NRF_TIMER_CC_CHANNEL0);
	uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

	/* setup ppi channel so that timer compare event is triggering sample task in SAADC */
	err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
										  timer_compare_event_addr,
										  saadc_sample_task_addr);
	APP_ERROR_CHECK(err_code);
}

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	//bool flag = 0;
	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		//printf("\r\n ADC Hit!\r\n");
		char stringResistance[SER_STR_SIZE];
		strncpy(stringResistance, "", sizeof(stringResistance));
		
		ret_code_t err_code;

		err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
		APP_ERROR_CHECK(err_code);

		uint32_t muxChannel = 0;
		for (uint32_t i = 0; i < SAMPLES_IN_BUFFER; i++)
		{ 
			// cycle through all the channels in the current sweep, add ADC readings to the buffer
			uint32_t adcReading = 0;
			float sensorResistance = 0.0, adcReadingFloat = 0.0;
			muxChannel = NUM_ADC_CHANS * muxSweepCount + i;
			
			if (p_event->data.done.p_buffer[i] > 0)
			{
				adcReading = p_event->data.done.p_buffer[i];
				
				adcReadingFloat = ConvertAdcReadingToVoltage(adcReading);
				sensorResistance = ConvertVoltageToResistance(adcReadingFloat);
				
				//if (muxChannel == 21)
					//sprintf(stringResistance, "ADC Reading = %d, adcV = %.4f, res = %.1f\n", adcReading, adcReadingFloat, sensorResistance);
			}
			sensResBuf[muxChannel] += sensorResistance;
		}
		muxSweepCount = (muxSweepCount < MUX_NUM_CHANNNELS - 1) ? (muxSweepCount + 1) : 0;
		ConfigureMuxGpio(muxSweepCount);
	
		if (muxChannel == MUX_LAST_CHANNEL) // After a full sweep all sensor channels are sampled
			avgCount++;

		if (avgCount == AVG_NUM_SAMPLES)
		{ // Take the average of each channel, convert to resistance, send serial data
			char stringResistance[SER_STR_SIZE];
			strncpy(stringResistance, "", sizeof(stringResistance));
			
			for (uint32_t i = 0; i < NUM_ADC_CHANS * MUX_NUM_CHANNNELS; i++)
			{
				char strTemp[10]; // add a define
				strncpy(strTemp, "", sizeof(strTemp));
				uint32_t avgSensRes = 0;

				avgSensRes = (uint32_t)roundf((sensResBuf[i] / (AVG_NUM_SAMPLES)));

				if (i < MUX_LAST_CHANNEL)
					sprintf(strTemp, "%d,", avgSensRes);
				else
					sprintf(strTemp, "%d", avgSensRes);
					
				strncat(stringResistance + strlen(stringResistance), strTemp, strlen(strTemp)); // strncpy?
				strncpy(strTemp, "", sizeof(strTemp));
			}
			//sprintf(stringResistance, "ADC Reading = %d, adcV = %.4f, res = %.1f\n", adcReading, adcReadingFloat, sensorResistance);
			uint32_t numMsgChunks = (strlen(stringResistance) / MAX_MSG_PAYLOAD_SIZE_BYTES) + 1;
			//uint32_t currMsgChunkIndex = 0;
			for (int currMsgChunkIndex = 1; currMsgChunkIndex <= numMsgChunks; currMsgChunkIndex++)
			{
				char strTemp[25]; // add a define
				uint16_t numBytesToSend = MAX_MSG_PAYLOAD_SIZE_BYTES;

				if (currMsgChunkIndex == 1)
					sprintf(strTemp,"?%d",numMsgChunks);
				else if (currMsgChunkIndex < numMsgChunks)
					sprintf(strTemp,":%d", currMsgChunkIndex);
				else
				{
					sprintf(strTemp,"!%d", currMsgChunkIndex);
					numBytesToSend = strlen(stringResistance) % MAX_MSG_PAYLOAD_SIZE_BYTES;
				}
				strncpy(strTemp+2, //TODO 
						stringResistance + MAX_MSG_PAYLOAD_SIZE_BYTES*(currMsgChunkIndex - 1),
						numBytesToSend);
				// Send chunks of 20 bytes unless this is a final chunk, which can have a smaller size
				//uint16_t numBytesToSend = (i == (strlen(stringResistance) / MAX_MSG_SIZE_BYTES)) 
				//? (strlen(stringResistance) % MAX_MSG_SIZE_BYTES) : MAX_MSG_SIZE_BYTES;
				//printf("%s\r\n", strTemp);

				ble_nus_c_string_send(&m_ble_nus_c, 
									 (strTemp), 
									 (uint16_t)numBytesToSend+2); //TODO

				//strncpy(strTemp, "", sizeof(strTemp));
			}
			avgCount = 0;
			memset(sensResBuf, 0, sizeof(sensResBuf));
			memset(stringResistance, 0, sizeof(stringResistance));
			
			muxSweepCount = 0;
			ConfigureMuxGpio(muxSweepCount);
		}
	}
}

void saadc_init(void)
{
	ret_code_t err_code;
	nrf_drv_saadc_config_t adcConfig  = { .resolution = SAADC_RESOLUTION_VAL_12bit,
										  .oversample = NRF_SAADC_OVERSAMPLE_DISABLED,
										  .interrupt_priority = 7,
										  .low_power_mode = 0
										};
	err_code = nrf_drv_saadc_init(&adcConfig, saadc_callback);
	APP_ERROR_CHECK(err_code);

	nrf_saadc_input_t saadcInputChan = NRF_SAADC_INPUT_AIN4;
	for (int i = 0; i < NUM_ADC_CHANS; i++)
	{
		nrf_saadc_channel_config_t channel_config =  NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(saadcInputChan);
		err_code = nrf_drv_saadc_channel_init(saadcInputChan - 1, &channel_config);
		APP_ERROR_CHECK(err_code);

		saadcInputChan++;
	}

	err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
	APP_ERROR_CHECK(err_code);

}

void SetupGPIO(void)
{
	nrf_gpio_cfg_output(PIN_MUX_A);
	nrf_gpio_cfg_output(PIN_MUX_B);
	nrf_gpio_cfg_output(PIN_MUX_C);
	nrf_gpio_cfg_output(PIN_MUX_EN);
	nrf_gpio_pin_clear(PIN_MUX_A);
	nrf_gpio_pin_clear(PIN_MUX_B);
	nrf_gpio_pin_clear(PIN_MUX_C);
	nrf_gpio_pin_set(PIN_MUX_EN);
}

static float ConvertAdcReadingToVoltage (uint32_t adcReading)
{
	return (float)(adcReading * NUM_MILLIVOLT_PER_LSB);
}

static float ConvertVoltageToResistance (float adcChanVoltage)
{
	return (float)((R_BIAS * adcChanVoltage / GAIN) / (V_BIAS - adcChanVoltage / GAIN));
}

static void ConfigureMuxGpio(uint32_t muxSweepCount)
{
	switch(muxSweepCount)
	{
		case 0 : 
			
			nrf_gpio_pin_set(PIN_MUX_EN);
			nrf_gpio_pin_clear(PIN_MUX_A);
			nrf_gpio_pin_clear(PIN_MUX_B);
			nrf_gpio_pin_clear(PIN_MUX_C);
			nrf_gpio_pin_clear(PIN_MUX_EN);
			break;
		case 1 : 
			nrf_gpio_pin_set(PIN_MUX_EN);
			nrf_gpio_pin_set(PIN_MUX_A);
			nrf_gpio_pin_clear(PIN_MUX_B);
			nrf_gpio_pin_clear(PIN_MUX_C);
			nrf_gpio_pin_clear(PIN_MUX_EN);
			break;
		case 2 : 
			nrf_gpio_pin_set(PIN_MUX_EN);
			nrf_gpio_pin_clear(PIN_MUX_A);
			nrf_gpio_pin_set(PIN_MUX_B);
			nrf_gpio_pin_clear(PIN_MUX_C);
			nrf_gpio_pin_clear(PIN_MUX_EN);
			break;
		case 3 : 
			nrf_gpio_pin_set(PIN_MUX_EN);
			nrf_gpio_pin_set(PIN_MUX_A);
			nrf_gpio_pin_set(PIN_MUX_B);
			nrf_gpio_pin_clear(PIN_MUX_C);
			nrf_gpio_pin_clear(PIN_MUX_EN);
			break;
		case 4 : 
			nrf_gpio_pin_set(PIN_MUX_EN);
			nrf_gpio_pin_clear(PIN_MUX_A);
			nrf_gpio_pin_clear(PIN_MUX_B);
			nrf_gpio_pin_set(PIN_MUX_C);
			nrf_gpio_pin_clear(PIN_MUX_EN);
			break;
		case 5 : 
			nrf_gpio_pin_set(PIN_MUX_EN);
			nrf_gpio_pin_set(PIN_MUX_A);
			nrf_gpio_pin_clear(PIN_MUX_B);
			nrf_gpio_pin_set(PIN_MUX_C);
			nrf_gpio_pin_clear(PIN_MUX_EN);
			break;
		case 6 : 
			nrf_gpio_pin_set(PIN_MUX_EN);
			nrf_gpio_pin_clear(PIN_MUX_A);
			nrf_gpio_pin_set(PIN_MUX_B);
			nrf_gpio_pin_set(PIN_MUX_C);
			nrf_gpio_pin_clear(PIN_MUX_EN);
			break;
		case 7 : 
			nrf_gpio_pin_set(PIN_MUX_EN);
			nrf_gpio_pin_set(PIN_MUX_A);
			nrf_gpio_pin_set(PIN_MUX_B);
			nrf_gpio_pin_set(PIN_MUX_C);
			nrf_gpio_pin_clear(PIN_MUX_EN);
			break;
		default :
			nrf_gpio_pin_set(PIN_MUX_EN);
			nrf_gpio_pin_clear(PIN_MUX_A);
			nrf_gpio_pin_clear(PIN_MUX_B);
			nrf_gpio_pin_clear(PIN_MUX_C);
			nrf_gpio_pin_clear(PIN_MUX_EN);
			break;
	}
}

int main(void)
{
	log_init();
	uint32_t err_code = nrf_drv_power_init(NULL);
	APP_ERROR_CHECK(err_code);
	power_init();
	timer_init();
	
	buttons_leds_init();
	LEDS_CONFIGURE(LEDS_MASK);
	SetupGPIO();
	ConfigureMuxGpio(0);
	
	db_discovery_init();
	//uart_initt();
	ble_stack_init();
	gatt_init();
	nus_c_init();
	
	saadc_init();
	saadc_sampling_event_init();
	
	//uint8_t stringToSend[] = "VympelVympelVympelVy";

	// Start scanning for peripherals and initiate connection
	// with devices that advertise NUS UUID.
	//printf("BLE UART central example started.");
	NRF_LOG_INFO("BLE UART central example started.");
	scan_start();
	
	printf("\r\nUART Start!\r\n");
		
	for (;;)
	{
		/*if (NRF_LOG_PROCESS() == false)
		{
			nrf_pwr_mgmt_run();
		}*/
		nrf_pwr_mgmt_run();
		//power_manage();
		//printf("test\n"); 
		//ble_nus_c_string_send(&m_ble_nus_c, (uint8_t *)stringToSend, (uint16_t)strlen(stringToSend));//(uint8_t *)stringToSend, strlen(stringToSend));
		//nrf_delay_ms(500);
	}
}
