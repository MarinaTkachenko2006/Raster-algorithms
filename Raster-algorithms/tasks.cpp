#include "algorithms.h"
#include "tasks.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

#include "stb_image_write.h"


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

void Task1::fillWithColor(std::vector<uint8_t>& pixels, int width, int height, int x, int y, uint8_t fillR, uint8_t fillG, uint8_t fillB)
{
	// Проверка координат до чтения пикселя
	if (x < 0 || x >= width || y < 0 || y >= height) return;

	// Считываем цвет стартовой точки — это и есть «целевой цвет»,
	// который мы будем заменять
	size_t i = (static_cast<size_t>(y) * width + x) * 4;
	uint8_t tr = pixels[i + 0];
	uint8_t tg = pixels[i + 1];
	uint8_t tb = pixels[i + 2];

	// Если стартовый цвет уже совпадает с цветом заливки — делать нечего
	if (tr == fillR && tg == fillG && tb == fillB) return;

	floodFillSeries(pixels, width, height,
		x, y,
		tr, tg, tb,
		fillR, fillG, fillB);
}

void Task1::fillWithTexture(std::vector<uint8_t>& pixels, int width, int height, int x, int y, const uint8_t* texData, int texWidth, int texHeight)
{
	if (x < 0 || x >= width || y < 0 || y >= height) return;
	if (!texData || texWidth <= 0 || texHeight <= 0) return;

	size_t i = (static_cast<size_t>(y) * width + x) * 4;
	uint8_t tr = pixels[i + 0];
	uint8_t tg = pixels[i + 1];
	uint8_t tb = pixels[i + 2];

	std::vector<uint8_t> visited(width * height, 0);

	floodFillTextureSeries(pixels, width, height,
		x, y,
		tr, tg, tb,
		x, y,                              // anchor = точка клика
		texData, texWidth, texHeight, visited);
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

void Task1::draw(const AppContext& ctx) {

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

	if (canvasMode == CanvasMode::DrawLine) {
		// --- Режим рисования линии (как раньше) ---
		if (active && mouseDown) {
			ImVec2 m = ImGui::GetIO().MousePos;
			ImVec2 c = toCanvasCoords(m);

			uint8_t r = (uint8_t)std::clamp(brushR, 0, 255);
			uint8_t g = (uint8_t)std::clamp(brushG, 0, 255);
			uint8_t b = (uint8_t)std::clamp(brushB, 0, 255);

			if (!wasDrawing) {
				drawThickLine(canvasPixels, CANVAS_W, CANVAS_H,
					(int)c.x, (int)c.y, (int)c.x, (int)c.y, brushThickness, r, g, b);
				canvasDirty = true;
			}
			else {
				drawThickLine(canvasPixels, CANVAS_W, CANVAS_H,
					(int)lastDrawPos.x, (int)lastDrawPos.y,
					(int)c.x, (int)c.y, brushThickness, r, g, b);
				canvasDirty = true;
			}

			lastDrawPos = c;
			wasDrawing = true;
		}
		else {
			wasDrawing = false;
		}
	}
	else if (canvasMode == CanvasMode::FillTexture) {
		wasDrawing = false;

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			if (!image.empty()) {
				ImVec2 m = ImGui::GetIO().MousePos;
				ImVec2 c = toCanvasCoords(m);

				fillWithTexture(canvasPixels, CANVAS_W, CANVAS_H,
					(int)c.x, (int)c.y,
					image.data.data(),
					image.width, image.height);
				canvasDirty = true;
			}
		}
	}
	else if (canvasMode == CanvasMode::TraceBoundary) {
		wasDrawing = false;

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			ImVec2 m = ImGui::GetIO().MousePos;
			ImVec2 c = toCanvasCoords(m);

			// 1. Находим первую граничную точку и её цвет
			int sx, sy;
			uint8_t bR, bG, bB;
			if (findBoundaryStart(canvasPixels, CANVAS_W, CANVAS_H,
				(int)c.x, (int)c.y,
				sx, sy,
				bR, bG, bB))
			{
				// 2. Обходим границу, ища пиксели цвета (bR, bG, bB)
				std::vector<std::pair<int, int>> boundary;
				if (traceBoundary(canvasPixels, CANVAS_W, CANVAS_H,
					sx, sy,
					bR, bG, bB,
					boundary))
				{
					// 3. Выделяем найденные пиксели цветом кисти
					uint8_t hr = (uint8_t)std::clamp(brushR, 0, 255);
					uint8_t hg = (uint8_t)std::clamp(brushG, 0, 255);
					uint8_t hb = (uint8_t)std::clamp(brushB, 0, 255);

					for (auto& [px, py] : boundary) {
						if (px < 0 || px >= CANVAS_W || py < 0 || py >= CANVAS_H)
							continue;
						size_t di = (static_cast<size_t>(py) * CANVAS_W + px) * 4;
						canvasPixels[di + 0] = hr;
						canvasPixels[di + 1] = hg;
						canvasPixels[di + 2] = hb;
						canvasPixels[di + 3] = 255;
					}
					canvasDirty = true;
				}
			}
		}
	}
	else { // CanvasMode::Fill
		// --- Режим заливки: реагируем на одиночный клик ---
		wasDrawing = false; // сбрасываем, чтобы при переключении режима не было «хвоста» штриха

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			ImVec2 m = ImGui::GetIO().MousePos;
			ImVec2 c = toCanvasCoords(m);

			uint8_t r = (uint8_t)std::clamp(brushR, 0, 255);
			uint8_t g = (uint8_t)std::clamp(brushG, 0, 255);
			uint8_t b = (uint8_t)std::clamp(brushB, 0, 255);

			fillWithColor(canvasPixels, CANVAS_W, CANVAS_H,
				(int)c.x, (int)c.y,
				r, g, b);
			canvasDirty = true;
		}
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

	// выбор режима работы
	ImGui::Text("Canvas mode");

	ImGui::SetNextItemWidth(-FLT_MIN);
	const char* modeNames[] = { "Draw line", "Fill", "Fill with texture", "Trace boundary" };
	int modeIndex = static_cast<int>(canvasMode);
	if (ImGui::Combo("##mode", &modeIndex, modeNames, IM_ARRAYSIZE(modeNames))) {
		canvasMode = static_cast<CanvasMode>(modeIndex);
	}

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

