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
/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf_nvmc.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"

#include "app_util.h"
#include "nrf_fstorage_sd.h"
#include "nrf_fstorage.h"

#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"

//SAADC
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"

#include "nrf_drv_twi.h"
#include "bno055.h"
// #include "bno055_support.c"

//End SAADC
//#define BoardNumber 3
int BoardNumber=3;
#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                           /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

#if (NRF_SD_BLE_API_VERSION == 3)
#define NRF_BLE_MAX_MTU_SIZE            GATT_MTU_SIZE_DEFAULT                       /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */
#endif

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

#define CENTRAL_LINK_COUNT              0                                           /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define ADVERTISING_LED_PIN             BSP_LED_0_MASK                              /**< Is on when device is advertising. */
#define CONNECTED_LED_PIN               BSP_LED_1_MASK                              /**< Is on when device has connected. */
#define LEDBUTTON_LED_PIN               BSP_LED_2_MASK                              /**< LED to be toggled with the help of the LED Button Service. */
#define LEDBUTTON_BUTTON_PIN            BSP_BUTTON_0                                /**< Button that will trigger the notification event with the LED Button Service */

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define DEVICE_NAME                     "Nordic_UART"                               /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */


BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};

char Recieve_Buff[UART_RX_BUF_SIZE];
char Check[30];
char *Locate_Cmd;
//static RX_STATE state;
static int numMsgs;
//static int msgIndex;
//static int numStringsPrinted = 0;
char serString[250];
//static int maxChunksReceived = 0;
//char Test[]="Hello World!";
/**************** SAADC DEFINES ***************/
uint16_t Numb= 35;
#define NUM_MILLIVOLT_PER_LSB   (3.6/4096)//(3.6/4096)
#define R_BIAS 			(120)//120k
#define V_BIAS 			(1.202)
#define GAIN 			(12)//200k/30k + 5 = 11.6

#define NUM_ADC_CHANS 		(4) // AIN4-7 for resistance, connected to MUX
//#define NUM_ADC_CHANS_PER_SWEEP 1
#define SAMPLES_IN_BUFFER       (NUM_ADC_CHANS+1) // AIN4-7 for resistance + AIN0 for ECG
#define MUX_NUM_CHANNNELS 		(8)
#define MUX_LAST_CHANNEL (NUM_ADC_CHANS * MUX_NUM_CHANNNELS - 1)
#define PIN_MUX_A 			(11)
#define PIN_MUX_B 			(12)
#define PIN_MUX_C 			(13)
#define PIN_MUX_EN 			(14)


#define AVG_NUM_SAMPLES			(2.0)
#define ADC_TRIGGER_INTERVAL_MS     	3//(3.0)
#define MAX_MSG_SIZE_BYTES     		(BLE_GATT_ATT_MTU_DEFAULT-3) //(20)
#define MAX_MSG_PAYLOAD_SIZE_BYTES      (MAX_MSG_SIZE_BYTES - 2)
//#define NUM_BYTES_TO_SEND		(7)
#define SER_STR_SIZE			(100)
#define bytesOfMessage                  (NUM_ADC_CHANS * MUX_NUM_CHANNNELS*2)


/**************** SAADC Private Variables ***************/
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(3);
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;

static uint32_t              muxSweepCount = 0;
static uint32_t              avgCount = 0;
float sensResBuf[NUM_ADC_CHANS * MUX_NUM_CHANNNELS] = {0}; // for resistance buffer

float ecgBuf[MUX_NUM_CHANNNELS] = {0}; // for ecg buffer
char strTemp1[100],strTemp2[100],strTemp[100];
char stringSerial[SER_STR_SIZE];
float avgSensRes = 0.0;
uint32_t avgSensRes_int;
/**************** SAADC Private Function Prototypes ***************/
static float ConvertAdcReadingToVoltage (uint32_t adcReading);
static float ConvertVoltageToResistance (float adcChanVoltage);
static void ConfigureMuxGpio(uint32_t muxSweepCount);
int bus_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t r_len);
int bus_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t r_len);
/**************** TWI/BNO055 Defines *************************/
/* TWI instance ID. */
#define TWI_INSTANCE_ID     1 
#define PIN_COM3            (19)
#define BNO055_API          1
/**************** TWI/BNO055 Private Function Prototypes ****************/
void bno055_read_operation_mode(void);
void bno055_read_quaternions(void);
void bno055_read_accel(void);
s8 stat_s8;
int16_t r[8];//For the output of quaternion
int16_t ab[4];//For the output of accel
//char BNO[100];
//char StrBNO[100];
int8_t Index=0;

#define Not_Calibrated  0x00
#define Done_Calibrated 0x01
uint32_t SaveCalibration = Not_Calibrated;

uint32_t Read_Data;
uint32_t RRead_Data[10];
uint32_t SaveData1,SaveData2,SaveData3,SaveData4;
uint16_t AccelX,AccelY,AccelZ,AccelR;
uint16_t GyroX,GyroY,GyroZ;

bool Reset_Calibration;
uint8_t Counter_Reset;

#define Flash_Start_add     0x7e000 
#define SaveCalibration_Add 0x7e100

