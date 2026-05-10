#include <windows.h>
#include <gdiplus.h>
#include <ctime>
#include <cmath>

using namespace Gdiplus;

// Terraria-style LCG (sugoi organic randomness~!)
class TerrariaRNG {
private:
    unsigned long long seed;
public:
    TerrariaRNG() : seed(static_cast<unsigned long long>(time(0))) {}
    int next(int min, int max) {
        seed = seed * 0x5DEECE66DULL + 0xBULL;
        return min + static_cast<int>((seed >> 16) % (max - min + 1));
    }
    float nextFloat() {
        return static_cast<float>(next(0, 1000)) / 1000.0f;
    }
};

int main() {
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Get the path of the current executable
    WCHAR exePath[MAX_PATH + 1];
    GetModuleFileNameW(NULL, exePath, sizeof(exePath));

    // Modify the path to save random_bg.png in the same directory as the .exe
    WCHAR *lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = L'\0'; // Terminate the string after the last slash
    }
    
    lstrcatW(exePath, L"rand_bg.bmp");

    // Set up Terraria RNG (kawaii~!)
    TerrariaRNG rng;

    // Random offsets for pattern variation each run
    float offsetX = rng.nextFloat() * 10.0f;
    float offsetY = rng.nextFloat() * 10.0f;

    // Create a 1920x1080 bitmap
    Bitmap bitmap(1920, 1080, PixelFormat32bppARGB);
    Graphics graphics(&bitmap);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // Generate multi-color gradient background (vibrant Terraria biome~!)
    Bitmap noiseMap(1920, 1080, PixelFormat32bppARGB);
    float scale = 0.003f; // Flowy pattern
    float warpScale = 0.001f; // For organic warping
    for (int y = 0; y < 1080; ++y) {
        for (int x = 0; x < 1920; ++x) {
            // Base coordinates with random offset
            float fx = static_cast<float>(x) * scale + offsetX;
            float fy = static_cast<float>(y) * scale + offsetY;

            // Add organic warping to break grid pattern
            float warpX = sinf(fy * warpScale + offsetX) * 0.5f;
            float warpY = cosf(fx * warpScale + offsetY) * 0.5f;
            fx += warpX;
            fy += warpY;

            // Multi-color gradient using sin/cos for smooth transitions
            float rValue = (sinf(fx * 3.14159f) + 1.0f) * 0.5f; // Red channel
            float gValue = (cosf(fy * 3.14159f) + 1.0f) * 0.5f; // Green channel
            float bValue = (sinf((fx + fy) * 3.14159f) + 1.0f) * 0.5f; // Blue channel

            // Define min and max for color range
            int mn = 64, mx = 255;

            // Apply color range using mn and mx
            int r = static_cast<int>(mn + rValue * (mx - mn));
            int g = static_cast<int>(mn + gValue * (mx - mn));
            int b = static_cast<int>(mn + bValue * (mx - mn));
            noiseMap.SetPixel(x, y, Color(255, r, g, b));
        }
    }
    graphics.DrawImage(&noiseMap, 0, 0);

    // Draw all shapes with rotation and random transparency (sparkly~!)
    for (int i = 0; i < 100; ++i) {
        int x = rng.next(0, 1920);
        int y = rng.next(0, 1080);
        int size = rng.next(10, 60);
        int alpha = rng.next(100, 255); // Random transparency
        Color shapeColor(alpha, rng.next(0, 255), rng.next(0, 255), rng.next(0, 255));
        SolidBrush brush(shapeColor);

        // Save the graphics state before rotation
        GraphicsState state = graphics.Save();

        // Set rotation (in degrees)
        float rotation = rng.nextFloat() * 360.0f; // Random angle (0-360 degrees)
        graphics.TranslateTransform(static_cast<float>(x + size / 2), static_cast<float>(y + size / 2)); // Move origin to shape center
        graphics.RotateTransform(rotation); // Apply rotation
        graphics.TranslateTransform(static_cast<float>(-size / 2), static_cast<float>(-size / 2)); // Adjust for drawing

        int shapeType = i % 3; // Cycle through shapes
        if (shapeType == 0) {
            // Circles (bubbly~! No rotation needed, but included in transform for consistency)
            graphics.FillEllipse(&brush, 0, 0, size, size);
        } else if (shapeType == 1) {
            // Squares (blocky cute~!)
            graphics.FillRectangle(&brush, 0, 0, size, size);
        } else {
            // Triangles (sharp and cool~!)
            Point points[3] = {
                Point(0, 0),
                Point(size, 0),
                Point(size / 2, size)
            };
            graphics.FillPolygon(&brush, points, 3);
        }

        // Restore the graphics state to reset the transform
        graphics.Restore(state);
    }

    // Save the image
    CLSID clsid;
    CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &clsid); // BMP CLSID
    Status saveStatus = bitmap.Save(exePath, &clsid, NULL);
    if (saveStatus != Ok) {
        MessageBoxW(NULL, L"Failed to save bitmap! Check permissions or path.\n", L"Error", MB_OK | MB_ICONERROR);
    }

    // Set as wallpaper
    SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, exePath, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

	//Sleep(500);

    // Cleanup
    GdiplusShutdown(gdiplusToken);
    
	ExitProcess(0);
	return 0;
}
