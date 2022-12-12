#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "boards.h"
#include "app_error.h"
#include "app_uart.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_power.h"
//#include "nrf_drv_uart.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

#define NUM_MILLIVOLT_PER_LSB (3.6 / 4096)
#define R_BIAS 760
#define V_BIAS 1.25
#define GAIN 8.925

#define NUM_ADC_CHANS 1
//#define NUM_ADC_CHANS_PER_SWEEP 1
#define SAMPLES_IN_BUFFER NUM_ADC_CHANS
#define MUX_NUM_CHANNNELS 8
#define MUX_LAST_CHANNEL (SAMPLES_IN_BUFFER * MUX_NUM_CHANNNELS - 1)
#define PIN_MUX_A 11
#define PIN_MUX_B 12
#define PIN_MUX_C 13
#define PIN_MUX_EN 14

#define AVG_NUM_SAMPLES 3

static float ConvertAdcReadingToVoltage (uint32_t adcReading);
static float ConvertVoltageToResistance (float adcChanVoltage);
static void ConfigureMuxGpio(uint32_t muxSweepCount);

volatile uint8_t state = 1;
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(0);
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              muxSweepCount = 0;
static uint32_t				 avgCount = 0;
uint32_t adcBuffer[NUM_ADC_CHANS * MUX_NUM_CHANNNELS] = {0};

/**
 * @brief UART events handler.
 */
void uart_events_handler(app_uart_evt_t * p_event)
{
}

void uart_error_handle(app_uart_evt_t * p_event)
{
	if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
	{
		APP_ERROR_HANDLER(p_event->data.error_communication);
	}
	else if (p_event->evt_type == APP_UART_FIFO_ERROR)
	{
		APP_ERROR_HANDLER(p_event->data.error_code);
	}
}
 
/**
 * @brief UART initialization.
 */
void uart_config(void)
{
	uint32_t err_code;
	const app_uart_comm_params_t comm_params =
	{
		RX_PIN_NUMBER,
		TX_PIN_NUMBER,
		RTS_PIN_NUMBER,
		CTS_PIN_NUMBER,
		APP_UART_FLOW_CONTROL_DISABLED,
		false,
		UART_BAUDRATE_BAUDRATE_Baud115200
	};
 
	APP_UART_FIFO_INIT(&comm_params,
					   UART_RX_BUF_SIZE,
					   UART_TX_BUF_SIZE,
					   uart_events_handler,
					   APP_IRQ_PRIORITY_LOWEST,
					   err_code);
	APP_ERROR_CHECK(err_code);
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

	/* setup m_timer for compare event every 50ms */
	uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 5);
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


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		ret_code_t err_code;

		err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
		APP_ERROR_CHECK(err_code);

		uint32_t muxChannel = 0;
		for (uint32_t i = 0; i < SAMPLES_IN_BUFFER; i++)
		{ // cycle through all the channels in the current sweep, add ADC readings to the buffer
			uint32_t adcReading = 0;
			muxChannel = NUM_ADC_CHANS * muxSweepCount + i;
			
			if (p_event->data.done.p_buffer[i] > 0)
				adcReading = p_event->data.done.p_buffer[i];

			adcBuffer[muxChannel] += adcReading;
		}
		muxSweepCount = (muxSweepCount < MUX_NUM_CHANNNELS - 1) ? (muxSweepCount + 1) : 0;
		ConfigureMuxGpio(muxSweepCount);
	
		if (muxChannel == MUX_LAST_CHANNEL) // After a full sweep all sensor channels are sampled
			avgCount++;

		if (avgCount == AVG_NUM_SAMPLES)
		{ // Take the average of each channel, convert to resistance, send serial data
			for (uint32_t i = 0; i < NUM_ADC_CHANS * MUX_NUM_CHANNNELS; i++)
			{
				float adcAvgChanRead = 0.0, adcChanVoltage = 0.0, sensorResistance = 0.0;

				adcAvgChanRead = (float)(adcBuffer[i] / (AVG_NUM_SAMPLES));
				adcChanVoltage = ConvertAdcReadingToVoltage(adcAvgChanRead);
				sensorResistance = ConvertVoltageToResistance(adcChanVoltage);

				if (i < MUX_LAST_CHANNEL)
					printf("%.1f,", i, sensorResistance);
				else
					printf("%.1f\r\n", i, sensorResistance);
			}
			avgCount = 0;
			memset(adcBuffer, 0, sizeof(adcBuffer));
			
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

/**
 * @brief Function for main application entry.
 */
int main(void)
{
	//nrf_delay_ms(500);
	uint32_t err_code = nrf_drv_power_init(NULL);
	APP_ERROR_CHECK(err_code);

	ret_code_t ret_code = nrf_pwr_mgmt_init();
	APP_ERROR_CHECK(ret_code);

	SetupGPIO();
	uart_config();
	printf("\r\n Readout V2 \r\n");
	
	saadc_init();
	saadc_sampling_event_init();
	nrf_delay_ms(500);
	ConfigureMuxGpio(0);
	saadc_sampling_event_enable();

	while (1)
	{
		//nrf_gpio_pin_toggle(PIN_MUX_A);
		//nrf_gpio_pin_toggle(LED_PIN_2);

		//nrf_delay_ms(500);
		//nrf_pwr_mgmt_run();
		//NRF_LOG_FLUSH();
	}
}


/** @} */
