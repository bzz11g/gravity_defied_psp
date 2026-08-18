#pragma once
#include <memory>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <tuple>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Image.h"
#include "Font.h"

constexpr auto PI_CONV = 3.1415926 / 180.0;

class Image;

class Graphics {
private:
    SDL_Renderer* renderer;
    std::shared_ptr<Font> font;
    SDL_Color currentColor;

    struct TextKey {
        std::string text;
        uint32_t color;
        TTF_Font* font;
        bool operator<(const TextKey& o) const {
            if (text != o.text) return text < o.text;
            if (color != o.color) return color < o.color;
            return font < o.font;
        }
    };
    std::map<TextKey, SDL_Texture*> textTextureCache;
    std::deque<TextKey> textCacheOrder;
    static constexpr size_t TEXT_CACHE_MAX = 48;

    void clearTextCache();
    SDL_Texture* getCachedTextTexture(const std::string& s, SDL_Color color, TTF_Font* font);

    // void _ellipse(int cx, int cy, int xradius, int yradius);
    void _putpixel(int x, int y);

public:
    enum Anchors {
        HCENTER = 1,
        VCENTER = 2,
        LEFT = 4,
        RIGHT = 8,
        TOP = 16,
        BOTTOM = 32,
        BASELINE = 64
    };
    Graphics(SDL_Renderer* renderer);
    void drawString(const std::string& s, int x, int y, int anchor);
    void setColor(int r, int g, int b);
    void setFont(std::shared_ptr<Font> font);
    std::shared_ptr<Font> getFont() const;
    void setClip(int x, int y, int w, int h);
    void drawChar(char c, int x, int y, int anchor);
    void fillRect(int x, int y, int w, int h);
    void fillArc(int x, int y, int w, int h, int startAngle, int arcAngle);
    void drawArc(int x, int y, int w, int h, int startAngle, int arcAngle);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawImage(Image* const image, int x, int y, int anchor);
    void drawImageRegion(Image* const image, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int anchor);
    static int getAnchorX(int x, int size, int anchor);
    static int getAnchorY(int y, int size, int anchor);
};