/*
 * fftanalysis.h
 *
 *  Created on: Jan 1, 2021
 *      Author: david.winant
 */

#ifndef SRC_FFTANALYSIS_H_
#define SRC_FFTANALYSIS_H_

#define ARM_MATH_CM7

#include "main.h"
#include "arm_math.h"

#define MAX_PEAKS		20

struct peak {
	int			pk_bin;
	q15_t		pk_value_at_bin;
	float		pk_interp_freq;
	float		pk_interp_value;
};

class fft_analysis {
public:
	fft_analysis (int _npoints, long _sample_rate);
	virtual ~fft_analysis();

	int perform_fft (q15_t* input_data);
	int find_peaks (void);

	q15_t get_max_value (void);
	float bin_frequency (float bin);

	int get_peak (int order, struct peak* result);

	void window_hamming (void);

protected:
	int process_complex_results_of_real_valued_fft (q15_t *cplex, q15_t *real);
	float interpolate_quadratic (int pk);
	void interpolate_baycentric (int pk);
	void interpolate_jains (int pk);
	void sort_peaks (void);
	void reset (void);


public:
	q15_t *		fftd;
	q15_t *		rWindow;
	q15_t *		cDstData;
	int			bins;
	int			npoints;
	long		sample_rate;

protected:
	uint32_t	fft_time;
	uint32_t	fft_process_time;
	q15_t		max_value;
	float		hz_per_bin;
	int			windowed;

	struct peak peaks[MAX_PEAKS];
	int 		pk_cnt;
};

#endif /* SRC_FFTANALYSIS_H_ */
