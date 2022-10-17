#include <stdbool.h>
#include <math.h>

#include "scale.h"

float ToScreenX(ScaleData scaleData, int timestamp)
{
	return timestamp / (scaleData.horizontalScale * scaleData.zoom);
}

float ToTimestamp(ScaleData scaleData, float screenX)
{
	return screenX * (scaleData.horizontalScale * scaleData.zoom);
}

float ToScreenY(ScaleData scaleData, float price, bool log)
{
	if (log)
	{
		return -(log10(price) / (scaleData.verticalScale * scaleData.zoom));
	}
	else
	{
		return -(price / (scaleData.verticalScale * scaleData.zoom));
	}
}

float ToPrice(ScaleData scaleData, float screenY, bool log)
{
	if (log)
	{
		return -pow(10, (screenY * (scaleData.verticalScale * scaleData.zoom)));
	}
	else
	{
		return -(screenY * (scaleData.verticalScale * scaleData.zoom));
	}
}
