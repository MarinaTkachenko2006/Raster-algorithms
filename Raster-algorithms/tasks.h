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

    // --- Холст ---
    static constexpr int CANVAS_W = 800;
    static constexpr int CANVAS_H = 600;

    std::vector<uint8_t> canvasPixels;   // RGBA, размер CANVAS_W*CANVAS_H*4
    SDL_Texture* canvasTex = nullptr;    // текстура холста
    bool canvasDirty = true;             // нужно ли обновлять текстуру из буфера

    // --- Текущий цвет кисти ---
    int brushR = 0, brushG = 0, brushB = 0;

    // толщина кисти
    int brushThickness = 1;

    // --- Состояние рисования между кадрами ---
    bool wasDrawing = false;
    ImVec2 lastDrawPos;                  // предыдущая точка в координатах холста

    // варианты взаимодействия с холстом: рисовать линии, залить область цветом
    enum class CanvasMode { DrawLine, Fill, FillTexture };
    CanvasMode canvasMode = CanvasMode::DrawLine;

    // рисование толстой линии
    void drawThickLine(std::vector<uint8_t>& pixels,
        int width, int height,
        int x0, int y0, int x1, int y1,
        int thickness,
        uint8_t r, uint8_t g, uint8_t b);

    void fillWithColor(std::vector<uint8_t>& pixels,
        int width, int height,
        int x, int y,
        uint8_t fillR, uint8_t fillG, uint8_t fillB);

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
};

class Task3 : public TaskInterface {
public:
    ~Task3() noexcept override;
    void prepare(const AppContext& ctx) override;
};


#endif // !__TASKS_IMD_MAR_VIK__
