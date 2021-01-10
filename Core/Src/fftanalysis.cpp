/*
 * fftanalysis.cpp
 *
 *  Created on: Jan 1, 2021
 *      Author: david.winant
 */
#include <stdlib.h>
#include "fftanalysis.h"
#include "output.h"
#include "fft.h"

#define DEBUG_CHECK_COMPLEX		0
#define SHOW_PEAKS				0

#define MAX_FFT_LEN				8192


fft_analysis::fft_analysis (int npoints, long sample_rate)
{
	this->npoints = npoints;
	this->sample_rate = sample_rate;
	windowed = false;

	bins = npoints / 2 + 1;
	fftd = (q15_t*) malloc (this->bins * sizeof(q15_t));
	rWindow = (q15_t*) malloc (npoints * sizeof(q15_t));
	cDstData = (q15_t*) malloc (2 * npoints * sizeof(q15_t));

	hz_per_bin = sample_rate / 2 / (npoints / 2);
	reset();
}

fft_analysis::~fft_analysis() {
	free (this->fftd);
	free (this->rWindow);
	free (this->cDstData);
}


void fft_analysis::reset (void)
{
	max_value = 0;
	pk_cnt = 0;
}


int fft_analysis::perform_fft (q15_t* input_data)
{
	int			status;
	uint32_t 	fftFlagForward = 0;
	uint32_t 	fftFlagNotReversed = 1;

	// we assume the input data is 1.15 (data between +/- 1.0)
	if (!windowed) window_hamming();
	// windowing

	// apply the window factors in-place to the input data
	arm_mult_q15 (input_data, rWindow, input_data, npoints);
	// windowing uses saturation multiplication, so still 1.15

	arm_rfft_instance_q15  S;

	reset();

	status = arm_rfft_init_q15 (&S, this->npoints, fftFlagForward, fftFlagNotReversed);
	if (status != 0) {
		output ("Failed to initialize RFFT structure\r\n");
		return status;
	}
	// this rfft function scales output to N.(16-N) where N is the order of the FFT (2^10 = 1024, so 1024 samples N = 10)

	uint32_t t0 = HAL_GetTick();
	arm_rfft_q15 (&S, input_data, cDstData);
	fft_time = HAL_GetTick() - t0;

	status = process_complex_results_of_real_valued_fft (cDstData, fftd);
	fft_process_time = HAL_GetTick() - t0;
	if (0) output ("FFT in %d ticks, total processing time %d ticks\r\n", fft_time, fft_process_time);
	// the processing first computes the magnitude squared, which scales by 2 bits (1.15 -> 3.13)
	// then there's an implicit *2 so 3.13 -> 4.12
	// so at this point, fftd data are (N+3).(13-N), so for N of 10, 13.3
	// then we need to divide by the number of samples (2^N)
	// so the fftd data are 3.13 so +/- 8.0

	return status;
}

int fft_analysis::find_peaks (void)
{
	int		n = bins;
	q15_t	pk_v;
	int		pt = 0;

	get_max_value();
	pk_cnt = 0;
	while (pt < n) {
		// first the next 10% value
		while (fftd[pt] < max_value/50 && pt < n) pt++;

		// keep counting until a decrease
		pk_v = fftd[pt];
		while (pt+1 < n && fftd[pt+1] > pk_v) {
			pk_v = fftd[++pt];
		}

		if (pk_cnt + 1 >= MAX_PEAKS) break;

		// we have a peak here, save it
		peaks[pk_cnt].pk_bin = pt;
		peaks[pk_cnt].pk_value_at_bin = pk_v;
		pk_cnt++;

		if (SHOW_PEAKS) output ("peak in bin %d at %f\r\n", pt, pk_v);

		// keep counting while decreasing
		q15_t last_v = fftd[++pt];
		while (pt+1 < n && fftd[pt+1] < last_v) {
			last_v = fftd[++pt];
		}
	}
	if (SHOW_PEAKS) output ("\r\n");

	sort_peaks();
	return pk_cnt;
}