#define SaveData1_Add   0x7e110
#define SaveData2_Add   0x7e120
#define SaveData3_Add   0x7e130
#define SaveData4_Add   0x7e140
/**************** TWI/BNO055 Private Variables ****************/
struct bno055_t myBNO; // BNO055 instance

struct bno055_gyro_offset_t   Gyro_offset;
struct bno055_mag_offset_t    Mag_offset;
struct bno055_accel_offset_t  Accel_offset;

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
/* Indicates if operation on TWI has ended. */
volatile bool m_xfer_done = false;
/* Buffer for samples read from BNO055. */
static uint8_t m_sample;
int ConnectedCenteral=0;
int SendingCommand=0;
/**************** Flash Variables ****************/
char Ser_data[100];
char Char_SaveData[10];
ret_code_t rc;
static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);
uint32_t My_fstorage_read(uint32_t addr, uint32_t len);

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    /* Set a handler for fstorage events. */
    .evt_handler = fstorage_evt_handler,

    /* These below are the boundaries of the flash space assigned to this instance of fstorage.
     * You must set these manually, even at runtime, before nrf_fstorage_init() is called.
     * The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
     * last page of flash available to write data. */
    .start_addr = 0x7e000,
    .end_addr   = 0x7ffff,
};

/* Dummy data to write to flash. */
uint32_t m_data          = 0xBADC0FFE;
char     m_hello_world[] = "hello world";

uint32_t Last_address;
uint32_t bootloader_addr;
uint32_t page_sz;
uint32_t code_sz;
/**@brief   Helper function to obtain the last address on the last page of the on-chip flash that
 *          can be used to write user data.
 */
static uint32_t nrf5_flash_end_addr_get()
{
    bootloader_addr = BOOTLOADER_ADDRESS;
    page_sz         = NRF_FICR->CODEPAGESIZE;
    code_sz         = NRF_FICR->CODESIZE;

    return (bootloader_addr != 0xFFFFFFFF ?
            bootloader_addr : (code_sz * page_sz));
}

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        default:
            break;
    }
}

static void print_flash_info(nrf_fstorage_t * p_fstorage)
{
    NRF_LOG_INFO("========| flash info |========");
    NRF_LOG_INFO("erase unit: \t%d bytes",      p_fstorage->p_flash_info->erase_unit);
    NRF_LOG_INFO("program unit: \t%d bytes",    p_fstorage->p_flash_info->program_unit);
    NRF_LOG_INFO("==============================");
}

/**@brief   Sleep until an event is received. */
static void power_manage(void)
{
#ifdef SOFTDEVICE_PRESENT
    (void) sd_app_evt_wait();
#else
    __WFE();
#endif
}

void wait_for_flash_ready(nrf_fstorage_t const * p_fstorage)
{
    /* While fstorage is busy, sleep and wait for an event. */
    while (nrf_fstorage_is_busy(p_fstorage))
    {
        power_manage();
    }
}


/**
 * @brief TWI initialization.
 */
void twi_init (void)
{
    const nrf_drv_twi_config_t twi_bno055_config = {
		.scl                = ARDUINO_6_PIN, //17,
		.sda                = ARDUINO_5_PIN, //16,
		.frequency          = NRF_TWI_FREQ_400K,
		.interrupt_priority = APP_IRQ_PRIORITY_LOWEST,
		.clear_bus_init     = true
    };

    ret_code_t err_code = nrf_drv_twi_init(&m_twi, &twi_bno055_config, NULL /*twi_handler*/, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}


int bus_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t r_len)
{
  
	/* Writing to BNO055 Sensor */
    uint8_t reg[2] = {reg_addr, *reg_data};
	ret_code_t err_code = nrf_drv_twi_tx(&m_twi, dev_addr, reg, sizeof(reg), false);
	while (err_code == 0x8201){
		nrf_delay_us(50);
		err_code = nrf_drv_twi_tx(&m_twi, dev_addr, reg, sizeof(reg), false);
	}
    APP_ERROR_CHECK(err_code);		
    //while (m_xfer_done == false);
	return err_code;
}

int bus_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t r_len)
{		
	//Writing to Pointer Byte (To Point Pointer to the Correct Register)
   // m_xfer_done = false;
	
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, dev_addr, &reg_addr, 1, true);
	while (err_code == 0x8201){
		nrf_delay_us(50);
		err_code = nrf_drv_twi_tx(&m_twi, dev_addr, &reg_addr, 1, true);
	}
    APP_ERROR_CHECK(err_code);		
    //while (m_xfer_done == false);

    /* Read from the pointed register */
	//m_xfer_done = false;
    err_code = nrf_drv_twi_rx(&m_twi, dev_addr, reg_data, r_len);
	while (err_code == 0x8201){
		nrf_delay_us(50);
		err_code = nrf_drv_twi_rx(&m_twi, dev_addr, reg_data, r_len);
	}
    APP_ERROR_CHECK(err_code);
	//while (m_xfer_done == false);
	
	return err_code;
}




