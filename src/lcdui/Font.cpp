#include "Font.h"
#include <iostream>

Font::Font(FontStyle style, FontSize pointSize)
{
    (void)style;
    this->scale = getScaleForSize(pointSize);
    this->font = intraFontLoadTTF("assets/FontSansSerif.ttf", INTRAFONT_CACHE_MED, 14.0f * this->scale);
    if (!this->font) {
        this->font = intraFontLoad("flash0:/font/ltn0.pgf", INTRAFONT_CACHE_MED);
    }
    if (this->font) {
        this->height = intraFontTextHeight(this->font);
        if (this->height <= 0) {
            this->height = static_cast<int>(14.0f * this->scale);
        }
    } else {
        this->height = static_cast<int>(14.0f * this->scale);
    }
}

Font::~Font()
{
    if (this->font) {
        intraFontUnload(this->font);
        this->font = nullptr;
    }
}

int Font::getBaselinePosition() const
{
    return this->height;
}

int Font::getHeight() const
{
    return this->height;
}

intraFont* Font::getIntraFont() const
{
    return this->font;
}

float Font::getScale() const
{
    return this->scale;
}

int Font::charWidth(char c)
{
    return stringWidth(std::string(1, c));
}

int Font::stringWidth(const std::string& s)
{
    if (!font) return static_cast<int>(s.length() * 8 * scale);
    return static_cast<int>(intraFontMeasureText(font, s.c_str()));
}

int Font::substringWidth(const std::string& string, int offset, int len)
{
    return stringWidth(string.substr(offset, len));
}

float Font::getScaleForSize(FontSize size)
{
    switch (size) {
    case SIZE_LARGE:
        return 1.2f;
    case SIZE_MEDIUM:
        return 0.9f;
    case SIZE_SMALL:
        return 0.7f;
    default:
        return 0.9f;
    }
}
