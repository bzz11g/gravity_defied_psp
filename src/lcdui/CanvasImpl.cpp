#include "CanvasImpl.h"

#include "../Micro.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept>
#include <iostream>

#include "Canvas.h"

CanvasImpl::CanvasImpl(Canvas* canvas)
    : controller(nullptr)
    , controllerUpPressed(false)
    , controllerDownPressed(false)
    , controllerLeftPressed(false)
    , controllerRightPressed(false)
{
    this->canvas = canvas;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        throw std::runtime_error(SDL_GetError());
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        throw std::runtime_error(IMG_GetError());
    }

    if (TTF_Init() == -1) {
        throw std::runtime_error(TTF_GetError());
    }

    window = SDL_CreateWindow(
        0,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_SHOWN);

    if (!window) {
        throw std::runtime_error(SDL_GetError());
    }

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    openFirstController();
}

CanvasImpl::~CanvasImpl()
{
    closeController();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    IMG_Quit();
    TTF_Quit();
}

void CanvasImpl::repaint()
{
    SDL_RenderPresent(renderer);
}

int CanvasImpl::getWidth()
{
    return width;
}

int CanvasImpl::getHeight()
{
    return height;
}

SDL_Renderer* CanvasImpl::getRenderer()
{
    return renderer;
}

void CanvasImpl::processEvents()
{
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
        case SDL_QUIT:
            exit(0); // IMPROVE This is a super dumb way to finish the game, but it works
            break;
        case SDL_KEYDOWN: {
            int sym = e.key.keysym.sym;
            int keyCode = convertKeyCharToKeyCode(sym);
            if (keyCode != 0) {
                canvas->publicKeyPressed(keyCode);
            } else if (sym >= SDLK_0 && sym <= SDLK_9) {
                canvas->publicKeyPressed(sym);
            }
        } break;
        case SDL_KEYUP: {
            int sym = e.key.keysym.sym;
            int keyCode = convertKeyCharToKeyCode(sym);
            if (keyCode != 0) {
                canvas->publicKeyReleased(keyCode);
            } else if (sym >= SDLK_0 && sym <= SDLK_9) {
                canvas->publicKeyReleased(sym);
            } else if (sym == SDLK_ESCAPE) {
                canvas->pressedEsc();
            }
        } break;
        case SDL_CONTROLLERDEVICEADDED:
            if (!controller) {
                openFirstController();
            }
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (e.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) {
                closeController();
            }
            break;
        case SDL_CONTROLLERAXISMOTION:
            handleControllerAxisMotion(e.caxis.axis, e.caxis.value);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            handleControllerButtonDown(e.cbutton.button);
            break;
        case SDL_CONTROLLERBUTTONUP:
            handleControllerButtonUp(e.cbutton.button);
            break;
        default:
            break;
        }
    }
}

void CanvasImpl::openFirstController()
{
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                std::cout << "Controller connected: " << SDL_GameControllerName(controller) << std::endl;
                return;
            }
        }
    }
}

void CanvasImpl::closeController()
{
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
        std::cout << "Controller disconnected" << std::endl;
    }
}

void CanvasImpl::handleControllerAxisMotion(int axis, int value)
{
    switch (axis) {
    case SDL_CONTROLLER_AXIS_LEFTY:
    case SDL_CONTROLLER_AXIS_RIGHTY:
        if (value < -ANALOG_DEADZONE) {
            if (!controllerUpPressed) {
                controllerUpPressed = true;
                canvas->publicKeyPressed(Canvas::Keys::UP);
            }
        } else {
            if (controllerUpPressed) {
                controllerUpPressed = false;
                canvas->publicKeyReleased(Canvas::Keys::UP);
            }
        }
        if (value > ANALOG_DEADZONE) {
            if (!controllerDownPressed) {
                controllerDownPressed = true;
                canvas->publicKeyPressed(Canvas::Keys::DOWN);
            }
        } else {
            if (controllerDownPressed) {
                controllerDownPressed = false;
                canvas->publicKeyReleased(Canvas::Keys::DOWN);
            }
        }
        break;
    case SDL_CONTROLLER_AXIS_LEFTX:
    case SDL_CONTROLLER_AXIS_RIGHTX:
        if (value < -ANALOG_DEADZONE) {
            if (!controllerLeftPressed) {
                controllerLeftPressed = true;
                canvas->publicKeyPressed(Canvas::Keys::LEFT);
            }
        } else {
            if (controllerLeftPressed) {
                controllerLeftPressed = false;
                canvas->publicKeyReleased(Canvas::Keys::LEFT);
            }
        }
        if (value > ANALOG_DEADZONE) {
            if (!controllerRightPressed) {
                controllerRightPressed = true;
                canvas->publicKeyPressed(Canvas::Keys::RIGHT);
            }
        } else {
            if (controllerRightPressed) {
                controllerRightPressed = false;
                canvas->publicKeyReleased(Canvas::Keys::RIGHT);
            }
        }
        break;
    }
}