void bno055_read_accel(void){
	s8 comres = 0;
	u8 calib_stat_accel;
	comres += bno055_get_accel_calib_stat(&calib_stat_accel);
	printf("Calib Stat: %i \n", calib_stat_accel);

	/*********read raw accel data***********/
	/* variable used to read the accel x data */
	s16 accel_datax = BNO055_INIT_VALUE;
	 /* variable used to read the accel y data */
	s16 accel_datay = BNO055_INIT_VALUE;
	/* variable used to read the accel z data */
	s16 accel_dataz = BNO055_INIT_VALUE;
	/* variable used to read the accel xyz data */
	struct bno055_accel_t accel_xyz;

	
	comres += bno055_read_linear_accel_x(&accel_datax);
	comres += bno055_read_linear_accel_y(&accel_datay);
	comres += bno055_read_linear_accel_z(&accel_dataz);
	comres += bno055_read_linear_accel_xyz(&accel_xyz);

	printf("Accel x: %d \n", accel_datax);
	printf("Accel y: %d \n", accel_datay);
	printf("Accel z: %d \n", accel_dataz);


	ab[0] = (int16_t)accel_datax;
	ab[1] = (int16_t)accel_datay;
	ab[2] = (int16_t)accel_dataz;

}


void bno055_read_quaternions (void){
	//Calibration Status:
	u8 calib_sys;
	u8 calib_accel;
	u8 calib_gyro;
	u8 calib_mag;

	s8 stat_s8 = bno055_get_sys_calib_stat(&calib_sys);
	stat_s8 += bno055_get_accel_calib_stat(&calib_accel);
	stat_s8 += bno055_get_gyro_calib_stat(&calib_gyro);
	stat_s8 += bno055_get_mag_calib_stat(&calib_mag);

	printf("Calibration Status: %i, %i, %i, %i \n", calib_sys, calib_accel, calib_gyro, calib_mag);

	/************read raw quaternion data**************/
	/* variable used to read the quaternion w data */
	s16 quaternion_data_w = BNO055_INIT_VALUE;
	/* variable used to read the quaternion x data */
	s16 quaternion_data_x = BNO055_INIT_VALUE;
	/* variable used to read the quaternion y data */
	s16 quaternion_data_y = BNO055_INIT_VALUE;
	/* variable used to read the quaternion z data */
	s16 quaternion_data_z = BNO055_INIT_VALUE;
	/* structure used to read the quaternion wxyz data */
	struct bno055_quaternion_t quaternion_wxyz;

	s8 comres = 0;
	comres += bno055_read_quaternion_w(&quaternion_data_w);
	comres += bno055_read_quaternion_x(&quaternion_data_x);
	comres += bno055_read_quaternion_y(&quaternion_data_y);
	comres += bno055_read_quaternion_z(&quaternion_data_z);
	comres += bno055_read_quaternion_wxyz(&quaternion_wxyz);

	printf("%d, %d, %d, %d \n", quaternion_data_w, quaternion_data_x, quaternion_data_y, quaternion_data_z);

	
	r[0] = (int16_t)quaternion_data_w;
	r[1] = (int16_t)quaternion_data_x;
	r[2] = (int16_t)quaternion_data_y;
	r[3] = (int16_t)quaternion_data_z;
	r[4] = (int16_t)calib_sys;
	r[5] = (int16_t)calib_accel;
	r[6] = (int16_t)calib_gyro;
	r[7] = (int16_t)calib_mag;
}

void BNO055_delay_msek_main(u32 msek)
{
	nrf_delay_ms(msek);
}

void bno055_initializing(void)
{
	myBNO.bus_read=bus_i2c_read;
	myBNO.bus_write =bus_i2c_write; 
	myBNO.delay_msec =BNO055_delay_msek_main;

	myBNO.dev_addr = BNO055_I2C_ADDR1;

	//Reset BNO055 
	nrf_gpio_pin_clear(22);
	nrf_delay_us(1);
	nrf_gpio_pin_set(22);
	nrf_delay_ms(800);
	
  
	//Initialize BNO055
	stat_s8 = bno055_init(&myBNO); // API initialization
	if (stat_s8 != BNO055_SUCCESS){
		printf("BNO055 Init Unsuccessful");
	}

	//Changing Power Mode to normal
	// u8 power_mode = BNO055_POWER_MODE_NORMAL;
	// stat_s8 = bno055_set_power_mode(power_mode);
	// if (stat_s8 != BNO055_SUCCESS){
	// 	printf("Cannot set power mode");
	// }

	//Changing operation mode to NDOF mode
	u8 operatingmode = BNO055_OPERATION_MODE_IMUPLUS;//BNO055_OPERATION_MODE_NDOF;//BNO055_OPERATION_MODE_IMUPLUS;
	stat_s8 = bno055_set_operation_mode(operatingmode);
	if (stat_s8 != BNO055_SUCCESS){
		printf("Cannot set operation mode");
	}
	else{
		printf("Set operating mode to %#010x \n", operatingmode);
	}
	nrf_delay_ms(10);
	
	//Making sure of self test result
	// u8 result;
	// stat_s8 = bus_i2c_read(BNO055_I2C_ADDR1, BNO055_SELFTEST_RESULT_ADDR, &result, BNO055_GEN_READ_WRITE_LENGTH);
	// printf("Self Test Result: %#010x \n", result);

	// //Getting System Status
	// u8 system_stat;
	// stat_s8 = bno055_get_sys_stat_code(&system_stat);
	// printf("System Status Code is: %#010x \n", system_stat);

	// // //Checking for System Error 
	// u8 system_error;
	// stat_s8 = bno055_get_sys_error_code(&system_error);
	// printf("System Error Code is: %#010x \n", system_error);

	//nrf_delay_ms(1500);
}

