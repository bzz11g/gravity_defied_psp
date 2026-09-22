#include "Graphics.h"

namespace {
const int INSIDE = 0;
const int LEFT_BIT = 1;
const int RIGHT_BIT = 2;
const int BOTTOM_BIT = 4;
const int TOP_BIT = 8;

static int computeOutCode(double x, double y, double xmin, double ymin, double xmax, double ymax) {
    int code = INSIDE;
    if (x < xmin)      code |= LEFT_BIT;
    else if (x > xmax) code |= RIGHT_BIT;
    if (y < ymin)      code |= BOTTOM_BIT;
    else if (y > ymax) code |= TOP_BIT;
    return code;
}

static bool clipLine(int& x1, int& y1, int& x2, int& y2, int xmin = -10, int ymin = -10, int xmax = 490, int ymax = 282) {
    double x0 = x1, y0 = y1;
    double x1_d = x2, y1_d = y2;

    int outcode0 = computeOutCode(x0, y0, xmin, ymin, xmax, ymax);
    int outcode1 = computeOutCode(x1_d, y1_d, xmin, ymin, xmax, ymax);
    bool accept = false;

    while (true) {
        if (!(outcode0 | outcode1)) {
            accept = true;
            break;
        } else if (outcode0 & outcode1) {
            break;
        } else {
            double x = 0, y = 0;
            int outcodeOut = outcode0 ? outcode0 : outcode1;

            if (outcodeOut & TOP_BIT) {
                x = x0 + (x1_d - x0) * (ymax - y0) / (y1_d - y0);
                y = ymax;
            } else if (outcodeOut & BOTTOM_BIT) {
                x = x0 + (x1_d - x0) * (ymin - y0) / (y1_d - y0);
                y = ymin;
            } else if (outcodeOut & RIGHT_BIT) {
                y = y0 + (y1_d - y0) * (xmax - x0) / (x1_d - x0);
                x = xmax;
            } else if (outcodeOut & LEFT_BIT) {
                y = y0 + (y1_d - y0) * (xmin - x0) / (x1_d - x0);
                x = xmin;
            }

            if (outcodeOut == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = computeOutCode(x0, y0, xmin, ymin, xmax, ymax);
            } else {
                x1_d = x;
                y1_d = y;
                outcode1 = computeOutCode(x1_d, y1_d, xmin, ymin, xmax, ymax);
            }
        }
    }

    if (accept) {
        x1 = static_cast<int>(std::round(x0));
        y1 = static_cast<int>(std::round(y0));
        x2 = static_cast<int>(std::round(x1_d));
        y2 = static_cast<int>(std::round(y1_d));
        return true;
    }
    return false;
}
}

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

    if (x + width < -10 || x > 490 || y + height < -10 || y > 282) {
        return;
    }

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
    if (x + w < -10 || x > 490 || y + h < -10 || y > 282) return;

    g2dBeginRects(NULL);
    g2dSetColor(currentColor);
    g2dSetCoordXY(x, y);
    g2dSetScaleWH(w, h);
    g2dAdd();
    g2dEnd();
}

void Graphics::drawArc(int x, int y, int width, int height, int startAngle, int arcAngle)
{
    if (x + width < -10 || x > 490 || y + height < -10 || y > 282) return;

    int rx = width / 2;
    int ry = height / 2;
    int cx = x + rx;
    int cy = y + ry;
    if (rx <= 0 && ry <= 0) return;

    int step = 10;
    for (int angle = startAngle; angle < startAngle + arcAngle; angle += step) {
        int a1 = angle;
        int a2 = (angle + step > startAngle + arcAngle) ? (startAngle + arcAngle) : (angle + step);
        float rad1 = a1 * PI_CONV;
        float rad2 = a2 * PI_CONV;
        int px1 = static_cast<int>(cx + rx * cos(rad1));
        int py1 = static_cast<int>(cy - ry * sin(rad1));
        int px2 = static_cast<int>(cx + rx * cos(rad2));
        int py2 = static_cast<int>(cy - ry * sin(rad2));
        drawLine(px1, py1, px2, py2);
    }
}

void Graphics::fillArc(int x, int y, int w, int h, int startAngle, int arcAngle)
{
    (void)startAngle; (void)arcAngle;
    if (x + w < -10 || x > 490 || y + h < -10 || y > 282) return;

    int rx = w / 2;
    int ry = h / 2;
    int cx = x + rx;
    int cy = y + ry;
    if (rx <= 0 || ry <= 0) return;

    for (int dy = -ry; dy <= ry; dy++) {
        float dx_max = rx * sqrt(1.0f - (float)(dy * dy) / (float)(ry * ry));
        int x1 = cx - (int)dx_max;
        int x2 = cx + (int)dx_max;
        int py = cy + dy;
        drawLine(x1, py, x2, py);
    }
}

void Graphics::drawLine(int x1, int y1, int x2, int y2)
{
    if (!clipLine(x1, y1, x2, y2)) {
        return;
    }

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

    if (x + w < -10 || x > 490 || y + h < -10 || y > 282) return;

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
