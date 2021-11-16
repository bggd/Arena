#pragma once

#include "glad.h"
#include <SDL.h>
#include <cassert>
#include <cstdio>
#include <cstdint>

struct GameAppConfig {
    uint32_t width;
    uint32_t height;
    const char* title;
};

struct GameAppState {
    bool isPressedLMB = false;
    bool isPressedMMB = false;
    bool isPressedRMB = false;
    uint32_t mouseX = 0;
    uint32_t mouseY = 0;
    uint32_t mousePrevX = 0;
    uint32_t mousePrevY = 0;
    int32_t mouseWheelY = 0;
    double dt = 0.0;
};

struct GameApp {
    void (*onInit)() = []() {};
    void (*onShutdown)() = []() {};
    void (*onUpdate)(const GameAppState&) = [](const GameAppState&) {};
};

void runGameApp(GameApp app, GameAppConfig appConfig)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow(appConfig.title, SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, appConfig.width,
                                    appConfig.height, SDL_WINDOW_OPENGL);
    assert(window);

    SDL_GLContext glctx = SDL_GL_CreateContext(window);
    assert(glctx);
    SDL_GL_MakeCurrent(window, glctx);

    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    assert(version != 0);
    printf("OpenGL: %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    app.onInit();

    double fpsCap = 1.0 / 60.0;
    uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t prevTime = SDL_GetPerformanceCounter();

    GameAppState state = {};

    for (;;)
    {
        uint64_t nowTime = SDL_GetPerformanceCounter();
        double deltaTime = double(nowTime - prevTime) / freq;
        prevTime = nowTime;

        while (deltaTime < fpsCap)
        {
            uint32_t ms = (uint32_t)((fpsCap - deltaTime) * 1000.0);
            SDL_Delay(ms);
            nowTime = SDL_GetPerformanceCounter();
            deltaTime += double(nowTime - prevTime) / freq;
            prevTime = nowTime;
        }

        state.mouseWheelY = 0;
        SDL_Event ev;
        while (SDL_PollEvent(&ev) != 0)
        {
            if (ev.type == SDL_QUIT)
            {
                goto app_quit;
            }
            else if (ev.type == SDL_MOUSEWHEEL)
            {
                state.mouseWheelY = ev.wheel.y;
                if (ev.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
                {
                    state.mouseWheelY *= -1;
                }
            }
        }

        int mx, my;
        Uint32 buttons = SDL_GetMouseState(&mx, &my);
        state.mousePrevX = state.mouseX;
        state.mousePrevY = state.mouseY;
        state.mouseX = uint32_t(mx);
        state.mouseY = uint32_t(my);
        state.isPressedLMB = ((buttons & SDL_BUTTON_LMASK) != 0);
        state.isPressedMMB = ((buttons & SDL_BUTTON_MMASK) != 0);
        state.isPressedRMB = ((buttons & SDL_BUTTON_RMASK) != 0);

        state.dt = deltaTime;
        app.onUpdate(state);

        SDL_GL_SwapWindow(window);
    }

app_quit:
    app.onShutdown();

    SDL_GL_DeleteContext(glctx);
    SDL_DestroyWindow(window);

    SDL_Quit();
}