void bno055_read_operation_mode(void) 
{
	s8 stat_s8 = BNO055_ERROR;
	u8 prev_opmode_u8 = BNO055_OPERATION_MODE_CONFIG;
	stat_s8 = bno055_get_operation_mode(&prev_opmode_u8);	

//	if (stat_s8 == BNO055_SUCCESS)
//	{
//		printf("Mode: %d \n", prev_opmode_u8);
//	}
//	else{
//		printf("ERROR CODE: %i", stat_s8);
//	}
	
	if (prev_opmode_u8 != BNO055_OPERATION_MODE_IMUPLUS)
	{
		bno055_initializing();
	}

}
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

    /* setup m_timer for compare event every 3ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, ADC_TRIGGER_INTERVAL_MS);
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


void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}

void saadc_sampling_event_disable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_disable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;

	//strncpy(stringSerial, "", sizeof(stringSerial));

        
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        uint32_t muxChannel = 0;	
        uint32_t adcReading = 0;
        float adcReadingFloat = 0.0;
        
        if (p_event->data.done.p_buffer[0] > 0)
        {
        adcReading = p_event->data.done.p_buffer[0];    
        adcReadingFloat = ConvertAdcReadingToVoltage(adcReading);
	ecgBuf[muxSweepCount] += adcReadingFloat;
        }

        for (uint32_t i = 0; i < SAMPLES_IN_BUFFER-1 ; i++)//we have -1 for decresing the ecg that we have measured
        {   
            // cycle through all the channels in the current sweep, add ADC readings to the sensResBuf[] buffer
            //uint32_t adcReading = 0;
            float sensorResistance = 0.0; //, adcReadingFloat = 0.0;
            muxChannel = NUM_ADC_CHANS * muxSweepCount + i;
            
            if (p_event->data.done.p_buffer[i+1] > 0)
            {
                    adcReading = p_event->data.done.p_buffer[i+1];
                    adcReadingFloat = ConvertAdcReadingToVoltage(adcReading);//adcReadingFloat
                    //adcReadingFloat=2.9;
                    sensorResistance = ConvertVoltageToResistance(adcReadingFloat);//594.059;
            }
            sensResBuf[muxChannel] += sensorResistance;
        }
        muxSweepCount = (muxSweepCount < MUX_NUM_CHANNNELS - 1) ? (muxSweepCount + 1) : 0;
        ConfigureMuxGpio(muxSweepCount);

        if (muxChannel == MUX_LAST_CHANNEL) // After a full sweep all sensor channels are sampled
        {	
          avgCount++;
          if (avgCount == AVG_NUM_SAMPLES)
          { 
            if(1)
            {
                // Take the average of each channel, convert to resistance, send serial data
                nrf_drv_timer_disable(&m_timer);
                nrf_gpio_pin_clear(PIN_MUX_EN);
            
                //char stringSerial[SER_STR_SIZE];
                strncpy(stringSerial, "", sizeof(stringSerial));
                uint8_t Index_Str=0;
                uint8_t i;
                for (i = 0; i < NUM_ADC_CHANS * MUX_NUM_CHANNNELS; i++)
                {
                    strncpy(strTemp1, "", sizeof(strTemp1));
              
                    
                    avgSensRes = (float)(sensResBuf[i] / AVG_NUM_SAMPLES);
                    avgSensRes_int=(int)(avgSensRes*10);
                    //avgSensRes_int = i;

                    Index_Str=i*2;
                    stringSerial[Index_Str]=avgSensRes_int & 0xFF;
                    stringSerial[Index_Str+1]=((avgSensRes_int>>8) & 0xFF);
                }
            
                float avgECG = 0.0;
                        strncpy(strTemp2, "", sizeof(strTemp2));
                avgECG = (float) (ecgBuf[0]/ AVG_NUM_SAMPLES);
                avgSensRes_int= (int) (avgECG*100000);
                //avgSensRes_int = 111111;

                Index_Str=i*2;
                stringSerial[Index_Str]=avgSensRes_int & 0xFF;
                stringSerial[Index_Str+1]=((avgSensRes_int>>8) & 0xFF);
                stringSerial[Index_Str+2]=((avgSensRes_int>>16) & 0xFF);
                stringSerial[Index_Str+3]=((avgSensRes_int>>24) & 0xFF);

                bno055_read_quaternions();
                bno055_read_accel();
                //Testing The Flash Section

                //r[4] = (int16_t)calib_sys;
                //r[5] = (int16_t)calib_accel;
                //r[6] = (int16_t)calib_gyro;
                //r[7] = (int16_t)calib_mag;
    
                if(r[5]==3 && r[6]==3 && !Reset_Calibration)
                {
                  if(!SaveCalibration)
                  {
                    SaveCalibration=Done_Calibrated;
                    bno055_read_accel_offset(&Accel_offset);
                    bno055_read_gyro_offset(&Gyro_offset);
                    //BoardNumber=5;
                    /*Data_Accel_offset[0]=Accel_offset.x & 0xFF;
                    Data_Accel_offset[1]=((Accel_offset.x >>8) & 0xFF);
                    Data_Accel_offset[2]=Accel_offset.y & 0xFF;
                    Data_Accel_offset[3]=((Accel_offset.y >>8) & 0xFF);
                    Data_Accel_offset[4]=Accel_offset.z & 0xFF;
                    Data_Accel_offset[5]=((Accel_offset.z >>8) & 0xFF);
                    Data_Accel_offset[6]=Accel_offset.r & 0xFF;
                    Data_Accel_offset[7]=((Accel_offset.r >>8) & 0xFF);

                    Data_Gyro_offset[0]=Gyro_offset.x & 0xFF;
                    Data_Gyro_offset[1]=((Gyro_offset.x >>8) & 0xFF);
                    Data_Gyro_offset[2]=Gyro_offset.y & 0xFF;
                    Data_Gyro_offset[3]=((Gyro_offset.y >>8) & 0xFF);
                    Data_Gyro_offset[4]=Gyro_offset.z & 0xFF;
                    Data_Gyro_offset[5]=((Gyro_offset.z >>8) & 0xFF);*/

                    //sprintf(Char_SaveData,"%d",SaveCalibration);
                    rc = nrf_fstorage_erase(&fstorage, 0x7e000, 1, NULL);
                    APP_ERROR_CHECK(rc);

                    rc = nrf_fstorage_write(&fstorage, SaveCalibration_Add, &SaveCalibration, sizeof(SaveCalibration), NULL);
                    APP_ERROR_CHECK(rc);

                    SaveData1= (Accel_offset.x) | (Accel_offset.y<<16);
                    SaveData2= (Accel_offset.r) | (Accel_offset.z<<16);
                    SaveData3= (Gyro_offset.x) | (Gyro_offset.y<<16);
                    SaveData4= Gyro_offset.z;

                    rc = nrf_fstorage_write(&fstorage, SaveData1_Add, &SaveData1, sizeof(SaveData1), NULL);
                    APP_ERROR_CHECK(rc);
                    wait_for_flash_ready(&fstorage);
                    rc = nrf_fstorage_write(&fstorage, SaveData2_Add, &SaveData2, sizeof(SaveData2), NULL);
                    APP_ERROR_CHECK(rc);
                    wait_for_flash_ready(&fstorage);
                    rc = nrf_fstorage_write(&fstorage, SaveData3_Add, &SaveData3, sizeof(SaveData3), NULL);
                    APP_ERROR_CHECK(rc);
                    wait_for_flash_ready(&fstorage);
                    rc = nrf_fstorage_write(&fstorage, SaveData4_Add, &SaveData4, sizeof(SaveData4), NULL);
                    APP_ERROR_CHECK(rc);
                    wait_for_flash_ready(&fstorage);
                  }
                  else
                  {

                  }
                }
                else
                {
                  Counter_Reset++;
                  if (Counter_Reset>100)
                  {
                    //Counter_Reset=0;
                    Reset_Calibration=0;
                  }
                }
                //End Testing The Flash Section

                //bno055_read_accel_offset

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

                stringSerial[Index_Str+26]=BoardNumber;
                if(!SaveCalibration) stringSerial[Index_Str+27]=Not_Calibrated;//Not_Calibrated=0, Calibrated=1;
                else stringSerial[Index_Str+27]=Done_Calibrated;
                stringSerial[Index_Str+28]='*';
                

                uint16_t Length_Str=93;//strlen(stringSerial);
                do{
                        err_code = ble_nus_data_send(&m_nus,stringSerial,&Length_Str,m_conn_handle);
                   }while(err_code == NRF_ERROR_RESOURCES);

                
                SendingCommand=0;
                LEDS_OFF(LEDBUTTON_LED_PIN); 
                nrf_drv_timer_enable(&m_timer);
            }
            avgCount = 0; 
            memset(sensResBuf, 0, sizeof(sensResBuf));
            memset(stringSerial, 0, sizeof(stringSerial));
            muxSweepCount = 0;
            ConfigureMuxGpio(muxSweepCount);
          }
        }
    }
}

