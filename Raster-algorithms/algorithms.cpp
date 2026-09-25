#include "algorithms.h"

inline bool sameColor(const std::vector<uint8_t>& p, size_t i, uint8_t r, uint8_t g, uint8_t b)
{
    return p[i + 0] == r && p[i + 1] == g && p[i + 2] == b;
}

// циклический индекс(a mod n)
inline int cyclic_index(int a, int n)
{
    int m = a % n;
    return (m < 0) ? m + n : m;
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
        // gradient <= 1 
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
        // gradient > 1
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

void floodFillTextureSeries(std::vector<uint8_t>& pixels, int width, int height, int x, int y, 
    uint8_t tr, uint8_t tg, uint8_t tb, 
    int anchorX, int anchorY, const uint8_t* texData, int texWidth, int texHeight,
    std::vector<uint8_t>& visited)
{
    // Левая граница серии (с учётом visited и целевого цвета)
    int left = x;
    while (left - 1 >= 0) {
        size_t idx = static_cast<size_t>(y) * width + (left - 1);
        if (visited[idx]) break;
        size_t i = idx * 4;
        if (!sameColor(pixels, i, tr, tg, tb)) break;
        --left;
    }

    // Правая граница серии
    int right = x;
    while (right + 1 < width) {
        size_t idx = static_cast<size_t>(y) * width + (right + 1);
        if (visited[idx]) break;
        size_t i = idx * 4;
        if (!sameColor(pixels, i, tr, tg, tb)) break;
        ++right;
    }

    // Помечаем всю серию как посещённую и закрашиваем её 
    const int anchorTexX = texWidth / 2;
    const int anchorTexY = texHeight / 2;

    for (int xi = left; xi <= right; ++xi) {
        size_t idx = static_cast<size_t>(y) * width + xi;
        visited[idx] = 1;

        int tx = cyclic_index(anchorTexX + (xi - anchorX), texWidth);
        int ty = cyclic_index(anchorTexY + (y - anchorY), texHeight);

        size_t si = (static_cast<size_t>(ty) * texWidth + tx) * 3;
        size_t di = idx * 4;

        pixels[di + 0] = texData[si + 0];
        pixels[di + 1] = texData[si + 1];
        pixels[di + 2] = texData[si + 2];
        pixels[di + 3] = 255;
    }

    // Идём по серии и отслеживаем момент, когда пиксель выше/ниже
    // был НЕ целевым, а стал целевым — это начало новой серии. Тогда
    // вызываем рекурсию один раз от этого пикселя.
    // Пропускаем уже посещённые пиксели.

    // Выше
    if (y - 1 >= 0) {
        bool inSegment = false;
        for (int xi = left; xi <= right; ++xi) {
            size_t idx = static_cast<size_t>(y - 1) * width + xi;
            bool ok = !visited[idx] && sameColor(pixels, idx * 4, tr, tg, tb);
            if (ok && !inSegment) {
                // начало нового сегмента
                floodFillTextureSeries(pixels, width, height,
                    xi, y - 1,
                    tr, tg, tb,
                    anchorX, anchorY,
                    texData, texWidth, texHeight,
                    visited);
                inSegment = true;
            }
            else if (!ok) {
                inSegment = false;
            }
        }
    }

    // Ниже
    if (y + 1 < height) {
        bool inSegment = false;
        for (int xi = left; xi <= right; ++xi) {
            size_t idx = static_cast<size_t>(y + 1) * width + xi;
            bool ok = !visited[idx] && sameColor(pixels, idx * 4, tr, tg, tb);
            if (ok && !inSegment) {
                floodFillTextureSeries(pixels, width, height,
                    xi, y + 1,
                    tr, tg, tb,
                    anchorX, anchorY,
                    texData, texWidth, texHeight,
                    visited);
                inSegment = true;
            }
            else if (!ok) {
                inSegment = false;
            }
        }
    }
}

// Смещения по 8 направлениям для алгоритма выделения границы, dir 0 = вправо, далее по часовой стрелке
// в экранных координатах (Y растёт вниз)
static const int DX[8] = { +1, +1,  0, -1, -1, -1,  0, +1 };
static const int DY[8] = { 0, +1, +1, +1,  0, -1, -1, -1 };

//  Поиск первой граничной точки: идём влево от клика, пока не встретим пиксель цвета границы. Если дошли до края холста — идём вправо.
bool findBoundaryStart(const std::vector<uint8_t>& pixels,
    int width, int height,
    int clickX, int clickY,
    int& outX, int& outY,
    uint8_t& borderR, uint8_t& borderG, uint8_t& borderB)
{
    if (clickX < 0 || clickX >= width || clickY < 0 || clickY >= height)
        return false;

    // Цвет области — цвет пикселя под кликом
    size_t iClick = (static_cast<size_t>(clickY) * width + clickX) * 4;
    uint8_t ar = pixels[iClick + 0];
    uint8_t ag = pixels[iClick + 1];
    uint8_t ab = pixels[iClick + 2];

    // Идём влево, пока цвет совпадает с цветом области.
    // Первый пиксель с другим цветом — граница; запоминаем её цвет.
    for (int x = clickX - 1; x >= 0; --x) {
        size_t i = (static_cast<size_t>(clickY) * width + x) * 4;
        bool sameAsArea = (pixels[i + 0] == ar &&
            pixels[i + 1] == ag &&
            pixels[i + 2] == ab);
        if (!sameAsArea) {
            outX = x;
            outY = clickY;
            borderR = pixels[i + 0];
            borderG = pixels[i + 1];
            borderB = pixels[i + 2];
            return true;
        }
    }

    // Слева не нашли — идём вправо
    for (int x = clickX + 1; x < width; ++x) {
        size_t i = (static_cast<size_t>(clickY) * width + x) * 4;
        bool sameAsArea = (pixels[i + 0] == ar &&
            pixels[i + 1] == ag &&
            pixels[i + 2] == ab);
        if (!sameAsArea) {
            outX = x;
            outY = clickY;
            borderR = pixels[i + 0];
            borderG = pixels[i + 1];
            borderB = pixels[i + 2];
            return true;
        }
    }

    return false;
}

//  Обход границы
//  Все граничные пиксели одного цвета (br,bg,bb).
//  Порядок обхода — по часовой стрелке.
bool traceBoundary(const std::vector<uint8_t>& pixels,
    int width, int height,
    int startX, int startY,
    uint8_t borderR, uint8_t borderG, uint8_t borderB,
    std::vector<std::pair<int, int>>& outBoundary)
{
    outBoundary.clear();

    if (startX < 0 || startX >= width || startY < 0 || startY >= height)
        return false;

    // Стартовая точка должна быть цвета границы
    size_t startIdx = (static_cast<size_t>(startY) * width + startX) * 4;
    if (!sameColor(pixels, startIdx, borderR, borderG, borderB))
        return false;

    int cx = startX;
    int cy = startY;

    // Первый раз — начинаем смотреть вниз (dir = 2)
    int scanStart = 2;

    bool firstStep = true;
    const int maxIter = width * height * 8;
    int iter = 0;

    while (true) {
        if (++iter > maxIter) break;

        outBoundary.emplace_back(cx, cy);

        // Ищем следующую точку по часовой стрелке от scanStart
        int foundDir = -1;
        int nx = cx, ny = cy;
        for (int k = 0; k < 8; ++k) {
            int dir = (scanStart + k) % 8;
            int px = cx + DX[dir];
            int py = cy + DY[dir];
            if (px < 0 || px >= width || py < 0 || py >= height) continue;

            size_t i = (static_cast<size_t>(py) * width + px) * 4;
            if (sameColor(pixels, i, borderR, borderG, borderB)) {
                foundDir = dir;
                nx = px;
                ny = py;
                break;
            }
        }

        if (foundDir < 0) break;

        // Поворот на 90 градусов по часовой стрелке от направления прихода
        scanStart = (foundDir - 2 + 8) % 8;

        cx = nx;
        cy = ny;

        if (!firstStep && cx == startX && cy == startY)
            break;
        firstStep = false;
    }

    return !outBoundary.empty();
}