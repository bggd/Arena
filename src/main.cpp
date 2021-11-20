#define SDL_MAIN_HANDLED
#include "game_app.hpp"
#include "model.hpp"
#include "gmath.hpp"

Model gModel;

void initOpenGL()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    //glFrontFace(GL_CCW);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void onInit()
{
    initOpenGL();
}

void onUpdate(const GameAppState& appState)
{
    float scale = 64.0F;
    Matrix4 model = mat4CreateScale(vec3(scale, scale, scale));
    Matrix4 view = mat4LookAt(vec3(0.0F, -200.0F, 0.0F), vec3Zero(), vec3(0.0F, 0.0F, 1.0F));
    //Matrix4 proj = mat4CreateOrthographicOffCenter(-320.0F, 320.0F, -240.0F, 240.0F, 1.0F, 1000.0F);
    Matrix4 proj = mat4CreatePerspectiveFieldOfView(deg2Rad(80.0F), 640.0F / 480.0F, 1.0F, 1000.0F);
    Matrix4 mvp = mat4Multiply(mat4Multiply(model, view), proj);
    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadMatrixf(mat4Ptr(mvp));
    gModel.draw();
}

int main()
{
    gModel.load("monkey.glb");

    GameAppConfig appConfig;
    appConfig.width = 640;
    appConfig.height = 480;
    appConfig.title = "bhr";
    GameApp app = {};
    app.onInit = onInit;
    app.onUpdate = onUpdate;
    runGameApp(app, appConfig);
    return 0;
}
