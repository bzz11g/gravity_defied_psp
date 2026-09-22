#include "Image.h"
#include <cmrc/cmrc.hpp>
#include <iostream>

CMRC_DECLARE(assets);

Image::Image(int width, int height)
{
    this->image = g2dTexCreatePlaceholder(width, height);
}

Image::Image(const std::string& embeddedPath)
{
    cmrc::embedded_filesystem embeddedFs = cmrc::assets::get_filesystem();
    if (embeddedFs.exists(embeddedPath)) {
        cmrc::file fileData = embeddedFs.open(embeddedPath);
        this->image = g2dTexLoad(NULL, (unsigned char*)fileData.begin(), fileData.size(), G2D_SWIZZLE);
    } else {
        this->image = nullptr;
    }
}

Image::~Image()
{
    if (this->image) {
        g2dTexFree(&this->image);
    }
}

g2dImage* Image::getG2DImage() const
{
    return this->image;
}

int Image::getWidth() const
{
    return this->image ? this->image->w : 0;
}

int Image::getHeight() const
{
    return this->image ? this->image->h : 0;
}
