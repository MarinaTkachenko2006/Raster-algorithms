#include "images.h"
#include "stb_image.h"
#include <iostream>

bool ImageRGB::empty() const noexcept { return width == 0 || height == 0; }

uint8_t* ImageRGB::at(int x, int y) { return &data[(static_cast<size_t>(y) * width + x) * 3]; }
const uint8_t* ImageRGB::at(int x, int y) const { return &data[(static_cast<size_t>(y) * width + x) * 3]; }

bool loadImageRGB(const std::string& path, ImageRGB& out)
{
    int w = 0, h = 0, srcChannels = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &w, &h, &srcChannels, 3);

    if (!pixels)
    {
        std::cerr << "stbi_load failed for '" << path << "': " << stbi_failure_reason() << std::endl;
        return false;
    }

    out.width = w;
    out.height = h;
    out.channels = srcChannels;
    out.data.assign(pixels, pixels + static_cast<size_t>(w) * h * 3); // Копирование массива указателей pixels в массив out.data

    stbi_image_free(pixels); // Освобождение буфера, который stbi_load выделил внутри себя через malloc

    return true;
}

SDL_Texture* makeTextureRGB(SDL_Renderer* r, const ImageRGB& img)
{
    if (img.empty()) return nullptr;
    SDL_Texture* t = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, img.width, img.height);
    if (!t) return nullptr;
    SDL_UpdateTexture(t, nullptr, img.data.data(), img.width * 3);
    return t;
}