void fft_analysis::sort_peaks (void)
{
	struct peak temp;

	for (int i = 0; i < pk_cnt; i++)
		for (int j = i+1; j < pk_cnt; j++)
			if (peaks[j].pk_value_at_bin > peaks[i].pk_value_at_bin) {
				memcpy (&temp, &peaks[j], sizeof(struct peak));
				memcpy (&peaks[j], &peaks[i], sizeof(struct peak));
				memcpy (&peaks[i], &temp, sizeof(struct peak));
			}
}


int fft_analysis::process_complex_results_of_real_valued_fft (q15_t *cplex, q15_t *real)
{
	// validity checks - imaginary part of DC & Nyquist should be zero
	if (DEBUG_CHECK_COMPLEX) {
		if (cplex[1] != 0) 			output ("PCRORVF: DC complex is %9.4f\r\n", cplex[1]);
		if (cplex[npoints+1] != 0) 	output ("PCRORVF: Nyq complex is %9.4f\r\n", cplex[npoints+1]);
	}

	// calculate magnitudes^2 of complex values for power spectrum
	arm_cmplx_mag_squared_q15 (cplex, real, npoints/2 + 1);

	// endpoints (DC value and Nyquist point) are not duplicated but others are doubled
	real[0] /= 2;
	real[npoints/2] /= 2;

	return 0;
}

q15_t fft_analysis::get_max_value (void)
{
	if (max_value == 0) {
		for (int bin = 0; bin < bins; bin++) {
			q15_t value = abs(fftd[bin]);
			if (value > max_value) max_value = value;
		}
	}

	return max_value;
}


float fft_analysis::interpolate_quadratic (int pk)
{
	int k = peaks[pk].pk_bin;

	if (k > 1 && k+1 < bins) {
		float y1 = fftd[k-1];
		float y2 = fftd[k];
		float y3 = fftd[k+1];

		float d = (y3-y1)/(2* (2*y2-y1-y3));
		output ("Qd bin %4d d %6.3f so bin %8.3f  freq %9.3f KHz\r\n",
				k, d, d+k, bin_frequency(d+k)/1000);

		//X(n0) = X[n] + (2*X[n] - X[n-1] - X[n+1])/2 * (n0-n)^2
		return bin_frequency(d+k);
	}
	return 0;
}


float fft_analysis::bin_frequency (float bin)
{
	return bin * hz_per_bin;
}

void fft_analysis::interpolate_baycentric (int pk)
{
	int k = peaks[pk].pk_bin;

	if (k > 1 && k+1 < bins) {
		float y1 = fftd[k-1];
		float y2 = fftd[k];
		float y3 = fftd[k+1];

		float d = (y3-y1)/(y1 + y2 + y3);
		output ("BC bin %4d d %6.3f so bin %8.3f  freq %9.3f KHz\r\n",
				k, d, d+k, bin_frequency(d+k)/1000);
	}
}


void fft_analysis::interpolate_jains (int pk)
{
	int k = peaks[pk].pk_bin;

	if (k > 1 && k+1 < bins) {
		float y1 = fftd[k-1];
		float y2 = fftd[k];
		float y3 = fftd[k+1];

		float d;
		if (y1 > y3) {
			float a = y2/y1;
			d = a / (1 + a) - 1;
		} else {
			float a = y3/y2;
			d = a / (1 + a);
		}
		output ("Js bin %4d d %6.3f so bin %8.3f  freq %9.3f KHz\r\n",
				k, d, d+k, bin_frequency(d+k)/1000);
	}
}


int fft_analysis::get_peak (int order, struct peak* result)
{
	if (pk_cnt == 0) {
		find_peaks();
	}

	if (order < pk_cnt) {
		peaks[order].pk_interp_freq = interpolate_quadratic (order);
		memcpy (result, &peaks[order], sizeof(struct peak));
		return 0;
	}
	return -1;
}


void fft_analysis::window_hamming (void)
{
	float alpha = 25.0/46;
	float factor = 2.0 * M_PI / npoints;

	for (int i = 0; i < npoints; i++) {
		float f  = alpha - (1.0 - alpha) * arm_cos_f32 (factor * i);
		rWindow[i] = f_to_q15 (f);
	}
	windowed = true;
}
