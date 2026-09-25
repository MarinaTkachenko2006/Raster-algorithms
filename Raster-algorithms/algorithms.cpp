#include "algorithms.h"

inline bool sameColor(const std::vector<uint8_t>& p, size_t i, uint8_t r, uint8_t g, uint8_t b)
{
    return p[i + 0] == r && p[i + 1] == g && p[i + 2] == b;
}

inline void setPixel(std::vector<uint8_t>& p, size_t i, uint8_t r, uint8_t g, uint8_t b)
{
    p[i + 0] = r;
    p[i + 1] = g;
    p[i + 2] = b;
    p[i + 3] = 255;
}

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

void floodFillSeries(std::vector<uint8_t>& pixels, int width, int height, int x, int y, uint8_t tr, uint8_t tg, uint8_t tb, uint8_t fr, uint8_t fg, uint8_t fb)
{
    // Проверка выхода за пределы холста
    if (x < 0 || x >= width || y < 0 || y >= height) return;

    size_t i0 = (static_cast<size_t>(y) * width + x) * 4;

    // Если текущая точка уже закрашена цветом заливки — выходим
    if (sameColor(pixels, i0, fr, fg, fb)) return;

    // Если текущая точка НЕ того цвета, который мы закрашиваем — это граница, выходим
    if (!sameColor(pixels, i0, tr, tg, tb)) return;

    // Находим левую границу серии: идём влево, пока пиксель равен целевому цвету и ещё не залит цветом заливки
    int left = x;
    while (left - 1 >= 0) {
        size_t i = (static_cast<size_t>(y) * width + (left - 1)) * 4;
        if (!sameColor(pixels, i, tr, tg, tb)) break; // другой цвет — граница
        if (sameColor(pixels, i, fr, fg, fb))  break; // уже залито
        --left;
    }

    // Находим правую границу серии
    int right = x;
    while (right + 1 < width) {
        size_t i = (static_cast<size_t>(y) * width + (right + 1)) * 4;
        if (!sameColor(pixels, i, tr, tg, tb)) break;
        if (sameColor(pixels, i, fr, fg, fb))  break;
        ++right;
    }

    // Заливаем серию [left; right]
    for (int xi = left; xi <= right; ++xi) {
        size_t i = (static_cast<size_t>(y) * width + xi) * 4;
        setPixel(pixels, i, fr, fg, fb);
    }

    // Рекурсивно обрабатываем строку выше и строку ниже
    for (int xi = left; xi <= right; ++xi) {
        if (y - 1 >= 0) {
            size_t i = (static_cast<size_t>(y - 1) * width + xi) * 4;
            if (sameColor(pixels, i, tr, tg, tb) &&
                !sameColor(pixels, i, fr, fg, fb))
            {
                floodFillSeries(pixels, width, height,
                    xi, y - 1,
                    tr, tg, tb, fr, fg, fb);
            }
        }
        if (y + 1 < height) {
            size_t i = (static_cast<size_t>(y + 1) * width + xi) * 4;
            if (sameColor(pixels, i, tr, tg, tb) &&
                !sameColor(pixels, i, fr, fg, fb))
            {
                floodFillSeries(pixels, width, height,
                    xi, y + 1,
                    tr, tg, tb, fr, fg, fb);
            }
        }
    }
}
