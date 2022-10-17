#ifndef SCALE_H
#define SCALE_H

#include <stdbool.h>

typedef struct ScaleData
{
	float horizontalScale;
	float verticalScale;

	float zoom;
} ScaleData;

float ToPixelX(ScaleData scaleData, int timestamp);
float ToTimestamp(ScaleData scaleData, float screenX);
float ToPixelY(ScaleData scaleData, float price, bool log);
float ToPrice(ScaleData scaleData, float screenY, bool log);

#endif
