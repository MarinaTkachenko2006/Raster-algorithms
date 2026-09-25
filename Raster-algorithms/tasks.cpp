#include "tasks.h"
#include <cstdint>
#include "algorithms.h"
#include <algorithm>


void Task1::drawThickLine(std::vector<uint8_t>& pixels, int width, int height, int x0, int y0, int x1, int y1, int thickness, uint8_t r, uint8_t g, uint8_t b)
{
    if (thickness <= 1) {
        bresenhamLine(pixels, width, height, x0, y0, x1, y1, r, g, b);
        return;
    }

    // радиус - толщина линии
    int half = thickness / 2;
    int half2 = half * half;


    // Сдвигаем линию по квадратной сетке — получается «толстая» линия,
    // визуально близкая к диску радиуса thickness/2
    for (int dy = -half; dy <= half; ++dy) {
        for (int dx = -half; dx <= half; ++dx) {
            if (dx * dx + dy * dy > half2) continue; // вне круга — пропускаем, чтобы перо было визуально круглым, а не квадратным
            bresenhamLine(pixels, width, height,
                x0 + dx, y0 + dy,
                x1 + dx, y1 + dy,
                r, g, b);
        }
    }
}

Task1::~Task1() noexcept {
    if (canvasTex) SDL_DestroyTexture(canvasTex);
    if (texImage)  SDL_DestroyTexture(texImage);
}

void Task1::prepare(const AppContext& ctx) {
    canvasPixels.assign(CANVAS_W * CANVAS_H * 4, 255); // весь белый
    canvasTex = SDL_CreateTexture(ctx.renderer, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        CANVAS_W, CANVAS_H);
    canvasDirty = true;
}

