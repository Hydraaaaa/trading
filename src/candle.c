#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>

#include "candle.h"

//const long ORIGIN_TIMESTAMP = 1501545600000;
const long ORIGIN_TIMESTAMP = 1502668800000;

struct Candle
{
	float open;
	float high;
	float low;
	float close;
};

struct CandleData
{
	Candle* candles;
	int candleCount;
	int scale;
	float offset;
};

void LoadCandleData(CandleData* candleData, char* fileName, int scale)
{
	candleData->scale = scale;

	FILE* file = fopen(fileName, "r");

	int candleCount = 0;

	// Calculate timestamp interval between candles
	char timestamp1[14];
	char timestamp2[14];

	for (int i = 0; i < 13; i++)
	{
		timestamp1[i] = fgetc(file);
	}

	timestamp1[13] = '\n';

	char ch = fgetc(file);

	while (ch != '\n')
	{
		ch = fgetc(file);
	}

	for (int i = 0; i < 13; i++)
	{
		timestamp2[i] = fgetc(file);
	}

	timestamp2[13] = '\n';

	long timestamp1int = atol(timestamp1);
	long timestamp2int = atol(timestamp2);

	int timestampInterval = timestamp2int - timestamp1int;

	printf("\ntimestamp1int %li - ORIGIN_TIMESTAMP %li\n", timestamp1int, ORIGIN_TIMESTAMP);

	candleData->offset = (timestamp1int - ORIGIN_TIMESTAMP) / (float)timestampInterval;

	printf("timestampInterval %i\n", timestampInterval);
	printf("offset %s %f\n", fileName, candleData->offset);

	rewind(file);

	while(!feof(file))
	{
		char ch = fgetc(file);
		if(ch == '\n')
		{
			candleCount++;
		}
	}

	printf("Candle Count %s: %i\n", fileName, candleCount);

	candleData->candles = malloc(sizeof(Candle) * candleCount);
	candleData->candleCount = candleCount;

	rewind(file);

	int currentCell = 0;
	int currentIndex = 0;
	int lineNumber = 0;

	char* string = malloc(15);

	while (!feof(file))
	{
		char ch = fgetc(file);

		if (ch == ',')
		{
			string[currentIndex] = '\n';

			switch (currentCell)
			{
				case 0: if (lineNumber == candleCount - 1)
						{
							long tmstmp = atof(string);

							printf("Timestamp: %li, Calculated: %li\n", tmstmp, timestamp1int + (long)timestampInterval * (candleCount - 1));
						};
						break; // Open Time
				case 1: candleData->candles[lineNumber].open = atof(string); break; // Open
				case 2: candleData->candles[lineNumber].high = atof(string); break; // High
				case 3: candleData->candles[lineNumber].low = atof(string); break; // Low
				case 4: candleData->candles[lineNumber].close = atof(string); break; // Close
				case 5: break; // Volume
				case 6: break; // Close Time
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
			currentCell = 0;
		}
		else
		{
			string[currentIndex] = ch;
		}

		currentIndex++;
	}
}
