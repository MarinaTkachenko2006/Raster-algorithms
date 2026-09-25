#include "algorithms.h"

void bresenhamLine(std::vector<uint8_t>& pixels,
    int width, int height,
    int x0, int y0, int x1, int y1,
    uint8_t r, uint8_t g, uint8_t b)
{
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    // Направления шага
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    // Закраска одного пикселя
    auto put = [&](int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) return; // проверка, не вышли ли за пределы холста
        size_t i = (static_cast<size_t>(y) * width + x) * 4;
        pixels[i + 0] = r;
        pixels[i + 1] = g;
        pixels[i + 2] = b;
        pixels[i + 3] = 255;
        };

    if (dy <= dx) {
        // ---------- gradient <= 1 ----------
        int d = 2 * dy - dx;
        int y = y0;

        for (int x = x0; ; x += sx) {
            put(x, y);

            if (x == x1) break;

            if (d < 0) {
                d += 2 * dy;
            }
            else {
                y += sy;
                d += 2 * (dy - dx);
            }
        }
    }
    else {
        // ---------- gradient > 1 ----------
        int d = 2 * dx - dy;
        int x = x0;

        for (int y = y0; ; y += sy) {
            put(x, y);

            if (y == y1) break;

            if (d < 0) {
                d += 2 * dx;
            }
            else {
                x += sx;
                d += 2 * (dx - dy);
            }
        }
    }
}