static float ConvertAdcReadingToVoltage (uint32_t adcReading)
{
	float ADC_Data=0;
        ADC_Data= adcReading * NUM_MILLIVOLT_PER_LSB;
        return ADC_Data;
}
float Res1=0,Res2=0, Res=0;
static float ConvertVoltageToResistance (float adcChanVoltage)
{
	
        Res2= adcChanVoltage / GAIN;
        Res1= R_BIAS*Res2;

        Res2= V_BIAS - Res2;
        Res=Res1/Res2;
        Res=Res*100;

        return Res;
        //return (float)((R_BIAS * adcChanVoltage / GAIN) / (V_BIAS - adcChanVoltage / GAIN));
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

    nrf_saadc_input_t saadcInputChan = NRF_SAADC_INPUT_AIN0;
    nrf_saadc_channel_config_t channel_config =  NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(saadcInputChan);
    err_code = nrf_drv_saadc_channel_init(saadcInputChan - 1, &channel_config);
    APP_ERROR_CHECK(err_code);
                                                                            
    saadcInputChan = NRF_SAADC_INPUT_AIN4;						
                                                                            
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

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/*****************************************************************************
** Extract_Forward_Str()
*****************************************************************************/
/*Extract_Forward_Str(str[]=>Massage which we search in,
serch[]=> word which we search in the massage
fwd => number of alphabet after searched word
read => number of alphabet which we like to have
ret_str => string which we like it);

Example:Extract_Forward_Str(Get_UART2,"*Date:",23,2,Temp_var);//extract Second
*/
bool Extract_Forward_Str(char str[],char serch[],int fwd,int read,char *ret_str)
{
  int N=0;
  Locate_Cmd=strstr(str,serch);
  if (Locate_Cmd!= NULL)
  {
    N=Locate_Cmd - str;
    N = N + fwd;
    strncpy(ret_str,str + N,read);
    return 1;
  }
  else
  {
    return 0;
    //error
  }
}
/*****************************************************************************
** Extract_Backward_Str()
*****************************************************************************/
/*Extract_Backward_Str(str[]=>Massage which we search in,
serch[]=> word which we search in the massage
fwd => number of alphabet after searched word
read => number of alphabet which we like to have
ret_str => string which we like it);

Example:Extract_Backward_Str(Get_UART2,"%*",5,Temp_var);//extract Second
*/
bool Extract_Backward_Str(char str[],char serch[],int fwd,char *ret_str)
{
  int N=0;
  Locate_Cmd=strstr(str,serch);
  if (Locate_Cmd!= NULL)
  {
    N=Locate_Cmd - str;
    strncpy(ret_str,str,N);
    return 1;
  }
  else
  {
    //error
    return 0;
  }
}

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        uint32_t err_code;

        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

        /*for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            do
            {
                err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                {
                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);
        }
        if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
        {
            while (app_uart_put('\n') == NRF_ERROR_BUSY);
        }*/

        strncpy(Recieve_Buff, (char *)p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
        //should search for ; but for now it is Ok
        if (Extract_Backward_Str(Recieve_Buff,";",strlen(Recieve_Buff),Check)) // first symbol in the first message, marks the start of the sequence  && Recieve_Buff[strlen(Recieve_Buff)-1]==';'
        {	
          //strncpy(serString, "", sizeof(serString));
          //printf("; was OK and, check is: %s\n",Check);
          if(Check[0]=='@') //Command @Board_numb;
          {
              //printf("@ was OK\n");
              numMsgs = Check[1]-0x30;//Change to the Source Number
              //printf("Num board: %d\n",numMsgs);
              if(numMsgs == BoardNumber) //Check if the packet belongs to this board
              {					
                  strcpy(serString, Check+2); // first three symbols are part of the header
                  //printf("SerString is %s",serString);
                  //LEDS_ON(LEDBUTTON_LED_PIN);

                  //Erase the Saved
                  SaveCalibration = Not_Calibrated;
                  rc = nrf_fstorage_erase(&fstorage, 0x7e000, 1, NULL);
                  Reset_Calibration=1;
                  Counter_Reset=0;

                  /*uint16_t Length_Str=strlen(serString);
                  //reply to msg
                  do{
                      //err_code = ble_nus_data_send(&m_nus, data_array, &length, m_conn_handle);
                      err_code = ble_nus_data_send(&m_nus,serString,&Length_Str,m_conn_handle);
                      }while(err_code == NRF_ERROR_RESOURCES);*/
              
                  //Clear Buff
                  memset(serString, 0, sizeof(serString));
                  memset(Check, 0, sizeof(Check));
                  memset(Recieve_Buff, 0, sizeof(Recieve_Buff));	
              }
              else
              {
                  LEDS_OFF(LEDBUTTON_LED_PIN);
                  //Clear Buff
                  memset(serString, 0, sizeof(serString));
                  memset(Check, 0, sizeof(Check));
                  memset(Recieve_Buff, 0, sizeof(Recieve_Buff));	
              }
          }
          else if(Check[0]=='!')//Command: !BoardNumber;
          {
              //printf("@ was OK\n");
              numMsgs = Check[1]-0x30;//Change to the Source Number
              //printf("Num board: %d\n",numMsgs);
              if(numMsgs == BoardNumber) //Check if the packet belongs to this board
              {					
                  LEDS_ON(LEDBUTTON_LED_PIN);
                  SendingCommand=1;

                  //Clear Buff
                  memset(serString, 0, sizeof(serString));
                  memset(Check, 0, sizeof(Check));
                  memset(Recieve_Buff, 0, sizeof(Recieve_Buff));	
              }
              else
              {
                  LEDS_OFF(LEDBUTTON_LED_PIN);
                  //Clear Buff
                  memset(serString, 0, sizeof(serString));
                  memset(Check, 0, sizeof(Check));
                  memset(Recieve_Buff, 0, sizeof(Recieve_Buff));	
              }
          }
 
          else
          {
                  LEDS_OFF(LEDBUTTON_LED_PIN);
                  //Clear Buff
                  memset(serString, 0, sizeof(serString));
                  memset(Check, 0, sizeof(Check));
                  memset(Recieve_Buff, 0, sizeof(Recieve_Buff));	
          }
        }
        else
        {
           LEDS_OFF(LEDBUTTON_LED_PIN);
        }
    }
}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
        default:
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
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            ConnectedCenteral=1;

            saadc_sampling_event_enable();
            //printf("\r\nConnection established! ADC started.\r\n");
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            saadc_sampling_event_disable();
            //printf("\r\nConnection established! ADC started.\r\n");
            ConnectedCenteral=0;
            break;

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

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
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


    // BLE Configuration
    //ble_cfg_t ble_cfg;

    // Max Packets Per Connection Event
    //memset(&ble_cfg, 0, sizeof ble_cfg);
    //ble_cfg.conn_cfg.conn_cfg_tag                            = APP_BLE_CONN_CFG_TAG;
    //ble_cfg.conn_cfg.params.gatts_conn_cfg.hvn_tx_queue_size = 7;
    //err_code = sd_ble_cfg_set(BLE_CONN_CFG_GATTS, &ble_cfg, ram_start);
    //APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;

            if ((data_array[index - 1] == '\n') ||
                (data_array[index - 1] == '\r') ||
                (index >= m_ble_nus_max_data_len))
            {
                if (index > 1)
                {
                    NRF_LOG_DEBUG("Ready to send data over BLE NUS");
                    NRF_LOG_HEXDUMP_DEBUG(data_array, index);

                    do
                    {
                        uint16_t length = (uint16_t)index;
                        err_code = ble_nus_data_send(&m_nus, data_array, &length, m_conn_handle);
                        if ((err_code != NRF_ERROR_INVALID_STATE) &&
                            (err_code != NRF_ERROR_RESOURCES) &&
                            (err_code != NRF_ERROR_NOT_FOUND))
                        {
                            APP_ERROR_CHECK(err_code);
                        }
                    } while (err_code == NRF_ERROR_RESOURCES);
                }

                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
/**@snippet [Handling the data received over UART] */


/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
#if defined (UART_PRESENT)
        .baud_rate    = NRF_UART_BAUDRATE_115200
#else
        .baud_rate    = NRF_UARTE_BAUDRATE_115200
#endif
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}
/**@snippet [UART Initialization] */


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
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
    nrf_gpio_pin_clear(PIN_MUX_EN);

    
    //IMU
    nrf_gpio_cfg_output(22);
    nrf_gpio_pin_set(22);
    nrf_gpio_cfg_output(19);
    nrf_gpio_pin_clear(19);
}

