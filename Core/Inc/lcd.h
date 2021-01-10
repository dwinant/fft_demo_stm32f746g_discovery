/*
 * lcd.h
 *
 *  Created on: Jan 3, 2021
 *      Author: david.winant
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>
#include "stm32746g_discovery_lcd.h"

#ifdef __cplusplus
extern "C" {
#endif

void XSP_LCD_DisplayStringAt (uint16_t Xpos, uint16_t Ypos, uint8_t *Text, Text_AlignModeTypdef Mode);

#ifdef __cplusplus
}
#endif

#endif /* INC_LCD_H_ */