Task2::~Task2() noexcept {
	if (canvasTex) SDL_DestroyTexture(canvasTex);
}

void Task2::prepare(const AppContext& ctx) {
	canvasPixels.assign(CANVAS_W * CANVAS_H * 4, 255); // весь белый
	canvasTex = SDL_CreateTexture(ctx.renderer, SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING,
		CANVAS_W, CANVAS_H);
	segments.clear();
	isDragging = false;
	canvasDirty = true;
}

// Отрезок с текущими цветом кисти и алгоритмом
Task2::Segment Task2::makeSegment(int x0, int y0, int x1, int y1) const {
	Segment s;
	s.x0 = x0; s.y0 = y0;
	s.x1 = x1; s.y1 = y1;
	s.r = (uint8_t)std::clamp(lineColor[0] * 255.0f + 0.5f, 0.0f, 255.0f);
	s.g = (uint8_t)std::clamp(lineColor[1] * 255.0f + 0.5f, 0.0f, 255.0f);
	s.b = (uint8_t)std::clamp(lineColor[2] * 255.0f + 0.5f, 0.0f, 255.0f);
	s.thickness = lineThickness;
	s.algo = algorithm;
	return s;
}

void Task2::rasterizeSegment(const Segment& s) {
	if (s.algo == LineAlgorithm::Bresenham)
		bresenhamThickLine(canvasPixels, CANVAS_W, CANVAS_H, s.x0, s.y0, s.x1, s.y1, s.thickness, s.r, s.g, s.b);
	else
		wuThickLine(canvasPixels, CANVAS_W, CANVAS_H, s.x0, s.y0, s.x1, s.y1, s.thickness, s.r, s.g, s.b);
}

// Буфер перерисовывается с нуля: Ву смешивает цвет с фоном,
// поэтому превью нельзя просто "стереть" — проще нарисовать всё заново
void Task2::redrawCanvas() {
	std::fill(canvasPixels.begin(), canvasPixels.end(), 255);
	for (const Segment& s : segments)
		rasterizeSegment(s);
	if (isDragging)
		rasterizeSegment(preview);
}