static void ConfigureMuxGpio(uint32_t muxSweepCount)
{
    switch(muxSweepCount)
    {
      case 0 : 
              
              nrf_gpio_pin_clear(PIN_MUX_EN);
              nrf_gpio_pin_clear(PIN_MUX_A);
              nrf_gpio_pin_clear(PIN_MUX_B);
              nrf_gpio_pin_clear(PIN_MUX_C);
              nrf_gpio_pin_set(PIN_MUX_EN);
              break;
      case 1 : 
              nrf_gpio_pin_clear(PIN_MUX_EN);
              nrf_gpio_pin_set(PIN_MUX_A);
              nrf_gpio_pin_clear(PIN_MUX_B);
              nrf_gpio_pin_clear(PIN_MUX_C);
              nrf_gpio_pin_set(PIN_MUX_EN);
              break;
      case 2 : 
              nrf_gpio_pin_clear(PIN_MUX_EN);
              nrf_gpio_pin_clear(PIN_MUX_A);
              nrf_gpio_pin_set(PIN_MUX_B);
              nrf_gpio_pin_clear(PIN_MUX_C);
              nrf_gpio_pin_set(PIN_MUX_EN);
              break;
      case 3 : 
              nrf_gpio_pin_clear(PIN_MUX_EN);
              nrf_gpio_pin_set(PIN_MUX_A);
              nrf_gpio_pin_set(PIN_MUX_B);
              nrf_gpio_pin_clear(PIN_MUX_C);
              nrf_gpio_pin_set(PIN_MUX_EN);
              break;
      case 4 : 
              nrf_gpio_pin_clear(PIN_MUX_EN);
              nrf_gpio_pin_clear(PIN_MUX_A);
              nrf_gpio_pin_clear(PIN_MUX_B);
              nrf_gpio_pin_set(PIN_MUX_C);
              nrf_gpio_pin_set(PIN_MUX_EN);
              break;
      case 5 : 
              nrf_gpio_pin_clear(PIN_MUX_EN);
              nrf_gpio_pin_set(PIN_MUX_A);
              nrf_gpio_pin_clear(PIN_MUX_B);
              nrf_gpio_pin_set(PIN_MUX_C);
              nrf_gpio_pin_set(PIN_MUX_EN);
              break;
      case 6 : 
              nrf_gpio_pin_clear(PIN_MUX_EN);
              nrf_gpio_pin_clear(PIN_MUX_A);
              nrf_gpio_pin_set(PIN_MUX_B);
              nrf_gpio_pin_set(PIN_MUX_C);
              nrf_gpio_pin_set(PIN_MUX_EN);
              break;
      case 7 : 
              nrf_gpio_pin_clear(PIN_MUX_EN);
              nrf_gpio_pin_set(PIN_MUX_A);
              nrf_gpio_pin_set(PIN_MUX_B);
              nrf_gpio_pin_set(PIN_MUX_C);
              nrf_gpio_pin_set(PIN_MUX_EN);
              break;
      default :
              nrf_gpio_pin_clear(PIN_MUX_EN);
              nrf_gpio_pin_clear(PIN_MUX_A);
              nrf_gpio_pin_clear(PIN_MUX_B);
              nrf_gpio_pin_clear(PIN_MUX_C);
              nrf_gpio_pin_set(PIN_MUX_EN);
              break;
    }
}

