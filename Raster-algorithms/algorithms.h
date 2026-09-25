#ifndef __ALGORITHMS_IMD_MAR_VIK__
#define __ALGORITHMS_IMD_MAR_VIK__

#include <vector>

static inline bool sameColor(const std::vector<uint8_t>& p, size_t i,
    uint8_t r, uint8_t g, uint8_t b);
static inline void setPixel(std::vector<uint8_t>& p, size_t i,
    uint8_t r, uint8_t g, uint8_t b);


void bresenhamLine(
    std::vector<uint8_t>& pixels,  // (1) буфер пикселей, куда рисуем
    int width,                     // (2) ширина буфера в пикселях
    int height,                    // (3) высота буфера в пикселях
    int x0, int y0,                // (4) начало отрезка
    int x1, int y1,                // (5) конец отрезка
    uint8_t r, uint8_t g, uint8_t b // (6) цвет кисти
);



void floodFillSeries(std::vector<uint8_t>& pixels,
    int width, int height,
    int x, int y,
    uint8_t tr, uint8_t tg, uint8_t tb,  // целевой цвет (то, что закрашиваем)
    uint8_t fr, uint8_t fg, uint8_t fb);  // цвет заливки

#endif // !__ALGORITHMS_IMD_MAR_VIK__