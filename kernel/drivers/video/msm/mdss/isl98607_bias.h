/* Bias IC head file*/


#ifndef __ISL98607_H
#define __ISL98607_H

enum bias_mode{
	BIAS_MODE_5P5V,
	BIAS_MODE_5V,
	BIAS_MODE_SLEEP,
};

extern int isl98607_set_mode(enum bias_mode isl98607_mode);
#endif
