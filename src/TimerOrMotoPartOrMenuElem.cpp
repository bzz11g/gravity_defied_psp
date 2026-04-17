#include "TimerOrMotoPartOrMenuElem.h"

#include "IMenuManager.h"
#include "GameMenu.h"
#include "lcdui/Graphics.h"

TimerOrMotoPartOrMenuElem::TimerOrMotoPartOrMenuElem()
{
    setToZeros();
}

TimerOrMotoPartOrMenuElem::TimerOrMotoPartOrMenuElem(int timerNo, Micro* micro)
{
    this->micro = micro;
    this->timerNo = timerNo;
}

TimerOrMotoPartOrMenuElem::TimerOrMotoPartOrMenuElem(std::string text, GameMenu* gameMenu, IMenuManager* menuManager)
{
    this->text = text + ">";
    this->gameMenu = gameMenu;
    this->menuManager = menuManager;
}

void TimerOrMotoPartOrMenuElem::setToZeros()
{
    xF16 = yF16 = angleF16 = 0;
    velocityXF16 = velocityYF16 = angularVelocityF16 = 0;
    forceAccumXF16 = forceAccumYF16 = torqueF16 = 0;
}

void TimerOrMotoPartOrMenuElem::setText(std::string text)
{
    this->text = text + ">";
}

std::string TimerOrMotoPartOrMenuElem::getText()
{
    return text;
}

bool TimerOrMotoPartOrMenuElem::isNotTextRender()
{
    return true;
}

void TimerOrMotoPartOrMenuElem::menuElemMethod(int action)
{
    switch (action) {
    case 1:
    case 2:
        menuManager->handleMenuSelection(this);
        gameMenu->setGameMenu(menuManager->getCurrentMenu());
        menuManager->switchToMenu(gameMenu, false);
    case 3:
    default:
        break;
    }
}

void TimerOrMotoPartOrMenuElem::setGameMenu(GameMenu* gameMenu)
{
    this->gameMenu = gameMenu;
}

void TimerOrMotoPartOrMenuElem::render(Graphics* graphics, int y, int x)
{
    graphics->drawString(text, x, y, 20);
}