#pragma once

#include "gmath.hpp"

struct Camera {
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fov;
    float nearClip;
    float farClip;
    float aspect;

    Matrix4 mvp;

    Camera()
    {
        position = vec3Zero();
        target = vec3Zero();
        up = vec3(0.0F, 0.0F, 1.0F);
        fov = deg2Rad(70.F);
        nearClip = 0.05F;
        farClip = 100.0F;
        aspect = 0.0F;
    }

    void setAspectRatio(float aspectRatio)
    {
        aspect = aspectRatio;
    }

    void updateMVP()
    {
        Matrix4 view = mat4LookAt(position, target, up);
        Matrix4 projection = mat4CreatePerspectiveFieldOfView(fov, aspect, nearClip, farClip);
        mvp = mat4Multiply(view, projection);
    }
};
