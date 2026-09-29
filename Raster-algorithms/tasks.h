#ifndef __TASKS_IMD_MAR_VIK__
#define __TASKS_IMD_MAR_VIK__

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <array>
#include "dialog_windows.h"
#include "images.h"

// Класс единого контекста, который получает любая задача
class AppContext {
public:
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
};

// Интерфейс класса задачи
class TaskInterface {
public:
    virtual ~TaskInterface() = default;

    virtual void prepare(const AppContext& ctx) = 0;

    virtual void draw(const AppContext& ctx) {}
};

class Task1 : public TaskInterface {
private:
    FileDialogState dlg; // Диалоговое окно для выбора загрузки файла
    const SDL_DialogFileFilter filters[2] = {
        { "Images", "png;jpg;jpeg;bmp;tga;gif;psd;hdr;pic;pnm" },
        { "All files", "*" } };

    ImageRGB image; // Изображение, загруженное в RAM
    SDL_Texture* texImage = nullptr; // Текстура загруженного изображения

    // Холст 
    static constexpr int CANVAS_W = 800;
    static constexpr int CANVAS_H = 600;

    std::vector<uint8_t> canvasPixels;   // RGBA, размер CANVAS_W*CANVAS_H*4
    SDL_Texture* canvasTex = nullptr;    // текстура холста
    bool canvasDirty = true;             // нужно ли обновлять текстуру из буфера

    // Текущий цвет кисти 
    int brushR = 0, brushG = 0, brushB = 0;

    // толщина кисти
    int brushThickness = 1;

    // Состояние рисования между кадрами
    bool wasDrawing = false;
    ImVec2 lastDrawPos;  // предыдущая точка в координатах холста

    // варианты взаимодействия с холстом: рисовать линии, залить область цветом
    enum class CanvasMode { DrawLine, Fill, FillTexture, TraceBoundary };
    CanvasMode canvasMode = CanvasMode::DrawLine;

    // рисование толстой линии
    void drawThickLine(std::vector<uint8_t>& pixels,
        int width, int height,
        int x0, int y0, int x1, int y1,
        int thickness,
        uint8_t r, uint8_t g, uint8_t b);

    // заполнение цветом
    void fillWithColor(std::vector<uint8_t>& pixels,
        int width, int height,
        int x, int y,
        uint8_t fillR, uint8_t fillG, uint8_t fillB);

    // заполнение текстурой
    void fillWithTexture(std::vector<uint8_t>& pixels,
        int width, int height,
        int x, int y,
        const uint8_t* texData,
        int texWidth, int texHeight);

public:

    ~Task1() noexcept override;

    void prepare(const AppContext& ctx) override;
    void draw(const AppContext& ctx) override;

};

class Task2 : public TaskInterface {
public:

    ~Task2() noexcept override;

    void prepare(const AppContext& ctx) override;
    void draw(const AppContext& ctx) override;

private:
    // Холст
    static constexpr int CANVAS_W = 800;
    static constexpr int CANVAS_H = 600;

    std::vector<uint8_t> canvasPixels;   // RGBA, размер CANVAS_W*CANVAS_H*4
    SDL_Texture* canvasTex = nullptr;    // текстура холста
    bool canvasDirty = true;             // нужно ли перерисовать буфер и обновить текстуру

    // Алгоритм рисования отрезка
    enum class LineAlgorithm { Bresenham, Wu };
    LineAlgorithm algorithm = LineAlgorithm::Bresenham;

    // Отрезок хранится вместе с цветом и алгоритмом, которым его рисовали
    struct Segment {
        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        uint8_t r = 0, g = 0, b = 0;
        int thickness = 1;
        LineAlgorithm algo = LineAlgorithm::Bresenham;
    };
    std::vector<Segment> segments;

    // Отрезок, который сейчас тянут мышью (превью)
    bool isDragging = false;
    Segment preview;

    // Цвет и толщина кисти
    float lineColor[3] = { 0.0f, 0.0f, 0.0f };
    int lineThickness = 1;

    // Концы отрезка для ввода координат вручную
    int manualCoords[4] = { 100, 100, 700, 450 };

    Segment makeSegment(int x0, int y0, int x1, int y1) const;
    void rasterizeSegment(const Segment& s);
    void redrawCanvas();
};

class Task3 : public TaskInterface {
public:
    ~Task3() noexcept override;
    void prepare(const AppContext& ctx) override;
    void draw(const AppContext& ctx) override;
private:
    static constexpr int CANVAS_WIDTH = 800, CANVAS_HEIGHT = 600;


    std::vector<uint8_t> canvas_pixels;  // RGBA, the size is CANVAS_W*CANVAS_H*4
    SDL_Texture* canvas_texture = nullptr;
    bool is_canvas_dirty = true;

    struct vertex {
        float x = 0, y = 0;
        float r = 1.0f, g = 1.0f, b = 1.0f;
    };
    vertex vertices[3];

    // For moving
    int dragging_vertex = -1; // -1 — not take
    float vertex_hit_radius = 8; // Hot air intake radius
    float vertex_draw_radius = 6;

    void reset_vertices();
    // Calculating the area of a random triangle ABC
    float area_triangle(float ax, float ay,
        float bx, float by,
        float cx, float cy);

    void rasterize_triangle(std::vector<uint8_t>& pixels, int width, int height);
    int  hit_test_vertex(float cursor_x, float cursor_y) const;

    void draw_vertex_handles(ImDrawList* draw_list,
        ImVec2 canvas_draw_position,
        float scale) const;
};


#endif // !__TASKS_IMD_MAR_VIK__
