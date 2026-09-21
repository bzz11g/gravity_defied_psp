#include "Micro.h"

#include "GameCanvas.h"
#include "GamePhysics.h"
#include "MenuManager.h"
#include "LevelLoader.h"
#include "utils/Time.h"
#include "lcdui/CanvasImpl.h"
#include "rms/RecordStore.h"

bool Micro::field_249 = false;
int Micro::gameLoadingStateStage = 0;

Micro::Micro()
{
}

Micro::~Micro()
{
}

void Micro::setNumPhysicsLoops(int value)
{
    numPhysicsLoops = value;
}

void Micro::gameToMenu()
{
    gameCanvas->removeMenuCommand();
    isInGameMenu = true;
    menuManager->addOkAndBackCommands();
}

void Micro::menuToGame()
{
    menuManager->removeOkAndBackCommands();
    isInGameMenu = false;
    gameCanvas->addMenuCommand();
}

int64_t Micro::goLoadingStep()
{
    ++gameLoadingStateStage;
    gameCanvas->repaint();
    int64_t startTimeMillis = Time::currentTimeMillis();
    switch (gameLoadingStateStage) {
    case 1:
        levelLoader = new LevelLoader(mrgFilePath);
        break;
    case 2:
        gamePhysics = new GamePhysics(levelLoader);
        gameCanvas->init(gamePhysics);
        break;
    case 3:
        menuManager = new MenuManager(this);
        menuManager->initPart(1);
        break;
    case 4:
        menuManager->initPart(2);
        break;
    case 5:
        menuManager->initPart(3);
        break;
    case 6:
        menuManager->initPart(4);
        break;
    case 7:
        menuManager->initPart(5);
        break;
    case 8:
        menuManager->initPart(6);
        break;
    case 9:
        menuManager->initPart(7);
        break;
    case 10:
        gameCanvas->setMenuManager(menuManager);
        gameCanvas->setViewPosition(-50, 150);
        setMode(1);
        break;
    default:
        --gameLoadingStateStage;

        // try {
        //     Thread.sleep(100L);
        // } catch (InterruptedException var3) {
        // }
        Time::sleep(100LL);
    }

    return Time::currentTimeMillis() - startTimeMillis;
}

void Micro::init()
{
    int64_t timeToLoading = 3000L;
    // Thread.yield();
    gameCanvas = new GameCanvas(this);
    gameCanvas->requestRepaint(1);

    while (!gameCanvas->isShown()) {
        goLoadingStep();
    }

    int64_t deltaTimeMs;
    while (timeToLoading > 0L) {
        deltaTimeMs = goLoadingStep();
        timeToLoading -= deltaTimeMs;
    }

    gameCanvas->requestRepaint(2);

    for (timeToLoading = 3000L; timeToLoading > 0L; timeToLoading -= deltaTimeMs) {
        deltaTimeMs = goLoadingStep();
    }

    while (gameLoadingStateStage < 10) {
        goLoadingStep();
    }

    gameCanvas->requestRepaint(0);
    isInited = true;
}

void Micro::restart(bool var1)
{
    gamePhysics->resetSmth(true);
    timeMs = 0;
    gameTimeMs = 0;
    field_246 = 0;
    if (var1) {
        gameCanvas->scheduleGameTimerTask(levelLoader->getName(menuManager->getCurrentLevel(), menuManager->getCurrentTrack()), 3000);
    }

    gameCanvas->method_129();
}

void Micro::destroyApp(bool var1)
{
    (void)var1;
    field_249 = false;
    field_242 = true;
    if (menuManager != nullptr) {
        menuManager->saveSmthToRecordStoreAndCloseIt();
    }
    RecordStore::saveAllOpened();
}

void Micro::startApp(int argc, char** argv)
{
    const char* progName = (argc > 0 && argv != nullptr && argv[0] != nullptr) ? argv[0] : nullptr;

    if (argc > 1 && argv != nullptr && argv[1] != nullptr) {
        std::string argv1(argv[1]);

        if (argv1 == "-h" || argv1 == "--help") {
            showHelp(progName != nullptr ? progName : "GravityDefied");
            return;
        }

        this->mrgFilePath = argv1;
    }

    RecordStore::setRecordStoreDir(progName);

    field_249 = true;
    // if (thread == null) {
    //     thread = new Thread(this);
    //     thread.start();
    // }
    run();
}

