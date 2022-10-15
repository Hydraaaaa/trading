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

	LoadCandleData(&candleData[0], "1mo");
	LoadCandleData(&candleData[1], "1w");
	LoadCandleData(&candleData[2], "1d");
	LoadCandleData(&candleData[3], "4h");
	LoadCandleData(&candleData[4], "1h");

	int zoomIndex = 2;
	int zoomLevel = 0;
	float verticalZoomLevel = 0;

	const float SCALE_INCREMENT = 1.05f;
	const float SCALE_THRESHOLD = 2.5f;

	float initialScale = candleData[zoomIndex].candles[0].scale / (SCALE_THRESHOLD * 2);
	float scale = initialScale;

	float initialVerticalScale = initialScale * 1000;
	float verticalScale = initialVerticalScale;

    SetTargetFPS(100);

	char fpsString[8];


	float cameraPosX;
	float cameraPosY;

	{
		Candle* candle = &candleData[zoomIndex].candles[candleData[zoomIndex].candleCount - 1];

		cameraPosX = (candle->timestamp + candle->scale) / scale - SCREEN_WIDTH;
		cameraPosY = -(log10(candle->close) * verticalScale / scale) - SCREEN_HEIGHT / 2.0f;
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
			}
		}

		if (IsKeyPressed(KEY_S))
		{
			if (zoomIndex > 0)
			{
				zoomIndex--;
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
			       scale < candleData[zoomIndex + 1].candles[0].scale / SCALE_THRESHOLD)
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

		DrawRectangle(-cameraPosX, 0, 1, SCREEN_HEIGHT, WHITE);

		float cameraTimestamp = cameraPosX * scale;
		float cameraEndTimestamp = (cameraPosX + SCREEN_WIDTH) * scale;

		// Draw Candles
		for (int i = 0; i < candleData[zoomIndex].candleCount; i++)
		{
			Candle* candle = &candleData[zoomIndex].candles[i];

			if (candle->timestamp > cameraEndTimestamp)
			{
				break;
			}

			if (candle->timestamp + candle->scale < cameraTimestamp)
			{
				continue;
			}

			int xPos = candle->timestamp / scale - cameraPosX;
			float candleWidth = candle->scale / scale;

			float scaledOpen = -(log10(candle->open) * verticalScale / scale);
			float scaledClose = -(log10(candle->close) * verticalScale / scale);
			float scaledHigh = -(log10(candle->high) * verticalScale / scale);
			float scaledLow = -(log10(candle->low) * verticalScale / scale);

			if (scaledClose > scaledOpen)
			{
				float candleHeight = scaledClose - scaledOpen;

				if (candleHeight < 1)
				{
					candleHeight = 1;
				}

				DrawRectangle(xPos, scaledOpen - cameraPosY, candleWidth, candleHeight, RED);
				DrawRectangle(xPos + candleWidth / 2 - 0.5f, scaledHigh - cameraPosY, 1, scaledLow - scaledHigh, RED);
			}
			else
			{
				float candleHeight = scaledOpen - scaledClose;

				if (candleHeight < 1)
				{
					candleHeight = 1;
				}

				DrawRectangle(xPos, scaledClose - cameraPosY, candleWidth, candleHeight, GREEN);
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

		float mousePosX = (cameraPosX + GetMouseX()) * scale;
		int mouseIndex = candleData[zoomIndex].candleCount;

		for (int i = 0; i < candleData[zoomIndex].candleCount; i++)
		{
			if (candleData[zoomIndex].candles[i].scale > mousePosX)
			{
				mouseIndex = i;
				break;
			}
		}

		Candle candle = candleData[zoomIndex].candles[mouseIndex];

		sprintf(candleString, "%i: %i - %i - %i - %i", mouseIndex, (int)candle.open, (int)candle.high, (int)candle.low, (int)candle.close);
		DrawText(candleString, 0, 60, 20, RAYWHITE);

		sprintf(candleString, "%i, %i", (int)cameraPosX, (int)cameraPosY);
		DrawText(candleString, 0, 80, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
}
