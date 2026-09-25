#ifndef __ALGORITHMS_IMD_MAR_VIK__
#define __ALGORITHMS_IMD_MAR_VIK__

#include <vector>

void bresenhamLine(
    std::vector<uint8_t>& pixels,  // (1) буфер пикселей, куда рисуем
    int width,                     // (2) ширина буфера в пикселях
    int height,                    // (3) высота буфера в пикселях
    int x0, int y0,                // (4) начало отрезка
    int x1, int y1,                // (5) конец отрезка
    uint8_t r, uint8_t g, uint8_t b // (6) цвет кисти
);


#endif // !__ALGORITHMS_IMD_MAR_VIK__