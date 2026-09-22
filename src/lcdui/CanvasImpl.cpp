#include "CanvasImpl.h"
#include "Canvas.h"
#include "../Micro.h"
#include "psp/glib2d.h"
#include <pspctrl.h>
#include <pspdisplay.h>

CanvasImpl::CanvasImpl(Canvas* canvas) : canvas(canvas)
{
}

CanvasImpl::~CanvasImpl()
{
}

void CanvasImpl::clear()
{
    g2dClear(WHITE);
}

void CanvasImpl::repaint()
{
    g2dFlip(G2D_VSYNC);
    sceDisplayWaitVblankStart();
}

int CanvasImpl::getWidth()
{
    return width;
}

int CanvasImpl::getHeight()
{
    return height;
}

void CanvasImpl::processEvents()
{
    SceCtrlData pad;
    sceCtrlPeekBufferPositive(&pad, 1);

    uint32_t currentButtons = pad.Buttons;
    uint32_t pressedButtons = currentButtons & ~lastButtons;
    uint32_t releasedButtons = lastButtons & ~currentButtons;
    lastButtons = currentButtons;

    bool isMenu = Micro::isInGameMenu;

    auto mapButtonToKey = [isMenu](uint32_t btn) -> int {
        switch (btn) {
        case PSP_CTRL_UP: return Canvas::Keys::UP;
        case PSP_CTRL_DOWN: return Canvas::Keys::DOWN;
        case PSP_CTRL_LEFT: return Canvas::Keys::LEFT;
        case PSP_CTRL_RIGHT: return Canvas::Keys::RIGHT;

        case PSP_CTRL_CROSS:
            return isMenu ? static_cast<int>(Canvas::Keys::FIRE) : static_cast<int>('8');
        case PSP_CTRL_CIRCLE:
            return isMenu ? 0 : static_cast<int>('6');
        case PSP_CTRL_SQUARE:
            return isMenu ? 0 : static_cast<int>('4');
        case PSP_CTRL_TRIANGLE:
            return isMenu ? 0 : static_cast<int>('2');

        case PSP_CTRL_LTRIGGER:
            return isMenu ? 0 : static_cast<int>('1');
        case PSP_CTRL_RTRIGGER:
            return isMenu ? 0 : static_cast<int>('3');

        case PSP_CTRL_START:
            return isMenu ? static_cast<int>(Canvas::Keys::FIRE) : 0;
        default:
            return 0;
        }
    };

    uint32_t buttons[] = {
        PSP_CTRL_UP, PSP_CTRL_DOWN, PSP_CTRL_LEFT, PSP_CTRL_RIGHT,
        PSP_CTRL_CROSS, PSP_CTRL_CIRCLE, PSP_CTRL_SQUARE, PSP_CTRL_TRIANGLE,
        PSP_CTRL_LTRIGGER, PSP_CTRL_RTRIGGER, PSP_CTRL_START
    };

    for (uint32_t btn : buttons) {
        if (pressedButtons & btn) {
            int keyCode = mapButtonToKey(btn);
            if (keyCode != 0) {
                canvas->publicKeyPressed(keyCode);
            }
        }
        if (releasedButtons & btn) {
            int keyCode = mapButtonToKey(btn);
            if (keyCode != 0) {
                canvas->publicKeyReleased(keyCode);
            } else if ((btn == PSP_CTRL_CIRCLE && isMenu) || (btn == PSP_CTRL_START && !isMenu)) {
                canvas->pressedEsc();
            }
        }
    }

    // Analog stick polling
    bool analogLeft = pad.Lx < 40;
    bool analogRight = pad.Lx > 216;
    bool analogUp = pad.Ly < 40;
    bool analogDown = pad.Ly > 216;

    if (analogLeft && !lastAnalogLeft) canvas->publicKeyPressed(Canvas::Keys::LEFT);
    if (!analogLeft && lastAnalogLeft) canvas->publicKeyReleased(Canvas::Keys::LEFT);

    if (analogRight && !lastAnalogRight) canvas->publicKeyPressed(Canvas::Keys::RIGHT);
    if (!analogRight && lastAnalogRight) canvas->publicKeyReleased(Canvas::Keys::RIGHT);

    if (analogUp && !lastAnalogUp) canvas->publicKeyPressed(Canvas::Keys::UP);
    if (!analogUp && lastAnalogUp) canvas->publicKeyReleased(Canvas::Keys::UP);

    if (analogDown && !lastAnalogDown) canvas->publicKeyPressed(Canvas::Keys::DOWN);
    if (!analogDown && lastAnalogDown) canvas->publicKeyReleased(Canvas::Keys::DOWN);

    lastAnalogLeft = analogLeft;
    lastAnalogRight = analogRight;
    lastAnalogUp = analogUp;
    lastAnalogDown = analogDown;
}

void CanvasImpl::setWindowTitle(const std::string& title)
{
    (void)title;
}