void CanvasImpl::handleControllerButtonDown(int button)
{
    switch (button) {
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        canvas->publicKeyPressed(Canvas::Keys::UP);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        canvas->publicKeyPressed(Canvas::Keys::DOWN);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        canvas->publicKeyPressed(Canvas::Keys::LEFT);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        canvas->publicKeyPressed(Canvas::Keys::RIGHT);
        break;
    case SDL_CONTROLLER_BUTTON_A:
#ifdef PSP
        canvas->publicKeyPressed(Canvas::Keys::DOWN);
#else
        canvas->publicKeyPressed(Canvas::Keys::FIRE);
#endif
        break;
    case SDL_CONTROLLER_BUTTON_B:
#ifdef PSP
        canvas->publicKeyPressed(Canvas::Keys::RIGHT);
#else
        canvas->pressedEsc();
#endif
        break;
    case SDL_CONTROLLER_BUTTON_X:
#ifdef PSP
        canvas->publicKeyPressed(Canvas::Keys::LEFT);
#endif
        break;
    case SDL_CONTROLLER_BUTTON_Y:
#ifdef PSP
        canvas->publicKeyPressed(Canvas::Keys::UP);
#endif
        break;
    case SDL_CONTROLLER_BUTTON_START:
#ifdef PSP
        canvas->pressedEsc();
#else
        canvas->publicKeyPressed(Canvas::Keys::FIRE);
#endif
        break;
#ifdef PSP
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        canvas->publicKeyPressed(Canvas::Keys::UP);
        canvas->publicKeyPressed(Canvas::Keys::LEFT);
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        canvas->publicKeyPressed(Canvas::Keys::UP);
        canvas->publicKeyPressed(Canvas::Keys::RIGHT);
        break;
#endif
    }
}

void CanvasImpl::handleControllerButtonUp(int button)
{
    switch (button) {
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        canvas->publicKeyReleased(Canvas::Keys::UP);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        canvas->publicKeyReleased(Canvas::Keys::DOWN);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        canvas->publicKeyReleased(Canvas::Keys::LEFT);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        canvas->publicKeyReleased(Canvas::Keys::RIGHT);
        break;
    case SDL_CONTROLLER_BUTTON_A:
#ifdef PSP
        canvas->publicKeyReleased(Canvas::Keys::DOWN);
#else
        canvas->publicKeyReleased(Canvas::Keys::FIRE);
#endif
        break;
    case SDL_CONTROLLER_BUTTON_B:
#ifdef PSP
        canvas->publicKeyReleased(Canvas::Keys::RIGHT);
#endif
        break;
    case SDL_CONTROLLER_BUTTON_X:
#ifdef PSP
        canvas->publicKeyReleased(Canvas::Keys::LEFT);
#endif
        break;
    case SDL_CONTROLLER_BUTTON_Y:
#ifdef PSP
        canvas->publicKeyReleased(Canvas::Keys::UP);
#endif
        break;
    case SDL_CONTROLLER_BUTTON_START:
#ifdef PSP
        // START on PSP is handled as pressedEsc in down handler, no release action needed
#else
        canvas->publicKeyReleased(Canvas::Keys::FIRE);
#endif
        break;
#ifdef PSP
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        canvas->publicKeyReleased(Canvas::Keys::UP);
        canvas->publicKeyReleased(Canvas::Keys::LEFT);
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        canvas->publicKeyReleased(Canvas::Keys::UP);
        canvas->publicKeyReleased(Canvas::Keys::RIGHT);
        break;
#endif
    }
}

int CanvasImpl::convertKeyCharToKeyCode(SDL_Keycode keyCode)
{
    switch (keyCode) {
    case SDLK_RETURN:
        return Canvas::Keys::FIRE;
    case SDLK_LEFT:
        return Canvas::Keys::LEFT;
    case SDLK_RIGHT:
        return Canvas::Keys::RIGHT;
    case SDLK_UP:
        return Canvas::Keys::UP;
    case SDLK_DOWN:
        return Canvas::Keys::DOWN;
    default:
        std::cout << "unknown keyEvent: " << keyCode << std::endl;
        return 0;
    }
}

void CanvasImpl::setWindowTitle(const std::string& title)
{
    SDL_SetWindowTitle(window, title.c_str());
}