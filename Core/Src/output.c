/*
 * output.c
 *
 *  Created on: Dec 13, 2020
 *      Author: davew
 */
#include "main.h"
#include "output.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

extern UART_HandleTypeDef huart1;
static UART_HandleTypeDef *huart = &huart1;

static volatile int huart_inprogress = 0;

/**
  * @brief Tx Transfer completed callback.
  * @param huart UART handle.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart_inprogress)
		huart_inprogress = 0;
}

static void wait_until_buffer_ready (void)
{
	while (huart_inprogress) HAL_Delay(1);
}

void output_string (const char* s)
{
	int len = strlen(s);

	while (huart_inprogress) HAL_Delay(1);
	HAL_Delay(5);

	huart_inprogress = 1;

	if (HAL_UART_Transmit_IT (huart, (uint8_t*) s, len) != HAL_OK) {
		huart_inprogress = 0;
	}
}


void output (const char* format, ...)
{
	static char outline[200];
	va_list		va;

	wait_until_buffer_ready();

	va_start (va, format);
	vsnprintf (outline, sizeof(outline), format, va);
	output_string (outline);
	va_end (va);
}
