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

	CandleData candleData[5];

	const int zoomIndexCount = 5;

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

	float horizontalScale = candleData[zoomIndex].candles[0].scale / (SCALE_THRESHOLD * 2);
	float zoom = 1;

    SetTargetFPS(100);

	char fpsString[8];

	float cameraPosX;
	float cameraPosY;

	// Set initial camera X position to show the most recent candle on the right
	{
		Candle* candle = &candleData[zoomIndex].candles[candleData[zoomIndex].candleCount - 1];

		cameraPosX = (candle->timestamp + candle->scale) / horizontalScale - SCREEN_WIDTH;
	}

	float cameraTimestamp = cameraPosX * horizontalScale;
	float cameraEndTimestamp = (cameraPosX + SCREEN_WIDTH) * horizontalScale;

	float initialVerticalScale;

	// Set initial vertical scale to fit all initially visible candles on screen
	{
		float low = 10000000;
		float high = 0;

		for (int i = 1; i < candleData[zoomIndex].candleCount; i++)
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

			if (candle->low < low)
			{
				low = candle->low;
			}

			if (candle->high > high)
			{
				high = candle->high;
			}
		}

		float middle = (log10(high) + log10(low)) / 2.0f;

		initialVerticalScale = (log10(high) - log10(low)) / (SCREEN_HEIGHT - 64);

		cameraPosY = -(middle / initialVerticalScale) - SCREEN_HEIGHT / 2.0f;
	}

	float verticalScale = initialVerticalScale;

	// Mouse selection
	bool dragging = false;
	bool rightDragging = false;

    // Main game loop
    while (!WindowShouldClose())
    {
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
			// Remove zoom from screen space as we adjust it
			float cameraCenterX = (cameraPosX + SCREEN_WIDTH / 2) * zoom;
			float cameraCenterY = (cameraPosY + SCREEN_HEIGHT / 2) * zoom;

			zoomLevel -= GetMouseWheelMove();

			zoom = 1;

			int i = zoomLevel;

			while (i > 0)
			{
				zoom *= SCALE_INCREMENT;
				i--;
			}

			while (i < 0)
			{
				zoom /= SCALE_INCREMENT;
				i++;
			}

			zoomIndex = 0;

			while (zoomIndex + 1 < zoomIndexCount &&
			       (zoom * horizontalScale) < candleData[zoomIndex + 1].candles[0].scale / SCALE_THRESHOLD)
			{
				zoomIndex++;
			}

			// Re-add zoom post update
			cameraPosX = cameraCenterX / zoom - SCREEN_WIDTH / 2;
			cameraPosY = cameraCenterY / zoom - SCREEN_HEIGHT / 2;
		}

		if (dragging)
		{
			cameraPosX -= GetMouseDelta().x;
			cameraPosY -= GetMouseDelta().y;
		}

		if (rightDragging)
		{
			float cameraCenterY = (cameraPosY + SCREEN_HEIGHT / 2) * verticalScale;

			verticalZoomLevel += GetMouseDelta().y;
			verticalScale = initialVerticalScale * exp2(verticalZoomLevel / 500);

			cameraPosY = cameraCenterY / verticalScale - SCREEN_HEIGHT / 2;
		}

		// Rendering <><><><><><><><><><><><><><><><><><><><><><><><><><><><>

        BeginDrawing();

		ClearBackground(BLACK);

		DrawRectangle(-cameraPosX, 0, 1, SCREEN_HEIGHT, WHITE);

		float cameraTimestamp = cameraPosX * (horizontalScale * zoom);
		float cameraEndTimestamp = (cameraPosX + SCREEN_WIDTH) * (horizontalScale * zoom);

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

			int xPos = candle->timestamp / (horizontalScale * zoom) - cameraPosX;
			float candleWidth = candle->scale / (horizontalScale * zoom);

			float scaledOpen = -(log10(candle->open) / (verticalScale * zoom));
			float scaledClose = -(log10(candle->close) / (verticalScale * zoom));
			float scaledHigh = -(log10(candle->high) / (verticalScale * zoom));
			float scaledLow = -(log10(candle->low) / (verticalScale * zoom));

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

		sprintf(fpsString, "%i", zoomIndex);

		DrawText(fpsString, 0, 20, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
}
