#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "raylib.h"

#include "constants.h"
#include "scale.h"
#include "candle.h"

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trading");

	CandleData candleData[5];

	LoadCandleData(&candleData[0], "1mo");
	LoadCandleData(&candleData[1], "1w");
	LoadCandleData(&candleData[2], "1d");
	LoadCandleData(&candleData[3], "4h");
	LoadCandleData(&candleData[4], "1h");

	int zoomIndex = 2;
	int zoomLevel = 0;
	float verticalZoomLevel = 0;

	bool logScale = true;

	ScaleData scaleData;

	scaleData.zoom = 1;

	scaleData.horizontalScale = candleData[zoomIndex].candles[0].scale / (ZOOM_THRESHOLD * 2);

	float cameraPosX;
	float cameraPosY;

    SetTargetFPS(100);

	char fpsString[8];

	// Set initial camera X position to show the most recent candle on the right
	{
		Candle* candle = &candleData[zoomIndex].candles[candleData[zoomIndex].candleCount - 1];

		cameraPosX = (candle->timestamp + candle->scale) / scaleData.horizontalScale - SCREEN_WIDTH;
	}

	float cameraTimestamp = cameraPosX * scaleData.horizontalScale;
	float cameraEndTimestamp = (cameraPosX + SCREEN_WIDTH) * scaleData.horizontalScale;

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

	scaleData.verticalScale = initialVerticalScale;

	bool dragging = false;
	bool rightDragging = false;

    // Main Loop
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

		if (dragging)
		{
			cameraPosX -= GetMouseDelta().x;
			cameraPosY -= GetMouseDelta().y;
		}

		// Vertical Scale Adjustment
		if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
		{
			rightDragging = true;
		}

		if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
		{
			rightDragging = false;
		}

		if (rightDragging)
		{
			float cameraCenterY = (cameraPosY + SCREEN_HEIGHT / 2) * scaleData.verticalScale;

			verticalZoomLevel += GetMouseDelta().y;
			scaleData.verticalScale = initialVerticalScale * exp2(verticalZoomLevel / 500);

			cameraPosY = cameraCenterY / scaleData.verticalScale - SCREEN_HEIGHT / 2;
		}

		// Debug zoom adjustment
		if (IsKeyPressed(KEY_A))
		{
			if (zoomIndex < ZOOM_INDEX_COUNT - 1)
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

		if (IsKeyPressed(KEY_L))
		{
			float cameraTop = ToPrice(scaleData, cameraPosY, logScale);
			float cameraBottom = ToPrice(scaleData, cameraPosY + SCREEN_HEIGHT, logScale);

			float cameraTimestamp = ToTimestamp(scaleData, cameraPosX);
			float cameraEndTimestamp = ToTimestamp(scaleData, cameraPosX + SCREEN_WIDTH);

			float priceUpper = 0;
			float priceLower = 10000000;

			// Draw Candles
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

				if (candle->high > priceUpper)
				{
					if (candle->high > cameraTop)
					{
						priceUpper = cameraTop;
					}
					else
					{
						priceUpper = candle->high;
					}
				}

				if (candle->low < priceLower)
				{
					if (candle->low < cameraBottom)
					{
						priceLower = cameraBottom;
					}
					else
					{
						priceLower = candle->low;
					}
				}
			}

			float originalPixelUpper = ToPixelY(scaleData, priceUpper, logScale);
			float originalPixelLower = ToPixelY(scaleData, priceLower, logScale);

			float pixelOffset = originalPixelUpper - cameraPosY;

			logScale = !logScale;

			float pixelUpper = ToPixelY(scaleData, priceUpper, logScale);
			float pixelLower = ToPixelY(scaleData, priceLower, logScale);

			float difference = (pixelLower - pixelUpper) / (originalPixelLower - originalPixelUpper);
			
			initialVerticalScale *= difference;
			scaleData.verticalScale *= difference;

			pixelUpper = ToPixelY(scaleData, priceUpper, logScale);

			cameraPosY = pixelUpper - pixelOffset;
		}

		// Zooming
		if (GetMouseWheelMove() != 0)
		{
			zoomLevel -= GetMouseWheelMove();

			// Remove zoom from screen space as we adjust it
			float cameraCenterX = (cameraPosX + SCREEN_WIDTH / 2) * scaleData.zoom;
			float cameraCenterY = (cameraPosY + SCREEN_HEIGHT / 2) * scaleData.zoom;

			scaleData.zoom = 1;

			int i = zoomLevel;

			while (i > 0)
			{
				scaleData.zoom *= ZOOM_INCREMENT;
				i--;
			}

			while (i < 0)
			{
				scaleData.zoom /= ZOOM_INCREMENT;
				i++;
			}

			zoomIndex = 0;

			while (zoomIndex + 1 < ZOOM_INDEX_COUNT &&
			       ToPixelX(scaleData, candleData[zoomIndex + 1].candles[0].scale) > ZOOM_THRESHOLD)
			{
				zoomIndex++;
			}

			// Re-add zoom post update
			cameraPosX = cameraCenterX / scaleData.zoom - SCREEN_WIDTH / 2;
			cameraPosY = cameraCenterY / scaleData.zoom - SCREEN_HEIGHT / 2;
		}

		// Rendering <><><><><><><><><><><><><><><><><><><><><><><><><><><><>

        BeginDrawing();

		ClearBackground(BLACK);

		DrawRectangle(-cameraPosX, 0, 1, SCREEN_HEIGHT, WHITE);

		float cameraTimestamp = ToTimestamp(scaleData, cameraPosX);
		float cameraEndTimestamp = ToTimestamp(scaleData, cameraPosX + SCREEN_WIDTH);

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

			int xPos = ToPixelX(scaleData, candle->timestamp) - cameraPosX;
			float candleWidth = ToPixelX(scaleData, candle->scale);

			float scaledOpen = ToPixelY(scaleData, candle->open, logScale);
			float scaledClose = ToPixelY(scaleData, candle->close, logScale);
			float scaledHigh = ToPixelY(scaleData, candle->high, logScale);
			float scaledLow = ToPixelY(scaleData, candle->low, logScale);

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

		char candleString[50];

		int timestamp = ToTimestamp(scaleData, GetMouseX() + cameraPosX);

		int index = 0;

		for (int i = 1; i < candleData[zoomIndex].candleCount; i++)
		{
			if (candleData[zoomIndex].candles[i].timestamp < timestamp)
			{
				index++;
			}
			else
			{
				break;
			}
		}
		
		sprintf(candleString, "%i: %f, %f", index, candleData[zoomIndex].candles[index].open, ToPixelY(scaleData, candleData[zoomIndex].candles[index].open, logScale) - cameraPosY);
		DrawText(candleString, 0, 40, 20, RAYWHITE);

		sprintf(candleString, "%i, %i", (int)cameraPosX, (int)cameraPosY);
		DrawText(candleString, 0, 60, 20, RAYWHITE);

		sprintf(candleString, "%f", scaleData.zoom);
		DrawText(candleString, 0, 800, 20, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
}
