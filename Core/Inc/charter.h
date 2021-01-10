/*
 * charter.h
 *
 *  Created on: Jan 2, 2021
 *      Author: david.winant
 */

#ifndef SRC_CHARTER_H_
#define SRC_CHARTER_H_

#include "fftanalysis.h"

#define SCREEN_BORDER_THICK		2

#define CHART_DISPLAY_WIDTH		440
#define CHART_DISPLAY_HEIGHT	200
#define CHART_INSET_X			10
#define CHART_INSET_Y			10
#define CHART_BORDER_THICK		2

#define MIN(A,B)		(((A) < (B)) ? (A) : (B))
#define MAX(A,B)		(((A) > (B)) ? (A) : (B))


class charter {
public:
	charter();
	virtual ~charter();

	void chart_fft (fft_analysis* fft);
	void plot_function (q15_t* data, float min, float max, int points);

protected:
	virtual float display_value (q15_t y);
	virtual int y_to_height (q15_t y);
	virtual void adjust_full_scale (q15_t max_v);
	virtual void grid_lines (void) {;}

protected:
	void setup_chart (void);
	void draw_screen_border (void);
	void draw_chart_border (void);
	void draw_chart (q15_t* data, int points);
	void draw_log_chart (q15_t* data, int points);
	void note_peak (struct peak* pk);

	float full_scale_value;
	int   ybase;
	int   xlimit;
};

#endif /* SRC_CHARTER_H_ */
