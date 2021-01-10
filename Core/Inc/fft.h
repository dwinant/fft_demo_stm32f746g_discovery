/*
 * fft.h
 *
 *  Created on: Jan 1, 2021
 *      Author: david.winant
 */

#ifndef INC_FFT_H_
#define INC_FFT_H_

#define ARM_MATH_CM7

#include "main.h"
#include "arm_math.h"


float q15_to_f (q15_t q15);
q15_t f_to_q15 (float f);

#endif /* INC_FFT_H_ */
