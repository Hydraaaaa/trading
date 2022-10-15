#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>

#include "candle.h"

const long ORIGIN_TIMESTAMP = 1501545600000;

struct Candle
{
	int timestamp;
	int scale; // Determines the width of the candle, necessary because months don't have a fixed size
	float open;
	float high;
	float low;
	float close;
};

struct CandleData
{
	Candle* candles;
	int candleCount;
};

void LoadCandleData(CandleData* candleData, char* fileName)
{
	FILE* file = fopen(fileName, "r");

	int candleCount = 0;

	rewind(file);

	while(!feof(file))
	{
		char ch = fgetc(file);
		if(ch == '\n')
		{
			candleCount++;
		}
	}

	candleData->candles = malloc(sizeof(Candle) * candleCount);
	candleData->candleCount = candleCount;

	rewind(file);

	int currentCell = 0;
	int currentIndex = 0;
	int lineNumber = 0;

	char* string = malloc(15);

	Candle* candle = &candleData->candles[lineNumber];

	while (!feof(file))
	{
		char ch = fgetc(file);

		if (ch == ',')
		{
			string[currentIndex] = '\n';

			switch (currentCell)
			{
				case 0: candle->timestamp = (atol(string) - ORIGIN_TIMESTAMP) / 1000; break; // Open Time
				case 1: candle->open = atof(string); break; // Open
				case 2: candle->high = atof(string); break; // High
				case 3: candle->low = atof(string); break; // Low
				case 4: candle->close = atof(string); break; // Close
				case 5: break; // Volume
				case 6: candle->scale = (atol(string) - ORIGIN_TIMESTAMP + 1) / 1000 - candle->timestamp; break; // Close Time
				case 7: break; // Quote Asset Volume
				case 8: break; // Number of Trades
				case 9: break; // Taker buy base asset volume
				case 10: break; // Taker buy quote asset volume
				default: printf("Unexpected cell");
			}

			currentIndex = -1;
			currentCell++;
		}
		else if (ch == '\n')
		{
			currentIndex = -1;
			lineNumber++;
			candle = &candleData->candles[lineNumber];
			currentCell = 0;
		}
		else
		{
			string[currentIndex] = ch;
		}

		currentIndex++;
	}
}
