#ifndef __IMAGES_IMD_MAR_VIK__
#define __IMAGES_IMD_MAR_VIK__

#include <vector>
#include <string>
#include <SDL3/SDL.h>

// Класс изображения в RGB-моделе
class ImageRGB {
public:

    int width = 0, height = 0, channels = 0;
    std::vector<uint8_t> data; // Вектор байтов размера width * height * 3

    uint8_t* at(int x, int y);
    const uint8_t* at(int x, int y) const;

    ImageRGB() = default;

    bool empty() const noexcept;
};

// Загрузка из файловой системы RGB-изображения
bool loadImageRGB(const std::string& path, ImageRGB& out);

// Преобразование RGB-изображения в SDL_Texture
SDL_Texture* makeTextureRGB(SDL_Renderer* r, const ImageRGB& img);

#endif // !__IMAGES_IMD_MAR_VIK__