// original method
void Micro::run()
{
    if (!isInited) {
        init();
    }

    gameCanvas->setCommandListener(gameCanvas);
    restart(false);
    menuManager->method_201(0);
    if (menuManager->method_196()) {
        restart(true);
    }

    int64_t lastTime = Time::currentTimeMillis();
    int64_t accumulator = 0;

    while (field_249) {
        int var5;
        if (gamePhysics->method_21() != menuManager->method_210()) {
            var5 = gameCanvas->loadSprites(menuManager->method_210());
            gamePhysics->method_22(var5);
            menuManager->method_211(var5);
        }

        bool var10000 = true;
        try {
            if (isInGameMenu) {
                menuManager->method_201(1);
                if (menuManager->method_196()) {
                    restart(true);
                }
                lastTime = Time::currentTimeMillis();
                accumulator = 0;
            }

            int64_t now = Time::currentTimeMillis();
            int64_t frameTime = now - lastTime;
            if (frameTime < 0) frameTime = 0;
            if (frameTime > 250) frameTime = 250;
            lastTime = now;
            accumulator += frameTime;

            const int64_t physicsStepMs = 30LL;

            while (accumulator >= physicsStepMs && field_249) {
                for (int i = numPhysicsLoops; i > 0; --i) {
                    if (field_248) {
                        gameTimeMs += 20L;
                    }

                    if (timeMs == 0L) {
                        timeMs = Time::currentTimeMillis();
                    }

                    if ((var5 = gamePhysics->updatePhysics()) == 3 && field_246 == 0L) {
                        field_246 = Time::currentTimeMillis() + 3000L;
                        gameCanvas->scheduleGameTimerTask("Crashed", 3000);
                        gameCanvas->repaint();
                        gameCanvas->serviceRepaints();
                    }

                    if (field_246 != 0L && field_246 < Time::currentTimeMillis()) {
                        restart(true);
                        lastTime = Time::currentTimeMillis();
                        accumulator = 0;
                        break;
                    }

                    if (var5 == 5) {
                        gameCanvas->scheduleGameTimerTask("Crashed", 3000);
                        gameCanvas->repaint();
                        gameCanvas->serviceRepaints();

                        int64_t var7 = 1000L;
                        if (field_246 > 0L) {
                            var7 = std::min(field_246 - Time::currentTimeMillis(), static_cast<int64_t>(1000));
                        }

                        if (var7 > 0L) {
                            Time::sleep(var7);
                        }

                        restart(true);
                        lastTime = Time::currentTimeMillis();
                        accumulator = 0;
                        break;
                    } else if (var5 == 4) {
                        timeMs = 0L;
                        gameTimeMs = 0L;
                    } else if (var5 == 1 || var5 == 2) {
                        if (var5 == 2) {
                            gameTimeMs -= 10L;
                        }

                        goalLoop();
                        lastTime = Time::currentTimeMillis();
                        accumulator = 0;
                        menuManager->method_215(gameTimeMs / 10L);
                        menuManager->method_201(2);
                        if (menuManager->method_196()) {
                            restart(true);
                            lastTime = Time::currentTimeMillis();
                            accumulator = 0;
                        }

                        if (!field_249) {
                            break;
                        }
                    }

                    field_248 = var5 != 4;
                }

                accumulator -= physicsStepMs;
            }

            var10000 = field_249;
        } catch (std::exception& var15) {
            continue;
        }

        if (!var10000) {
            break;
        }

        try {
            gamePhysics->method_53();
            int64_t renderTime = Time::currentTimeMillis() - lastTime;
            if (renderTime < 16LL) {
                Time::sleep(16LL - renderTime);
            }

            gameCanvas->repaint();
        } catch (std::exception& var14) {
        }
    }

    destroyApp(true);
}

void Micro::goalLoop()
{
    if (!gamePhysics->field_69) {
        gameCanvas->scheduleGameTimerTask("Wheelie!", 1000);
    } else {
        gameCanvas->scheduleGameTimerTask("Finished", 1000);
    }

    int64_t lastTime = Time::currentTimeMillis();
    int64_t accumulator = 0;
    int64_t endTime = lastTime + 1000L;

    while (Time::currentTimeMillis() < endTime) {
        if (isInGameMenu) {
            gameCanvas->repaint();
            return;
        }

        int64_t now = Time::currentTimeMillis();
        int64_t frameTime = now - lastTime;
        if (frameTime < 0) frameTime = 0;
        if (frameTime > 250) frameTime = 250;
        lastTime = now;
        accumulator += frameTime;

        while (accumulator >= 30LL) {
            for (int i = numPhysicsLoops; i > 0; --i) {
                if (gamePhysics->updatePhysics() == 5) {
                    int64_t deltaTime = endTime - Time::currentTimeMillis();
                    if (deltaTime > 0L) {
                        Time::sleep(deltaTime);
                    }
                    return;
                }
            }
            accumulator -= 30LL;
        }

        gamePhysics->method_53();
        int64_t renderTime = Time::currentTimeMillis() - now;
        if (renderTime < 16LL) {
            Time::sleep(16LL - renderTime);
        }
        gameCanvas->repaint();
    }
}

void Micro::setMode(int mode)
{
    gamePhysics->setMode(mode);
}

void Micro::showHelp(const char* progName)
{
    std::cout << "Usage: " << progName << " <FILE>" << std::endl
              << "Example:" << std::endl
              << "  " << progName << " levels.mrg  # A path to a custom levels file could be specified" << std::endl
              << "  " << progName << "             # When no path is specified, the built-in levels file will be used" << std::endl
              << std::endl;
}