void gatt_mtu_set(uint16_t att_mtu)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, att_mtu);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_ble_gatt_att_mtu_central_set(&m_gatt, att_mtu);
    APP_ERROR_CHECK(err_code);
}


void conn_evt_len_ext_set(bool status)
{
    ret_code_t err_code;
    ble_opt_t  opt;

    memset(&opt, 0x00, sizeof(opt));
    opt.common_opt.conn_evt_ext.enable = status ? 1 : 0;

    err_code = sd_ble_opt_set(BLE_COMMON_OPT_CONN_EVT_EXT, &opt);
    APP_ERROR_CHECK(err_code);
}


uint32_t My_fstorage_read(uint32_t addr, uint32_t len)
{
    ret_code_t rc;
    uint8_t    data[256] = {0};
    uint32_t Data_read;

    if (len > sizeof(data))
    {
        len = sizeof(data);
    }

    // Read data.
    rc = nrf_fstorage_read(&fstorage, addr, data, len);
    if (rc != NRF_SUCCESS)
    { 
        sprintf(Ser_data,"nrf_fstorage_read() returned: %s\n",nrf_strerror_get(rc));
        return 0;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        //nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "0x%x ", data[i]);
        Ser_data[i]= data[i];
    }

    Data_read=(Ser_data[3]<<24) | (Ser_data[2]<<16) | (Ser_data[1]<<8) | (Ser_data[0]);
    return Data_read;
}

