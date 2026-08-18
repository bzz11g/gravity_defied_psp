#pragma once

#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

class Canvas;

class CanvasImpl {
private:
    Canvas* canvas;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_GameController* controller;

    const int width = 480;
    const int height = 272;

    static const int ANALOG_DEADZONE = 8000;

    bool controllerUpPressed;
    bool controllerDownPressed;
    bool controllerLeftPressed;
    bool controllerRightPressed;

    bool buttonAPressedAsFire = false;
    bool buttonBPressedAsEsc = false;

    static int convertKeyCharToKeyCode(SDL_Keycode keyCode);
    void openFirstController();
    void closeController();
    void handleControllerAxisMotion(int axis, int value);
    void handleControllerButtonDown(int button);
    void handleControllerButtonUp(int button);

public:
    CanvasImpl(Canvas* canvas);
    ~CanvasImpl();

    void repaint();
    int getWidth();
    int getHeight();

    SDL_Renderer* getRenderer();
    void processEvents();
    void setWindowTitle(const std::string& title);
};