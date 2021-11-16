#define SDL_MAIN_HANDLED
#include "game_app.hpp"

int main()
{
    GameAppConfig appConfig;
    appConfig.width = 640;
    appConfig.height = 480;
    appConfig.title = "bhr";
    GameApp app = {};
    runGameApp(app, appConfig);
    return 0;
}
