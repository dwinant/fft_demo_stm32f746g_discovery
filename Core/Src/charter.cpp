/*
 * charter.cpp
 *
 *  Created on: Jan 2, 2021
 *      Author: david.winant
 */
#include <string.h>
#include <stdio.h>
#include "charter.h"
#include "fftanalysis.h"
#include "output.h"
#include "rectangle.h"
#include "fft.h"
#include "lcd.h"

#define		LCD_X		480
#define		LCD_Y		272


charter::charter() {
	full_scale_value = 0.0125;
	ybase = CHART_INSET_Y + CHART_DISPLAY_HEIGHT;
}

charter::~charter() {
}


void charter::draw_chart_border (void)
{
	BSP_LCD_SetTextColor (LCD_COLOR_BLUE);
	rectangle R(CHART_INSET_X, CHART_INSET_Y, CHART_DISPLAY_WIDTH, CHART_DISPLAY_HEIGHT);

	for (int i = 1; i <= CHART_BORDER_THICK; i++) {
		R.outline (-i);
	}
}


void charter::draw_screen_border (void)
{
	BSP_LCD_SetTextColor (LCD_COLOR_BLUE);
	rectangle R(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

	for (int i = 0; i < SCREEN_BORDER_THICK; i++) {
		R.outline (i);
	}
}


void charter::plot_function (q15_t* data, float min, float max, int points)
{
	int   xp0 = 0;
	int	  xp1;
	float y = q15_to_f(data[0]);
	int   yp0 = (1.0 - (y - min) / (max - min)) * CHART_DISPLAY_HEIGHT;
	int   yp1;

	setup_chart();

	for (int pt = 1; pt < points; pt++) {
		xp1 = 1.0 * CHART_DISPLAY_WIDTH * pt / points + 0.5;
		y = q15_to_f(data[pt]);
		yp1 = (1.0 - (y - min) / (max - min)) * CHART_DISPLAY_HEIGHT;

		BSP_LCD_DrawLine (xp0 + CHART_INSET_X, yp0 + CHART_INSET_Y, xp1 + CHART_INSET_X, yp1 + CHART_INSET_Y);

		xp0 = xp1;
		yp0 = yp1;
	}
}


float charter::display_value (q15_t y)
{
	return q15_to_f(y);
}

int charter::y_to_height (q15_t y)
{
	return q15_to_f(y) / full_scale_value * CHART_DISPLAY_HEIGHT + 0.5;
}


void charter::draw_chart (q15_t* data, int points)
{
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);

	for (int x = 0; x <= xlimit; x++) {
		int height = y_to_height (data[x]);

		if (height > 0) {
			BSP_LCD_DrawVLine (x + CHART_INSET_X,  ybase - height, height);
		}
	}
}


void charter::note_peak (struct peak* pk)
{
	const int	Xsz = 2;

	char msg[12];

	int x = CHART_INSET_X + pk->pk_bin;
	int height = y_to_height (pk->pk_value_at_bin);
	int y = ybase - height;

	// X marks the spot
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_DrawLine (x-Xsz, y-Xsz, x+Xsz, y+Xsz);
	BSP_LCD_DrawLine (x-Xsz, y+Xsz, x+Xsz, y-Xsz);

	//snprintf (msg, sizeof(msg), "#%d %.3f KHz", pk+1, pk->pk_f/1000);

#if 0
	snprintf (msg, sizeof(msg), "%.2f", pk->pk_interp_freq/1000);
	Text_AlignModeTypdef mode;
	if (pk->pk_bin > 9 * CHART_DISPLAY_WIDTH/10) {
		x -= 5;
		mode = RIGHT_MODE;
	} else {
		x += 5;
		mode = LEFT_MODE;
	}
	y -= BSP_LCD_GetFont()->Height;
	BSP_LCD_DisplayStringAt (x, y, (uint8_t*) msg, mode);
#else
	int ymark;
	if (y < 2 * BSP_LCD_GetFont()->Height)
		ymark = y+4;
	else
		ymark = y-3-1*BSP_LCD_GetFont()->Height;
	snprintf (msg, sizeof(msg), "%5.3f", display_value(pk->pk_value_at_bin));
	XSP_LCD_DisplayStringAt (x, ymark, (uint8_t*) msg, CENTER_MODE);

	snprintf (msg, sizeof(msg), " %7.3f ", pk->pk_interp_freq/1000);
	XSP_LCD_DisplayStringAt (x, ybase+3, (uint8_t*) msg, CENTER_MODE);
#endif
}


void charter::setup_chart (void)
{
	/* Clear the Background Layer */
	BSP_LCD_Clear (LCD_COLOR_WHITE);

	draw_screen_border();
	draw_chart_border();

	BSP_LCD_SetFont(&Font12);

	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
}

void charter::adjust_full_scale (q15_t max_v)
{
	if (max_v > full_scale_value) full_scale_value = max_v * 1.2;
}

void charter::chart_fft (fft_analysis* fft)
{
	char msg[80];

	xlimit = MIN(CHART_DISPLAY_WIDTH-1, fft->bins);
	adjust_full_scale (fft->get_max_value());

	setup_chart();
	grid_lines();
	draw_chart (fft->fftd, fft->bins);

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	snprintf (msg, sizeof(msg), "Freq 0 - %.1f KHz", fft->bin_frequency(xlimit)/1000);
	BSP_LCD_DisplayStringAt (0, 242, (uint8_t*) msg, CENTER_MODE);
	snprintf (msg, sizeof(msg), "Full Scale %.4f", full_scale_value);
	BSP_LCD_DisplayStringAt (0, 257, (uint8_t*) msg, CENTER_MODE);

	// highlight peaks
	struct peak peak;
	for (int pk = 8; pk >= 0; pk--) {
		if (fft->get_peak(pk, &peak) == 0) {
			if (peak.pk_bin < CHART_DISPLAY_WIDTH && peak.pk_bin > 3 && peak.pk_value_at_bin > .003)
				note_peak (&peak);
		}
	}
}

