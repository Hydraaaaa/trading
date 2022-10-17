#ifndef CANDLE_H
#define CANDLE_H

typedef struct Candle
{
	int timestamp;
	int scale; // Determines the width of the candle, necessary because months don't have a fixed size
	float open;
	float high;
	float low;
	float close;
} Candle;

typedef struct CandleData
{
	Candle* candles;
	int candleCount;
} CandleData;

void LoadCandleData(CandleData* candleData, char* fileName);
void UnloadCandleData(CandleData* candleData);

#endif
