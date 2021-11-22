#define SDL_MAIN_HANDLED
#include "game_app.hpp"
#include "model.hpp"
#include "gmath.hpp"
#include "camera.hpp"

Camera gCam;
Model gModel;

void initOpenGL()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    //glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void onInit()
{
    initOpenGL();
    gCam.setAspectRatio(640.0F / 480.0F);
    //gCam.fov = deg2Rad(80.0F);
    gCam.position = vec3(0.0F, -200.0F, 0.0F);
    gCam.farClip = 1000.0F;
}

void onUpdate(const GameAppState& appState)
{
    float scale = 32.0F;
    Matrix4 model = mat4CreateScale(vec3(scale, scale, scale));
    model = mat4Multiply(mat4CreateFromAxisAngle(vec3(0.0F, 0.0F, 1.0F), deg2Rad(45.0F)), model);
    gModel.updateAnimation(appState.dt);
    gModel.updateMesh(model);
    gCam.updateMVP();
    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadMatrixf(mat4Ptr(gCam.mvp));
    gModel.draw();
}

int main()
{
    gModel.load("cube.fbx");
    gModel.animation.setCurrentAction("Armature|RunCycle");

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