void Task1::draw(const AppContext& ctx){

    if (dlg.ready) { // Диалоговое окно закрылось
        if (dlg.ok && loadImageRGB(dlg.path, image)) { // Пользователь выбрал файл, и он был успешно загружен
            if (texImage) SDL_DestroyTexture(texImage);
            texImage = makeTextureRGB(ctx.renderer, image);
        }
        dlg.reset(); // Сброс диалогового окна
    }


    // Параметры компоновки
    const float rightPanelWidth = 260.0f; // ширина правой панели
    const float splitterHeight = 6.0f;   // высота "разделителя" между верхней и нижней левой областью
    const float spacing = 4.0f;

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float leftWidth = avail.x - rightPanelWidth - spacing;

    // ------------------- Левая часть -------------------
    ImGui::BeginChild("LeftColumn", ImVec2(leftWidth, avail.y), false);

    ImVec2 leftAvail = ImGui::GetContentRegionAvail();

    // Верхняя левая область: белый "холст" для рисования
    float topHeight = (leftAvail.y - splitterHeight - spacing) * 0.6f; // 60% высоты

    ImGui::BeginChild("CanvasArea", ImVec2(leftAvail.x, topHeight),
        ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 canvasAreaPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasAreaSize = ImGui::GetContentRegionAvail();

    float scale = std::min(canvasAreaSize.x / (float)CANVAS_W,
        canvasAreaSize.y / (float)CANVAS_H);
    ImVec2 canvasDrawSize(CANVAS_W * scale, CANVAS_H * scale);
    ImVec2 canvasOffset((canvasAreaSize.x - canvasDrawSize.x) * 0.5f,
        (canvasAreaSize.y - canvasDrawSize.y) * 0.5f);
    ImVec2 canvasDrawPos(canvasAreaPos.x + canvasOffset.x,
        canvasAreaPos.y + canvasOffset.y);

    // Обновляем текстуру только когда буфер менялся
    if (canvasDirty && canvasTex) {
        SDL_UpdateTexture(canvasTex, nullptr, canvasPixels.data(), CANVAS_W * 4);
        canvasDirty = false;
    }

    // Рисуем холст
    ImGui::SetCursorScreenPos(canvasDrawPos);
    ImGui::Image((ImTextureID)(intptr_t)canvasTex, canvasDrawSize);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Рисуем холст как изображение — без виджета
    dl->AddImage((ImTextureID)(intptr_t)canvasTex,
        canvasDrawPos,
        ImVec2(canvasDrawPos.x + canvasDrawSize.x,
            canvasDrawPos.y + canvasDrawSize.y));


    // Интерактивная зона поверх холста
    ImGui::SetCursorScreenPos(canvasDrawPos);
    ImGui::InvisibleButton("##canvas", canvasDrawSize);

    bool active = ImGui::IsItemActive();
    bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);

    // Экранные координаты -> координаты холста
    auto toCanvasCoords = [&](ImVec2 screenPos) -> ImVec2 {
        return ImVec2((screenPos.x - canvasDrawPos.x) / scale,
            (screenPos.y - canvasDrawPos.y) / scale);
        };

    // рисуем на холсте
    if (active && mouseDown) {
        ImVec2 m = ImGui::GetIO().MousePos;
        ImVec2 c = toCanvasCoords(m);

        uint8_t r = (uint8_t)std::clamp(brushR, 0, 255);
        uint8_t g = (uint8_t)std::clamp(brushG, 0, 255);
        uint8_t b = (uint8_t)std::clamp(brushB, 0, 255);

        if (!wasDrawing) {
            drawThickLine(canvasPixels, CANVAS_W, CANVAS_H,
                (int)c.x, (int)c.y, (int)c.x, (int)c.y,
                brushThickness, r, g, b);
            canvasDirty = true;
        }
        else {
            drawThickLine(canvasPixels, CANVAS_W, CANVAS_H,
                (int)lastDrawPos.x, (int)lastDrawPos.y,
                (int)c.x, (int)c.y,
                brushThickness, r, g, b);
            canvasDirty = true;
        }

        lastDrawPos = c;
        wasDrawing = true;
    }
    else {
        wasDrawing = false;
    }

    ImGui::EndChild();

    // Небольшой отступ-разделитель
    ImGui::Dummy(ImVec2(0, spacing));

    // Нижняя левая область: сюда потом будем вставлять загруженное изображение
    float bottomHeight = ImGui::GetContentRegionAvail().y;
    ImGui::BeginChild("ImageArea", ImVec2(leftAvail.x, bottomHeight),
        ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (!image.empty() && texImage) { // Есть ли что рисовать
        // Расчёт масштаба и размеров
        ImVec2 avail = ImGui::GetContentRegionAvail(); // Сколько свободного места внутри текущей child-области
        float scale = std::min(avail.x / image.width, avail.y / image.height); // Коэффициент вписывания с сохранением пропорций
        ImVec2 sz(image.width * scale, image.height * scale); // Реальный размер, в который картинка будет выведена с сохранением пропорций
        ImGui::Image((ImTextureID)(intptr_t)texImage, sz);
    }
    else ImGui::TextUnformatted("Load an image to see it here");

    ImGui::EndChild();

    ImGui::EndChild(); // LeftColumn

    // ------------------- Правая часть -------------------
    ImGui::SameLine();

    ImGui::BeginChild("ControlsPanel", ImVec2(rightPanelWidth, avail.y), true);

    ImGui::Text("Controls");
    ImGui::Separator();

    // --- Цвет кисти ---
    ImGui::Text("Brush color (0-255)");
    ImGui::SetNextItemWidth(70);
    ImGui::InputInt("R", &brushR);
    ImGui::SetNextItemWidth(70);
    ImGui::InputInt("G", &brushG);
    ImGui::SetNextItemWidth(70);
    ImGui::InputInt("B", &brushB);

    // Ограничим диапазон
    brushR = std::clamp(brushR, 0, 255);
    brushG = std::clamp(brushG, 0, 255);
    brushB = std::clamp(brushB, 0, 255);

    // Превью цвета
    ImGui::ColorButton("##preview",
        ImVec4(brushR / 255.0f, brushG / 255.0f, brushB / 255.0f, 1.0f),
        ImGuiColorEditFlags_NoTooltip, ImVec2(40, 40));
    
    // Толщина кисти
    ImGui::Text("Brush size");
    ImGui::SetNextItemWidth(70);
    ImGui::InputInt("px", &brushThickness);
    brushThickness = std::clamp(brushThickness, 1, 64);


    // --- Очистка холста ---
    if (ImGui::Button("Reset")) {
        std::fill(canvasPixels.begin(), canvasPixels.end(), 255);
        canvasDirty = true;
    }

    ImGui::Separator();


    // Загрузка изображения (будем использовать его как текстуру при заливке области)
    if (ImGui::Button("load image")) {
        if (!dlg.pending) {
            dlg.pending = true;
            SDL_ShowOpenFileDialog(onFileDialogResult, &dlg, ctx.window,
                filters, static_cast<int>(sizeof(filters) / sizeof(filters[0])),
                nullptr, false);
        }
    }

    ImGui::EndChild();
}

Task2::~Task2() noexcept {}
void Task2::prepare(const AppContext& ctx) {}

Task3::~Task3() noexcept { }
void Task3::prepare(const AppContext& ctx) {}
