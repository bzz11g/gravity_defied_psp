#pragma once

#include <string>
#include "psp/intra/intraFont.h"

class Font {
public:
    enum FontSize {
        SIZE_SMALL = 8,
        SIZE_MEDIUM = 0,
        SIZE_LARGE = 16
    };

    enum FontStyle {
        STYLE_PLAIN = 0,
        STYLE_BOLD = 1,
        STYLE_ITALIC = 2
    };

    enum FontFace {
        FACE_SYSTEM = 0
    };

    Font(FontStyle style, FontSize pointSize);
    ~Font();

    int getBaselinePosition() const;
    int getHeight() const;
    intraFont* getIntraFont() const;
    float getScale() const;
    int charWidth(char c);
    int stringWidth(const std::string& s);
    int substringWidth(const std::string& string, int offset, int len);

private:
    intraFont* font = nullptr;
    float scale = 1.0f;
    int height = 14;

    static float getScaleForSize(FontSize size);
};
