/*
 * rectangle.cpp
 *
 *  Created on: Jan 2, 2021
 *      Author: david.winant
 */
#include "rectangle.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"
#include "stm32746g_discovery_ts.h"

rectangle::~rectangle() {
	// TODO Auto-generated destructor stub
}

void rectangle::outline (int16_t offset)
{
	/* Draw horizontal lines */
	BSP_LCD_DrawHLine (X+offset, Y+offset, W-1-2*offset);
	BSP_LCD_DrawHLine (X+offset, Y+H-1-offset, W-1-2*offset+1);

	/* Draw vertical lines */
	BSP_LCD_DrawVLine (X+offset, Y+offset, H-1-2*offset);
	BSP_LCD_DrawVLine (X+W-1-offset, Y+offset, H-1-2*offset);
}
