/*
 * charterlog.cpp
 *
 *  Created on: Jan 9, 2021
 *      Author: david.winant
 */
#include "charterlog.h"
#include "output.h"
#include "fft.h"
#include "lcd.h"


charter_log::charter_log() {
	full_scale_value = display_value(1);
}

charter_log::~charter_log() {
}

float charter_log::display_value (q15_t y)
{
	return 10 * log10(q15_to_f(y));
}

int charter_log::y_to_height (q15_t y)
{
	if (y <= 0) return 0;

	if (0) output ("y %04x %7.5f log %9.4f\r\n", y, q15_to_f(y), 10 * log10(q15_to_f(y)));
	return (full_scale_value - 10 * log10(q15_to_f(y))) / full_scale_value * CHART_DISPLAY_HEIGHT + 1.5;
}

void charter_log::grid_lines (void)
{
	BSP_LCD_SetTextColor (LCD_COLOR_BLUE);

	for (float y = 0; y > full_scale_value; y -= 10.0) {
		int ypos = (full_scale_value - y) / full_scale_value * CHART_DISPLAY_HEIGHT + 1.5;
		BSP_LCD_DrawHLine (CHART_INSET_X, ybase - ypos,  CHART_DISPLAY_WIDTH);
	}
}

