#ifndef CANDLE_H
#define CANDLE_H

char* DAILY_DATA_PATH = "../../../binancedata/spot/daily/klines/BTCUSDT/";
char* MONTHLY_DATA_PATH = "../../../binancedata/spot/monthly/klines/BTCUSDT/";

typedef struct Candle Candle;

typedef struct CandleData CandleData;

void LoadCandleData(CandleData* candleData, char* fileName, int scale);

#endif