void Task2::draw(const AppContext& ctx) {
	const float rightPanelWidth = 280.0f;
	const float spacing = 4.0f;

	ImVec2 avail = ImGui::GetContentRegionAvail();
	float leftWidth = avail.x - rightPanelWidth - spacing;

	// ------------------- Левая часть: холст -------------------
	ImGui::BeginChild("Task2_Canvas", ImVec2(leftWidth, avail.y),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 canvasAreaPos = ImGui::GetCursorScreenPos();
	ImVec2 canvasAreaSize = ImGui::GetContentRegionAvail();

	float scale = std::min(canvasAreaSize.x / (float)CANVAS_W,
		canvasAreaSize.y / (float)CANVAS_H);
	// Если холст помещается целиком — берём целый масштаб: каждый пиксель холста
	// становится ровным квадратом, и разница Брезенхем/Ву видна без искажений
	bool pixelExact = scale >= 1.0f;
	if (pixelExact) scale = std::floor(scale);
	ImVec2 canvasDrawSize(CANVAS_W * scale, CANVAS_H * scale);
	ImVec2 canvasDrawPos(canvasAreaPos.x + (canvasAreaSize.x - canvasDrawSize.x) * 0.5f,
		canvasAreaPos.y + (canvasAreaSize.y - canvasDrawSize.y) * 0.5f);

	// Интерактивная зона поверх холста
	ImGui::SetCursorScreenPos(canvasDrawPos);
	ImGui::InvisibleButton("##task2_canvas", canvasDrawSize);

	// Экранные координаты -> координаты холста (с ограничением краями холста)
	ImVec2 m = ImGui::GetIO().MousePos;
	int cx = std::clamp((int)((m.x - canvasDrawPos.x) / scale), 0, CANVAS_W - 1);
	int cy = std::clamp((int)((m.y - canvasDrawPos.y) / scale), 0, CANVAS_H - 1);

	// Отрезок задаётся перетаскиванием: нажали — начало, отпустили — конец
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
		isDragging = true;
		preview = makeSegment(cx, cy, cx, cy);
		canvasDirty = true;
	}
	if (isDragging) {
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			if (preview.x1 != cx || preview.y1 != cy) {
				preview.x1 = cx;
				preview.y1 = cy;
				canvasDirty = true;
			}
		}
		else {
			segments.push_back(preview);
			isDragging = false;
			canvasDirty = true;
		}
	}

	// Обновляем буфер и текстуру только когда что-то изменилось
	if (canvasDirty && canvasTex) {
		redrawCanvas();
		SDL_UpdateTexture(canvasTex, nullptr, canvasPixels.data(), CANVAS_W * 4);
		canvasDirty = false;
	}

	// При целом масштабе рисуем без фильтрации текстуры (nearest): линейная фильтрация
	// размыла бы отрезок Брезенхема и он выглядел бы сглаженным, как у Ву.
	// При уменьшении nearest терял бы тонкие линии, поэтому там остаётся linear.
	const ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	if (pixelExact && pio.DrawCallback_SetSamplerNearest) dl->AddCallback(pio.DrawCallback_SetSamplerNearest, nullptr);
	dl->AddImage((ImTextureID)(intptr_t)canvasTex,
		canvasDrawPos,
		ImVec2(canvasDrawPos.x + canvasDrawSize.x, canvasDrawPos.y + canvasDrawSize.y));
	if (pixelExact && pio.DrawCallback_SetSamplerLinear) dl->AddCallback(pio.DrawCallback_SetSamplerLinear, nullptr);

	ImGui::EndChild();

	// ------------------- Правая часть: управление -------------------
	ImGui::SameLine();
	ImGui::BeginChild("Task2_Controls", ImVec2(rightPanelWidth, avail.y), true);

	ImGui::Text("Line algorithm");
	ImGui::Separator();

	int algoIndex = static_cast<int>(algorithm);
	ImGui::RadioButton("Bresenham (integer)", &algoIndex, static_cast<int>(LineAlgorithm::Bresenham));
	ImGui::RadioButton("Wu (anti-aliased)", &algoIndex, static_cast<int>(LineAlgorithm::Wu));
	algorithm = static_cast<LineAlgorithm>(algoIndex);

	ImGui::Separator();

	ImGui::Text("Line color");
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::ColorEdit3("##line_color", lineColor);

	ImGui::Text("Line thickness");
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::SliderInt("##thickness", &lineThickness, 1, 30, "%d px");
	lineThickness = std::clamp(lineThickness, 1, 30);

	ImGui::Separator();

	// Ввод отрезка по координатам
	ImGui::Text("Segment by coordinates");
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputInt4("##coords", manualCoords);
	manualCoords[0] = std::clamp(manualCoords[0], 0, CANVAS_W - 1);
	manualCoords[1] = std::clamp(manualCoords[1], 0, CANVAS_H - 1);
	manualCoords[2] = std::clamp(manualCoords[2], 0, CANVAS_W - 1);
	manualCoords[3] = std::clamp(manualCoords[3], 0, CANVAS_H - 1);
	ImGui::TextDisabled("x0, y0, x1, y1");
	if (ImGui::Button("Draw segment", ImVec2(-FLT_MIN, 0))) {
		segments.push_back(makeSegment(manualCoords[0], manualCoords[1],
			manualCoords[2], manualCoords[3]));
		canvasDirty = true;
	}

	ImGui::Separator();

	if (ImGui::Button("Undo", ImVec2(-FLT_MIN, 0)) && !segments.empty()) {
		segments.pop_back();
		canvasDirty = true;
	}
	if (ImGui::Button("Clear", ImVec2(-FLT_MIN, 0))) {
		segments.clear();
		canvasDirty = true;
	}

	ImGui::EndChild();
}
Task3::~Task3() noexcept {
	if (this->canvas_texture) SDL_DestroyTexture(this->canvas_texture);
}

