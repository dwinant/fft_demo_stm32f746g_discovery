/*
 * adc_single.c
 *
 *  Created on: Jan 1, 2021
 *      Author: david.winant
 */
#include "main.h"
#include "output.h"
enum { FALSE, TRUE };

#define APB1_TIMER_CLOCK		100000000

static int go_ahead_process_buffer = FALSE;
int adc_sample_rate = 250000;

extern ADC_HandleTypeDef hadc3;
extern TIM_HandleTypeDef htim5;

/**
  * @brief  Regular conversion complete callback in non blocking mode
  * @param  hadc pointer to a ADC_HandleTypeDef structure that contains
  *         the configuration information for the specified ADC.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  go_ahead_process_buffer = TRUE;
}


void adc_to_q15 (uint16_t* buffer, int nsamp)
{
	for (int i = 0; i < nsamp; i++)
		buffer[i] = buffer[i] ^ 0x8000;
}

int adc_set_sample_rate (long rate) {
	long period = APB1_TIMER_CLOCK / rate;

	float actual = APB1_TIMER_CLOCK / period;

	if (period * rate != APB1_TIMER_CLOCK) {
		float error = actual / rate - 1.0;

		output ("Slight error, actual rate of %8.3f KHz, error %5.2f%%\r\n", actual/1000.0, error * 100.0);
	}

	__HAL_TIM_SET_AUTORELOAD (&htim5, period-1);
	adc_sample_rate = actual;
	return actual;
}


int adc_collect (uint16_t* buffer, int nsamp)
{
	if (0) output ("About to collect data\r\n");

	go_ahead_process_buffer = FALSE;
	int status = HAL_ADC_Start_DMA (&hadc3, (uint32_t*) buffer, nsamp);

	if (status != HAL_OK) {
	  output ("Could not start ADC DMA capture\r\n");
	  return -1;
	}

	int timeout = 500;
	while (! go_ahead_process_buffer) {
		HAL_Delay(2);
		if (--timeout == 0) break;
	}

	HAL_ADC_Stop_DMA (&hadc3);
	return (go_ahead_process_buffer) ? 0 : -1;
}
