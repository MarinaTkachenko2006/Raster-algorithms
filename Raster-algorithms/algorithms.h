#ifndef __ALGORITHMS_IMD_MAR_VIK__
#define __ALGORITHMS_IMD_MAR_VIK__

#include <vector>
#include "images.h"

// проверка совпадения цветов
static inline bool sameColor(const std::vector<uint8_t>& p, size_t i,
    uint8_t r, uint8_t g, uint8_t b);

// смена цвета пикселя
static inline void setPixel(std::vector<uint8_t>& p, size_t i,
    uint8_t r, uint8_t g, uint8_t b);


void bresenhamLine(
    std::vector<uint8_t>& pixels,  // буфер пикселей, куда рисуем
    int width,                     // ширина буфера в пикселях
    int height,                    // высота буфера в пикселях
    int x0, int y0,                // начало отрезка
    int x1, int y1,                // конец отрезка
    uint8_t r, uint8_t g, uint8_t b // цвет кисти
);


// все алгоритмы заливки и выделения границы используются следующим образом: рисуем границы и кликаем по любому пикселю из области
// которую хотим залить (или границу которой выделить)
void floodFillSeries(std::vector<uint8_t>& pixels,
    int width, int height,
    int x, int y,
    uint8_t tr, uint8_t tg, uint8_t tb,  // целевой цвет (то, что закрашиваем)
    uint8_t fr, uint8_t fg, uint8_t fb);  // цвет заливки


void floodFillTextureSeries(std::vector<uint8_t>& pixels,
    int width, int height,
    int x, int y,
    uint8_t tr, uint8_t tg, uint8_t tb,   // целевой цвет холста
    int anchorX, int anchorY,             // точка клика
    const uint8_t* texData,
    int texWidth, int texHeight,
    std::vector<uint8_t>& visited);       // уже посещенные пиксели, так как текстура может содержать разные цвета и проверка на цвет здесь не работает



// Возвращает true, если граница найдена, и заполняет outBoundary
// точками в порядке обхода (по часовой стрелке).
// Так как этот алгоритм находит только внутренние пиксели границы, 
// брать максимально контрастные цвета и хорошо вглядываться при тестировании - линия рисуется очень тонкая
bool traceBoundary(const std::vector<uint8_t>& pixels,
    int width, int height,
    int startX, int startY,
    uint8_t borderR, uint8_t borderG, uint8_t borderB,
    std::vector<std::pair<int, int>>& outBoundary);

// Вспомогательная: найти первую граничную точку, идя влево (или вправо)
// от стартового клика. Возвращает false, если граница не найдена.
bool findBoundaryStart(const std::vector<uint8_t>& pixels,
    int width, int height,
    int clickX, int clickY,
    int& outX, int& outY,
    uint8_t& borderR, uint8_t& borderG, uint8_t& borderB);

#endif // !__ALGORITHMS_IMD_MAR_VIK__