void Task3::reset_vertices() {
	this->vertices[0] = { 200.0f, 480.0f, 1.0f, 0.0f, 0.0f }; // REG AND STANDART POSITIONS
	this->vertices[1] = { 600.0f, 480.0f, 0.0f, 1.0f, 0.0f }; // GREEN AND STANDART POSITIONS
	this->vertices[2] = { 400.0f, 100.0f, 0.0f, 0.0f, 1.0f }; // BLUE AND STANDART POSITIONS
}

void Task3::prepare(const AppContext& context) {
	this->canvas_pixels.assign(CANVAS_WIDTH * CANVAS_HEIGHT * 4, 255); // The white canvas
	this->canvas_texture = SDL_CreateTexture(context.renderer,
		SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
		CANVAS_WIDTH, CANVAS_HEIGHT);
	this->reset_vertices();
	this->is_canvas_dirty = true;
}

// Calculating the area of a random triangle ABC
float Task3::area_triangle(float ax, float ay,
	float bx, float by,
	float cx, float cy)
{
	return ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax)) / 2;
}

void Task3::rasterize_triangle(std::vector<uint8_t>& pixels, int width, int height)
{
	std::fill(pixels.begin(), pixels.end(), 255); // The white canvas

	// Bounding rectangle (for optimization)
	int min_x = (int)std::floor(std::min({ this->vertices[0].x, this->vertices[1].x, this->vertices[2].x }));
	int max_x = (int)std::ceil(std::max({ this->vertices[0].x, this->vertices[1].x, this->vertices[2].x }));
	int min_y = (int)std::floor(std::min({ this->vertices[0].y, this->vertices[1].y, this->vertices[2].y }));
	int max_y = (int)std::ceil(std::max({ this->vertices[0].y, this->vertices[1].y, this->vertices[2].y }));

	min_x = std::max(min_x, 0);
	min_y = std::max(min_y, 0);
	max_x = std::min(max_x, width - 1);
	max_y = std::min(max_y, height - 1);

	float area = this->area_triangle(this->vertices[0].x, this->vertices[0].y,
		this->vertices[1].x, this->vertices[1].y,
		this->vertices[2].x, this->vertices[2].y);
	if (std::fabs(area) < 1e-6f) return;

	// The pixel traversal
	for (int y = min_y; y <= max_y; ++y) {
		for (int x = min_x; x <= max_x; ++x) {
			// Pixel (x, y) occupies the square from (x, y) to (x+1, y+1). Its center is (x + 0.5, y + 0.5).
			float pixel_x(x + 0.5f), pixel_y(y + 0.5f);

			// The barycentric coordinates
			float a = this->area_triangle(this->vertices[1].x, this->vertices[1].y,
				this->vertices[2].x, this->vertices[2].y,
				pixel_x, pixel_y) / area;
			float b = this->area_triangle(this->vertices[2].x, this->vertices[2].y,
				this->vertices[0].x, this->vertices[0].y,
				pixel_x, pixel_y) / area;
			float c = this->area_triangle(this->vertices[0].x, this->vertices[0].y,
				this->vertices[1].x, this->vertices[1].y,
				pixel_x, pixel_y) / area;

			if (a < -1e-8f || b < -1e-8f || c < -1e-8f) continue; // The current pixel is outside the triangle

			// The color interpolation using barycentric weights
			float red = a * this->vertices[0].r + b * this->vertices[1].r + c * this->vertices[2].r;
			float green = a * this->vertices[0].g + b * this->vertices[1].g + c * this->vertices[2].g;
			float blue = a * this->vertices[0].b + b * this->vertices[1].b + c * this->vertices[2].b;

			// The buffer recording
			size_t index = (y * width + x) * 4;
			pixels[index + 0] = (uint8_t)std::clamp(red * 255.0f, 0.0f, 255.0f);
			pixels[index + 1] = (uint8_t)std::clamp(green * 255.0f, 0.0f, 255.0f);
			pixels[index + 2] = (uint8_t)std::clamp(blue * 255.0f, 0.0f, 255.0f);
			pixels[index + 3] = 255;
		}
	}
}

