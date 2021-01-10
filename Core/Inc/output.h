/*
 * output.h
 *
 *  Created on: Dec 13, 2020
 *      Author: davew
 */

#ifndef APPLICATION_USER_INCLUDE_OUTPUT_H_
#define APPLICATION_USER_INCLUDE_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void output_string (const char* s);
extern void output (const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_USER_INCLUDE_OUTPUT_H_ */
