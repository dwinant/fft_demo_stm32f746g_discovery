/*
 * lcd.c
 *
 *  Created on: Jan 3, 2021
 *      Author: david.winant
 */
#include <string.h>
#include <stdio.h>
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"
#include "stm32746g_discovery_ts.h"




/**
  * @brief  Displays characters on the LCD.
  * @param  Xpos: X position (in pixel)
  * @param  Ypos: Y position (in pixel)
  * @param  Text: Pointer to string to display on LCD
  * @param  Mode: Display mode
  *          This parameter can be one of the following values:
  *            @arg  CENTER_MODE
  *            @arg  RIGHT_MODE
  *            @arg  LEFT_MODE
  * @retval None
  */
void XSP_LCD_DisplayStringAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text, Text_AlignModeTypdef Mode)
{
  uint16_t ref_column = 1, i = 0;
  uint32_t size = 0;
  uint8_t  *ptr = Text;

  sFONT* font = BSP_LCD_GetFont();
  int    fontwidth = font->Width;

  /* Get the text size */
  while (*ptr++) size ++ ;

  switch (Mode)
  {
  case LEFT_MODE:
    {
      ref_column = Xpos;
      break;
    }
  case CENTER_MODE:
  {
	  ref_column = Xpos - (size * fontwidth) / 2;
	  break;
  }
  case RIGHT_MODE:
    {
	  ref_column = Xpos - size * fontwidth;
	  break;
    }
  default:
    {
      ref_column = Xpos;
      break;
    }
  }

  /* Check that the Start column is located in the screen */
  if ((ref_column < 1) || (ref_column >= 0x8000))
  {
    ref_column = 1;
  }

  /* Send the string character by character on LCD */
  while ((*Text != 0) & (((BSP_LCD_GetXSize() - (i*fontwidth)) & 0xFFFF) >= fontwidth))
  {
    /* Display one character on LCD */
    BSP_LCD_DisplayChar(ref_column, Ypos, *Text);
    /* Decrement the column position by 16 */
    ref_column += fontwidth;
    /* Point on the next character */
    Text++;
    i++;
  }
}
