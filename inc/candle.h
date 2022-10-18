#ifndef CANDLE_H
#define CANDLE_H

#include "raylib.h"

typedef struct Candle
{
	int timestamp;
	int scale; // Determines the width of the candle, necessary because months don't have a fixed size
	float open;
	float high;
	float low;
	float close;
} Candle;

typedef struct CandleCloseLevel
{
	int startTimestamp;
	int endTimestamp;
	float price;
} CandleCloseLevel;

typedef struct CandleData
{
	Candle* candles;
	int candleCount;

	CandleCloseLevel* closeLevels;
	int closeLevelCount;

	Color closeLevelColor;
} CandleData;

void LoadCandleData(CandleData* candleData, char* fileName);
void CreateCloseLevels(CandleData* candleData, Color closeLevelColor);
void DestroyCloseLevels(CandleData* candleData);
void UnloadCandleData(CandleData* candleData);

#endif