// Determines which triangle vertex the mouse cursor is over?
int Task3::hit_test_vertex(float cursor_x, float cursor_y) const {
	int best = -1;
	float best_distance_squared = this->vertex_hit_radius * this->vertex_hit_radius;
	for (int i = 0; i < 3; ++i) {
		float delta_x(cursor_x - this->vertices[i].x), delta_y(cursor_y - this->vertices[i].y);
		float distance_squared = delta_x * delta_x + delta_y * delta_y;
		if (distance_squared <= best_distance_squared) {
			best_distance_squared = distance_squared;
			best = i;
		}
	}
	return best;
}

void Task3::draw_vertex_handles(ImDrawList* draw_list,
	ImVec2 canvas_draw_position,
	float scale) const
{
	for (int i(0); i < 3; ++i) {
		ImVec2 pixel_position(canvas_draw_position.x + this->vertices[i].x * scale,
			canvas_draw_position.y + this->vertices[i].y * scale); // The current pixel
		ImU32 color = IM_COL32((int)(this->vertices[i].r * 255),
			(int)(this->vertices[i].g * 255),
			(int)(this->vertices[i].b * 255), 255);

		draw_list->AddCircleFilled(pixel_position, this->vertex_draw_radius, color, 24);
		draw_list->AddCircle(pixel_position, this->vertex_draw_radius,
			IM_COL32(255, 255, 255, 255), 24, 2.0f);
		draw_list->AddCircle(pixel_position, this->vertex_draw_radius + 2.0f,
			IM_COL32(0, 0, 0, 255), 24, 1.0f);

		char label[2] = { (char)('A' + i), 0 };
		draw_list->AddText(ImVec2(pixel_position.x + 8, pixel_position.y - 8),
			IM_COL32(0, 0, 0, 255), label);
	}
}

