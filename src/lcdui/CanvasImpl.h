#pragma once

#include <memory>
#include <string>
#include <cstdint>

class Canvas;

class CanvasImpl {
private:
    Canvas* canvas;

    const int width = 480;
    const int height = 272;

    uint32_t lastButtons = 0;
    bool lastAnalogLeft = false;
    bool lastAnalogRight = false;
    bool lastAnalogUp = false;
    bool lastAnalogDown = false;

public:
    CanvasImpl(Canvas* canvas);
    ~CanvasImpl();

    void clear();
    void repaint();
    int getWidth();
    int getHeight();

    void processEvents();
    void setWindowTitle(const std::string& title);
};
