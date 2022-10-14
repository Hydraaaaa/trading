#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include <math.h>

#include "candle.c"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 720

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trading");

	CandleData candleData[4];

	const int zoomIndexCount = 4;

	LoadCandleData(&candleData[0], "1w", 10080);
	LoadCandleData(&candleData[1], "1d", 1440);
	LoadCandleData(&candleData[2], "4h", 240);
	LoadCandleData(&candleData[3], "1h", 60);

	int zoomIndex = 1;
	int zoomLevel = 0;
	float verticalZoomLevel = 0;

	const float SCALE_INCREMENT = 1.05f;
	const float SCALE_THRESHOLD = 1.0f;

	float initialScale = candleData[zoomIndex].scale / SCALE_THRESHOLD;
	float scale = initialScale;

	float initialVerticalScale = 500000;
	float verticalScale = initialVerticalScale;

    SetTargetFPS(100);

	char fpsString[8];


	float cameraPosX;
	float cameraPosY;

	{
		float candleWidth = candleData[zoomIndex].scale / scale;
		int candleCount = candleData[zoomIndex].candleCount - 1;
		cameraPosX = candleCount * candleWidth - SCREEN_WIDTH;
		cameraPosY = -(log10(candleData[zoomIndex].candles[candleCount].close) * verticalScale / scale) - SCREEN_HEIGHT / 2.0f;
	}

	// Mouse selection
	bool dragging = false;
	bool rightDragging = false;

    // Main game loop
    while (!WindowShouldClose())
    {
		float time = GetFrameTime();

		// Camera Panning
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			dragging = true;
		}

		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
		{
			dragging = false;
		}

		if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
		{
			rightDragging = true;
		}

		if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
		{
			rightDragging = false;
		}

		if (IsKeyPressed(KEY_A))
		{
			if (zoomIndex < zoomIndexCount - 1)
			{
				zoomIndex++;

				printf("Candle Count: %i", candleData[zoomIndex].candleCount);
				printf(" - Candle Scale: %i", candleData[zoomIndex].scale);
				printf(" - Candle Offset: %f", candleData[zoomIndex].offset);
				printf(" - Combined: %f\n", (candleData[zoomIndex].candleCount + candleData[zoomIndex].offset) * candleData[zoomIndex].scale);
			}
		}

		if (IsKeyPressed(KEY_S))
		{
			if (zoomIndex > 0)
			{
				zoomIndex--;

				printf("Candle Count: %i", candleData[zoomIndex].candleCount);
				printf(" - Candle Scale: %i", candleData[zoomIndex].scale);
				printf(" - Candle Offset: %f", candleData[zoomIndex].offset);
				printf(" - Combined: %f\n", (candleData[zoomIndex].candleCount + candleData[zoomIndex].offset) * candleData[zoomIndex].scale);
			}
		}

		if (GetMouseWheelMove() != 0)
		{
			// Remove scale from screen space as we adjust it
			float cameraCenterX = (cameraPosX + SCREEN_WIDTH / 2) * scale;
			float cameraCenterY = (cameraPosY + SCREEN_HEIGHT / 2) * scale;

			zoomLevel -= GetMouseWheelMove();

			scale = initialScale;

			int i = zoomLevel;

			while (i > 0)
			{
				scale *= SCALE_INCREMENT;
				i--;
			}

			while (i < 0)
			{
				scale /= SCALE_INCREMENT;
				i++;
			}

			zoomIndex = 0;

			while (zoomIndex + 1 < zoomIndexCount &&
			       scale < candleData[zoomIndex + 1].scale / SCALE_THRESHOLD)
			{
				zoomIndex++;
			}

			// Re-add scale post update
			cameraPosX = cameraCenterX / scale - SCREEN_WIDTH / 2;
			cameraPosY = cameraCenterY / scale - SCREEN_HEIGHT / 2;
		}

		if (dragging)
		{
			cameraPosX -= GetMouseDelta().x;
			cameraPosY -= GetMouseDelta().y;
		}

		if (rightDragging)
		{
			float cameraCenterY = (cameraPosY + SCREEN_HEIGHT / 2) / verticalScale;

			verticalZoomLevel -= GetMouseDelta().y;
			verticalScale = initialVerticalScale * exp2(verticalZoomLevel / 500);

			cameraPosY = cameraCenterY * verticalScale - SCREEN_HEIGHT / 2;
		}

		// Rendering <><><><><><><><><><><><><><><><><><><><><><><><><><><><>

        BeginDrawing();

		ClearBackground(BLACK);

		float candleWidth = candleData[zoomIndex].scale / scale;

		int startingCandle = cameraPosX / candleWidth - candleData[zoomIndex].offset;
		int endingCandle = (cameraPosX + SCREEN_WIDTH) / candleWidth + 1 - candleData[zoomIndex].offset;

		if (startingCandle < 0)
		{
			startingCandle = 0;
		}

		if (endingCandle >= candleData[zoomIndex].candleCount)
		{
			endingCandle = candleData[zoomIndex].candleCount - 1;
		}

		DrawRectangle(-cameraPosX, 0, 1, SCREEN_HEIGHT, WHITE);

		// Draw Candles
		for (int i = startingCandle; i < endingCandle; i++)
		{
			float scaledOpen = -(log10(candleData[zoomIndex].candles[i].open) * verticalScale / scale);
			float scaledClose = -(log10(candleData[zoomIndex].candles[i].close) * verticalScale / scale);
			float scaledHigh = -(log10(candleData[zoomIndex].candles[i].high) * verticalScale / scale);
			float scaledLow = -(log10(candleData[zoomIndex].candles[i].low) * verticalScale / scale);

			int xPos = (i + candleData[zoomIndex].offset) * candleWidth - cameraPosX;

			if (scaledClose > scaledOpen)
			{
				DrawRectangle(xPos, scaledOpen - cameraPosY, candleWidth, scaledClose - scaledOpen, RED);
				DrawRectangle(xPos + candleWidth / 2 - 0.5f, scaledHigh - cameraPosY, 1, scaledLow - scaledHigh, RED);
			}
			else
			{
				DrawRectangle(xPos, scaledClose - cameraPosY, candleWidth, scaledOpen - scaledClose, GREEN);
				DrawRectangle(xPos + candleWidth / 2 - 0.5f, scaledHigh - cameraPosY, 1, scaledLow - scaledHigh, GREEN);
			}
		}

		sprintf(fpsString, "%i", GetFPS());

		DrawText(fpsString, 0, 0, 20, RAYWHITE);

		sprintf(fpsString, "%i", (int)scale);

		DrawText(fpsString, 0, 20, 20, RAYWHITE);

		sprintf(fpsString, "%i", zoomIndex);

		DrawText(fpsString, 0, 40, 20, RAYWHITE);

		char candleString[50];

		int hoverIndex = (cameraPosX + GetMouseX()) / candleWidth;

		if (hoverIndex < 0)
		{
			hoverIndex = 0;
		}

		if (hoverIndex >= candleData[zoomIndex].candleCount)
		{
			hoverIndex = candleData[zoomIndex].candleCount - 1;
		}

		Candle candle = candleData[zoomIndex].candles[hoverIndex];

		sprintf(candleString, "%i: %i - %i - %i - %i", hoverIndex, (int)candle.open, (int)candle.high, (int)candle.low, (int)candle.close);
		DrawText(candleString, 0, 60, 20, RAYWHITE);

		sprintf(candleString, "%i", (int)cameraPosY);
		DrawText(candleString, 0, 80, 20, RAYWHITE);

		sprintf(candleString, "%i", (int)verticalScale);
		DrawText(candleString, 0, 100, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
}
