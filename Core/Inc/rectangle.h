/*
 * rectangle.h
 *
 *  Created on: Jan 2, 2021
 *      Author: david.winant
 */

#ifndef SRC_RECTANGLE_H_
#define SRC_RECTANGLE_H_

#include <stdint.h>

class rectangle {
public:
	rectangle(int16_t x=0, int16_t y=0, int16_t w=0, int16_t h=0)
	: X(x), Y(y), W(w), H(h) {;}

	virtual ~rectangle();

	int16_t		X,Y,W,H;

	void outline (int16_t offset = 0);
};

#endif /* SRC_RECTANGLE_H_ */
