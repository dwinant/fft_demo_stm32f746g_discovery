/*
 * charterlog.h
 *
 *  Created on: Jan 9, 2021
 *      Author: david.winant
 */

#ifndef SRC_CHARTERLOG_H_
#define SRC_CHARTERLOG_H_

#include "charter.h"

class charter_log: public charter {
public:
	charter_log();
	virtual ~charter_log();

protected:
	virtual float display_value (q15_t y);
	virtual int y_to_height (q15_t y);
	virtual void adjust_full_scale (q15_t max_v) {;}
	virtual void grid_lines (void);
};

#endif /* SRC_CHARTERLOG_H_ */
