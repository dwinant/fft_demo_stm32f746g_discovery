/*
 * ffts_q15.cc
 *
 *  Created on: Jan 2, 2021
 *      Author: david.winant
 *
 *  from ffts.c
 *
 *  Created on: Dec 31, 2020
 *      Author: david.winant
 */
#include <stdint.h>
#include <stdlib.h>
#include "fft.h"
#include "output.h"
#include "adc.h"
#include "fftanalysis.h"
#include "charterlog.h"

#define PLOT_WINDOW				0
#define PLOT_DATA				0
#define DEBUG_STOP_ON_FIRST		0

static int show_captured_data = 0;
static int show_fft_text = 0;

//len: power of 2 between 32 and 8k
#define FFT_LEN			1024

#define Q15_SBIT		0x8000
#define Q15_MBITS		0x7FFF
#define Q15_MULT		0x8000
#define Q15_PMAX		0x7FFF
#define Q15_NMAX		0x8000

static int16_t srcData[FFT_LEN];


static void sine (uint32_t base_rate, uint32_t sample_rate, float amplitude, int phase);
static void q15_show (float f);
static void show_array (int16_t* p, int len);
static void show_complex_array (int16_t* p, int len);
static void show_fft_results (fft_analysis* pfft);

static long sample_rate = 100000;

float fft_max_value = 0;

extern "C" void ffts_q15 (void)
{
	output ("fft Q15 test\r\n");
	output ("fft sample rate %d KHz\r\n", adc_set_sample_rate (sample_rate) / 1000);

	fft_analysis		fft (FFT_LEN, adc_sample_rate);
	charter_log			C;

	if (PLOT_WINDOW) {
		fft.window_hamming();
		C.plot_function (fft.rWindow, 0, 1.0, fft.npoints);
		while(1);
	}
	while (1) {
		memset (srcData, 0, sizeof(srcData));
		if (0) {
			sine (160471, sample_rate, 0.3, 0);
			sine (14400, sample_rate, 0.3, 0);
			sine (214400, sample_rate, 0.3, 0);
		} else {
			int status = adc_collect ((uint16_t*) srcData, FFT_LEN);
			if (status != 0) {
				output ("Failed to collect ADC data\r\n");
				return;
			}
			adc_to_q15 ((uint16_t*) srcData, FFT_LEN);
		}

		if (PLOT_DATA) {
			C.plot_function (srcData, -1, 1, FFT_LEN/20);
			while(1);
		}

		if (show_captured_data) show_array (srcData, FFT_LEN);

		fft.perform_fft (srcData);

		if (show_fft_text) show_fft_results (&fft);
		fft.find_peaks();
		C.chart_fft (&fft);
		HAL_Delay (500);
		if (DEBUG_STOP_ON_FIRST) while(1);
	}
}

int16_t f_to_q15 (float f)
{
	if (f >= 1.0) return Q15_PMAX;
	if (f <= -1.0) return Q15_NMAX;

	if (f < 0) {
		return - (Q15_MBITS & (int16_t) (-f * Q15_MULT));
	} else {
		return Q15_MBITS & (int16_t) (f * Q15_MULT);
	}
}

float q15_to_f (int16_t q15)
{
	uint16_t uq15 = q15;
	float f = (uq15 & Q15_MBITS) * 1.0 / Q15_MULT;
	if (uq15 & Q15_SBIT) f = -f;
	return q15 * 1.0 / Q15_MULT;
}

__attribute__((unused))
static void q15_show (float f)
{
	output ("Q15 %6.3f  %04X\r\n", f, (uint16_t) f_to_q15(f));
}

static void show_array (int16_t* p, int len)
{
	output ("array contents:\r\n");
	for (int i = 0; i < len; ) {
		for (int j = 0; j < 10 && i < len; j++, i++)
			output ("  %04X %6.3f", (uint16_t) p[i], q15_to_f(p[i]));
		output ("\r\n");
	}
}

__attribute__((unused))
static void show_complex_array (int16_t* p, int len)
{
	output ("array contents:\r\n");
	for (int i = 0; i < len*2; ) {
		for (int j = 0; j < 6 && i < len*2; j++, i+= 2)
			output (" %4d (%04X,%04X) ", i/2, (uint16_t) p[i], (uint16_t) p[i+1]);
		output ("\r\n");
	}
}

static void sine (uint32_t base_rate, uint32_t sample_rate, float amplitude, int phase)
{
	if (1) output ("sine at %d KHz, ampl %5.3f phase %d%%\r\n", base_rate/1000, amplitude, phase);
	float samples_per_cycle = 1.0 * sample_rate / base_rate;
	float phase_shift = 2.0 * M_PI * phase / 100;

	if (0) output ("s-rate %d  freq %d  s/cycle %f\r\n", sample_rate, base_rate, samples_per_cycle);
	for (uint32_t sample = 0; sample < FFT_LEN; sample++) {
		// sine waveform math
		float omega = sample * 1.0 / samples_per_cycle * 2 * M_PI + phase_shift;
		if (0) output ("  %4d %8.5f %04X\r\n", sample, sin(omega)*amplitude, (uint16_t) f_to_q15 (sin(omega) * amplitude));
		srcData[sample] += f_to_q15 (sin(omega) * amplitude);
	}
}


static char* stars (float value, float max, int num_stars)
{
	static char star_buffer[80];

	float per_star = max * 1.1 / num_stars;
	int stars = value / per_star;

	memset (star_buffer, 0, sizeof(star_buffer));

	if (stars < 0) stars = -stars;

	memset (star_buffer, '*', stars);
	return star_buffer;
}


void show_fft_results (fft_analysis* pfft)
{
	q15_t* data = pfft->fftd;
	float hz_per_bin = pfft->sample_rate/2 / (pfft->npoints/2);
	float max_v = pfft->get_max_value();

	output (" BIN   FREQUENCY    LEVEL\r\n");
	for (int bin = 0; bin < pfft->bins; bin++) {
		float value = q15_to_f(data[bin]);
		if (fabs (value) > 0.003)
			output ("%4d   %8.2f KHz  %8.5f   %s\r\n", bin, (bin * hz_per_bin / 1000), value, stars(value, max_v, 25));
	}
	output ("\r\n");
}