/**@brief Application main function.
 */
int main(void)
{
    bool erase_bonds;
    ConnectedCenteral=0;
    nrf_fstorage_api_t * p_fs_api;
    
    //ConvertVoltageToResistance(29);

    //log_init();
    power_management_init();
    timers_init();

    
    //nrf_delay_ms(1000);
    uart_init();

    // Start execution.
    SetupGPIO();
    ConfigureMuxGpio(0);

    twi_init();
    bno055_initializing();
    bno055_read_operation_mode();

#ifdef SOFTDEVICE_PRESENT
    NRF_LOG_INFO("SoftDevice is present.");
    NRF_LOG_INFO("Initializing nrf_fstorage_sd implementation...");
    /* Initialize an fstorage instance using the nrf_fstorage_sd backend.
     * nrf_fstorage_sd uses the SoftDevice to write to flash. This implementation can safely be
     * used whenever there is a SoftDevice, regardless of its status (enabled/disabled). */
    p_fs_api = &nrf_fstorage_sd;
#else
    NRF_LOG_INFO("SoftDevice not present.");
    NRF_LOG_INFO("Initializing nrf_fstorage_nvmc implementation...");
    /* Initialize an fstorage instance using the nrf_fstorage_nvmc backend.
     * nrf_fstorage_nvmc uses the NVMC peripheral. This implementation can be used when the
     * SoftDevice is disabled or not present.
     *
     * Using this implementation when the SoftDevice is enabled results in a hardfault. */
    p_fs_api = &nrf_fstorage_nvmc;
#endif
    rc = nrf_fstorage_init(&fstorage, p_fs_api, NULL);
    APP_ERROR_CHECK(rc);
    Last_address=nrf5_flash_end_addr_get();
    //rc = nrf_fstorage_erase(&fstorage, 0x7e000, 1, NULL);

    //Read The Calibration Address from the Flash
    Read_Data= My_fstorage_read(0x7e100,4);
    
    if(Read_Data!=Done_Calibrated)
    {
      SaveCalibration=Not_Calibrated;
    }
    else//there is a Calibration saved in EEPROM (As we have saved 0x01 in specific location of Flash)
    {
      //Read All the Calibration Data
      RRead_Data[0]= My_fstorage_read(SaveData1_Add,4);
      Accel_offset.x= (RRead_Data[0] & 0xFFFF);
      Accel_offset.y=((RRead_Data[0]>>16) & 0xFFFF);
      RRead_Data[1]= My_fstorage_read(SaveData2_Add,4);
      Accel_offset.r= (RRead_Data[1] & 0xFFFF);
      Accel_offset.z=((RRead_Data[1]>>16) & 0xFFFF);
      RRead_Data[2]= My_fstorage_read(SaveData3_Add,4);
      Gyro_offset.x= (RRead_Data[2] & 0xFFFF);
      Gyro_offset.y=((RRead_Data[2]>>16) & 0xFFFF);
      RRead_Data[3]= My_fstorage_read(SaveData4_Add,4);
      Gyro_offset.z=(RRead_Data[3] & 0xFFFF);

      bno055_write_accel_offset(&Accel_offset);
      bno055_write_gyro_offset(&Gyro_offset);
      SaveCalibration=Done_Calibrated;
      //BNO055_RETURN_FUNCTION_TYPE bno055_write_mag_offset(struct bno055_mag_offset_t *mag_offset);
    }

    

    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();

    advertising_start();

    saadc_init();
    saadc_sampling_event_init();
    //saadc_sampling_event_enable();

    //printf("\r\nUART started.\r\n");
    NRF_LOG_INFO("Debug logging for UART over RTT started.");

    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
        
    }
}


/**
 * @}
 */