void Task3::draw(const AppContext& context) {
	const float right_panel_width = 280;

	ImVec2 available = ImGui::GetContentRegionAvail();
	float left_width = available.x - right_panel_width;

	// The left part is a canvas
	ImGui::BeginChild("Task3_Left", ImVec2(left_width, available.y), false);

	ImVec2 left_available = ImGui::GetContentRegionAvail();
	ImGui::BeginChild("Task3_CanvasSpace", ImVec2(left_available.x, left_available.y),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 canvas_area_position = ImGui::GetCursorScreenPos();
	ImVec2 canvas_area_size = ImGui::GetContentRegionAvail();
	float scale = std::min(canvas_area_size.x / CANVAS_WIDTH,
		canvas_area_size.y / CANVAS_HEIGHT);

	ImVec2 canvas_draw_size(CANVAS_WIDTH * scale, CANVAS_HEIGHT * scale); // The actual canvas size on screen
	ImVec2 canvas_offset((canvas_area_size.x - canvas_draw_size.x) / 2,
		(canvas_area_size.y - canvas_draw_size.y) / 2); // The margins to center the text within the available area
	ImVec2 canvas_draw_position(canvas_area_position.x + canvas_offset.x,
		canvas_area_position.y + canvas_offset.y);

	// Buffer redrawing - upon every change of vertices/colors
	if (this->is_canvas_dirty) {
		this->rasterize_triangle(this->canvas_pixels, CANVAS_WIDTH, CANVAS_HEIGHT);
		SDL_UpdateTexture(this->canvas_texture, nullptr, this->canvas_pixels.data(), CANVAS_WIDTH * 4);
		this->is_canvas_dirty = false;
	}

	// Draw the canvas as an image
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage((ImTextureID)(intptr_t)this->canvas_texture, canvas_draw_position,
		ImVec2(canvas_draw_position.x + canvas_draw_size.x,
			canvas_draw_position.y + canvas_draw_size.y));

	ImGui::SetCursorScreenPos(canvas_draw_position);
	ImGui::InvisibleButton("##task3_canvas", canvas_draw_size,
		ImGuiButtonFlags_MouseButtonLeft); // Interactive area for monitoring the mouse's status.

	bool hovered = ImGui::IsItemHovered();
	bool active = ImGui::IsItemActive();

	ImVec2 mouse = ImGui::GetIO().MousePos;
	ImVec2 cursor_local((mouse.x - canvas_draw_position.x) / scale,
		(mouse.y - canvas_draw_position.y) / scale); // The mouse coordinates in the local world

	// Drag&drop logic
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		this->dragging_vertex = this->hit_test_vertex(cursor_local.x, cursor_local.y);
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		this->dragging_vertex = -1;

	if (this->dragging_vertex >= 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left)) { // Constrain the vertex to the canvas
		this->vertices[this->dragging_vertex].x = std::clamp(cursor_local.x, 0.0f, (float)CANVAS_WIDTH - 1);
		this->vertices[this->dragging_vertex].y = std::clamp(cursor_local.y, 0.0f, (float)CANVAS_HEIGHT - 1);
		this->is_canvas_dirty = true;
	}

	this->draw_vertex_handles(draw_list, canvas_draw_position, scale);

	ImGui::EndChild();
	ImGui::EndChild();

	// The right part is a control panel
	ImGui::SameLine();
	ImGui::BeginChild("Task3_Controls", ImVec2(right_panel_width, available.y), true);

	for (int i(0); i < 3; ++i) {
		ImGui::PushID(i); // The unique id for the current elements

		char label[32];
		std::snprintf(label, sizeof(label), "Vertex %c", 'A' + i);
		ImGui::TextUnformatted(label);

		float color[3] = { this->vertices[i].r, this->vertices[i].g, this->vertices[i].b };
		ImGui::SetNextItemWidth(180);
		if (ImGui::ColorEdit3("##col", color,
			ImGuiColorEditFlags_Float |
			ImGuiColorEditFlags_PickerHueBar)) {
			this->vertices[i].r = color[0];
			this->vertices[i].g = color[1];
			this->vertices[i].b = color[2];
			this->is_canvas_dirty = true;
		}

		ImGui::SetNextItemWidth(90);
		if (ImGui::DragFloat("x", &this->vertices[i].x, 1.0f, 0.0f,
			(float)CANVAS_WIDTH - 1, "%.1f")) {
			this->is_canvas_dirty = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90);
		if (ImGui::DragFloat("y", &this->vertices[i].y, 1.0f, 0.0f,
			(float)CANVAS_HEIGHT - 1, "%.1f")) {
			this->is_canvas_dirty = true;
		}

		ImGui::Separator();
		ImGui::PopID();
	}

	if (ImGui::Button("Reset vertices", ImVec2(-FLT_MIN, 0))) {
		this->reset_vertices();
		this->is_canvas_dirty = true;
	}

	ImGui::EndChild();
}