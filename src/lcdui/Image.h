#pragma once

#include <string>
#include "psp/glib2d.h"

class Image {
private:
    g2dImage* image = nullptr;

public:
    Image(const std::string& embeddedPath);
    Image(int width, int height);
    ~Image();

    int getWidth() const;
    int getHeight() const;
    g2dImage* getG2DImage() const;
};
