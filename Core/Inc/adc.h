/*
 * adc.h
 *
 *  Created on: Jan 1, 2021
 *      Author: david.winant
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int adc_sample_rate;

int adc_collect (uint16_t* buffer, int nsamp);
void adc_to_q15 (uint16_t* buffer, int nsamp);
int adc_set_sample_rate (long rate);

#ifdef __cplusplus
}
#endif


#endif /* INC_ADC_H_ */
