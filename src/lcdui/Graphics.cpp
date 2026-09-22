#include "Graphics.h"

Graphics::Graphics()
{
    this->currentColor = G2D_RGBA(0, 0, 0, 255);
    this->font = nullptr;
}

Graphics::~Graphics()
{
}

void Graphics::drawString(const std::string& s, int x, int y, int anchor)
{
    if (!font || !font->getIntraFont()) return;
    int width = font->stringWidth(s);
    int height = font->getHeight();

    x = getAnchorX(x, width, anchor);
    y = getAnchorY(y, height, anchor);

    intraFont* ifont = font->getIntraFont();
    intraFontSetStyle(ifont, font->getScale(), currentColor, 0, 0.0f, INTRAFONT_ALIGN_LEFT);
    intraFontPrint(ifont, x, y + height, s.c_str());
}

void Graphics::setColor(int r, int g, int b)
{
    currentColor = G2D_RGBA(r, g, b, 255);
}

void Graphics::setFont(std::shared_ptr<Font> font)
{
    this->font = font;
}

std::shared_ptr<Font> Graphics::getFont() const
{
    return font;
}

void Graphics::setClip(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0 || (x <= 0 && y <= 0 && w >= 480 && h >= 272)) {
        g2dResetScissor();
    } else {
        int cx = x < 0 ? 0 : x;
        int cy = y < 0 ? 0 : y;
        int cw = (x + w > 480) ? (480 - cx) : (x + w - cx);
        int ch = (y + h > 272) ? (272 - cy) : (y + h - cy);
        if (cw > 0 && ch > 0) {
            g2dSetScissor(cx, cy, cw, ch);
        } else {
            g2dSetScissor(0, 0, 0, 0);
        }
    }
}

void Graphics::drawChar(char c, int x, int y, int anchor)
{
    drawString(std::string(1, c), x, y, anchor);
}

void Graphics::fillRect(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) return;
    g2dBeginRects(NULL);
    g2dSetColor(currentColor);
    g2dSetCoordXY(x, y);
    g2dSetScaleWH(w, h);
    g2dAdd();
    g2dEnd();
}

void Graphics::drawArc(int x, int y, int width, int height, int startAngle, int arcAngle)
{
    int rx = width / 2;
    int ry = height / 2;
    int cx = x + rx;
    int cy = y + ry;
    if (rx <= 0 && ry <= 0) return;

    g2dBeginLines(G2D_STRIP);
    g2dSetColor(currentColor);

    int step = 10;
    for (int angle = startAngle; angle <= startAngle + arcAngle; angle += step) {
        int a = (angle > startAngle + arcAngle) ? (startAngle + arcAngle) : angle;
        float rad = a * PI_CONV;
        float px = cx + rx * cos(rad);
        float py = cy - ry * sin(rad);
        g2dSetCoordXY(px, py);
        g2dAdd();
    }
    g2dEnd();
}

void Graphics::fillArc(int x, int y, int w, int h, int startAngle, int arcAngle)
{
    (void)startAngle; (void)arcAngle;
    int rx = w / 2;
    int ry = h / 2;
    int cx = x + rx;
    int cy = y + ry;
    if (rx <= 0 || ry <= 0) return;

    g2dBeginLines((g2dLine_Mode)0);
    g2dSetColor(currentColor);

    for (int dy = -ry; dy <= ry; dy++) {
        float dx_max = rx * sqrt(1.0f - (float)(dy * dy) / (float)(ry * ry));
        int x1 = cx - (int)dx_max;
        int x2 = cx + (int)dx_max;
        int py = cy + dy;
        g2dSetCoordXY(x1, py); g2dAdd();
        g2dSetCoordXY(x2, py); g2dAdd();
    }
    g2dEnd();
}

void Graphics::drawLine(int x1, int y1, int x2, int y2)
{
    g2dBeginLines((g2dLine_Mode)0);
    g2dSetColor(currentColor);
    g2dSetCoordXY(x1, y1); g2dAdd();
    g2dSetCoordXY(x2, y2); g2dAdd();
    g2dEnd();
}

void Graphics::drawImage(Image* const image, int x, int y, int anchor)
{
    if (!image || !image->getG2DImage()) return;
    int w = image->getWidth();
    int h = image->getHeight();
    x = getAnchorX(x, w, anchor);
    y = getAnchorY(y, h, anchor);

    g2dBeginRects(image->getG2DImage());
    g2dSetColor(WHITE);
    g2dSetCoordXY(x, y);
    g2dSetScaleWH(w, h);
    g2dAdd();
    g2dEnd();
}

int Graphics::getAnchorX(int x, int size, int anchor)
{
    if ((anchor & LEFT) != 0) {
        return x;
    }
    if ((anchor & RIGHT) != 0) {
        return x - size;
    }
    if ((anchor & HCENTER) != 0) {
        return x - size / 2;
    }
    return x;
}

int Graphics::getAnchorY(int y, int size, int anchor)
{
    if ((anchor & TOP) != 0) {
        return y;
    }
    if ((anchor & BOTTOM) != 0) {
        return y - size;
    }
    if ((anchor & VCENTER) != 0) {
        return y - size / 2;
    }
    return y;
}
