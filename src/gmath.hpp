#pragma once

#include <cmath>

struct Vector3 {
    float x, y, z;
};

struct Vector4 {
    float x, y, z, w;
};

struct Matrix4 {
    float m11, m12, m13, m14,
        m21, m22, m23, m24,
        m31, m32, m33, m34,
        m41, m42, m43, m44;
};

inline float deg2Rad(float degree)
{
    float radian = degree * (3.14159265358979323846 / 180.0F);
    return radian;
}

inline Vector3 vec3Zero()
{
    return {0.0F, 0.0F, 0.0F};
}

inline Vector3 vec3(float x, float y, float z)
{
    return {x, y, z};
}

Vector3 vec3Add(const Vector3& a, const Vector3& b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3 vec3Sub(const Vector3& a, const Vector3& b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3 vec3Multiply(const Vector3& v, float scalar)
{
    return {v.x * scalar, v.y * scalar, v.z * scalar};
}

float vec3Dot(const Vector3& a, const Vector3& b)
{
    const float* A = (const float*)&a;
    const float* B = (const float*)&b;

    float dot = 0.0F;
    for (int i = 0; i < 3; ++i)
    {
        dot += A[i] * B[i];
    }

    return dot;
}

float vec3Length(const Vector3& v)
{
    return sqrtf(vec3Dot(v, v));
}

Vector3 vec3Normalize(const Vector3& v)
{
    const float* V = (const float*)&v;

    Vector3 n = vec3Zero();
    float* N = (float*)&n;
    float len = 1.0F / vec3Length(v);
    for (int i = 0; i < 3; ++i)
    {
        N[i] = V[i] * len;
    }

    return n;
}

Vector3 vec3Cross(const Vector3& a, const Vector3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

inline Vector4 vec4Zero()
{
    return {0.0F, 0.0F, 0.0F, 0.0F};
}

inline Vector4 vec4One()
{
    return {1.0F, 1.0F, 1.0F, 1.0F};
}

Vector4 vec4Transform(const Vector4& v, const Matrix4& m)
{
    Vector4 result = vec4Zero();
    const float* A = (const float*)&v;
    const float* B = (const float*)&m;
    float* out = (float*)&result;

    for (int j = 0; j < 4; ++j)
    {
        for (int k = 0; k < 4; ++k)
        {
            out[j] += A[k] * B[4 * k + j];
        }
    }

    return result;
}

inline Matrix4 mat4Zero()
{
    return {
        0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F};
}

const float* mat4Ptr(const Matrix4& m)
{
    return (const float*)&m;
}

Matrix4 mat4Multiply(const Matrix4& a, const Matrix4& b)
{
    Matrix4 m = mat4Zero();
    const float* A = (const float*)&a;
    const float* B = (const float*)&b;
    float* M = (float*)&m;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                M[4 * i + j] += A[4 * i + k] * B[4 * k + j];
            }
        }
    }
    return m;
}

Matrix4 mat4CreateScale(Vector3 v)
{
    Matrix4 m = mat4Zero();
    m.m11 = v.x;
    m.m22 = v.y;
    m.m33 = v.z;
    m.m44 = 1.0F;
    return m;
}

Matrix4 mat4CreateFromAxisAngle(Vector3 axisUnit, float angleRadian)
{
    float x = axisUnit.x;
    float y = axisUnit.y;
    float z = axisUnit.z;
    float c = cosf(angleRadian);
    float s = sinf(angleRadian);
    float t = 1.0F - c;

    Matrix4 m = {
        (x * x * t) + c, (y * x * t) + (z * s), (x * z * t) - (y * s), 0.0F,
        (x * y * t) - (z * s), (y * y * t) + c, (y * z * t) + (x * s), 0.0F,
        (x * z * t) + (y * s), (y * z * t) - (x * s), (z * z * t) + c, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F};
    return m;
}

Matrix4 mat4CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane)
{
    float tX = -((right + left) / (right - left));
    float tY = -((top + bottom) / (top - bottom));
    float tZ = -((zFarPlane + zNearPlane) / (zFarPlane - zNearPlane));

    Matrix4 m = {
        2.0F / (right - left), 0.0F, 0.0F, 0.0F,
        0.0F, 2.0F / (top - bottom), 0.0F, 0.0F,
        0.0F, 0.0F, -2.0F / (zFarPlane - zNearPlane), 0.0F,
        tX, tY, tZ, 1.0F};

    return m;
}

Matrix4 mat4CreatePerspectiveFieldOfView(float fovYRadian, float aspect, float nearPlaneDistance, float farPlaneDistance)
{
    float f = 1.0F / tanf(fovYRadian / 2.0F);

    Matrix4 m = {
        f / aspect, 0.0F, 0.0F, 0.0F,
        0.0F, f, 0.0F, 0.0F,
        0.0F, 0.0F, (farPlaneDistance + nearPlaneDistance) / (nearPlaneDistance - farPlaneDistance), -1.0F,
        0.0F, 0.0F, (2.0F * farPlaneDistance * nearPlaneDistance) / (nearPlaneDistance - farPlaneDistance), 0.0F};

    return m;
}

Matrix4 mat4LookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
{
    Vector3 cameraDirection = vec3Normalize(vec3Sub(eye, center));
    Vector3 cameraRight = vec3Normalize(vec3Cross(up, cameraDirection));
    Vector3 cameraUp = vec3Cross(cameraDirection, cameraRight);

    Matrix4 rotation = {
        cameraRight.x, cameraUp.x, cameraDirection.x, 0.0F,
        cameraRight.y, cameraUp.y, cameraDirection.y, 0.0F,
        cameraRight.z, cameraUp.z, cameraDirection.z, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F};

    Matrix4 translation = {
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        -eye.x, -eye.y, -eye.z, 1.0F};

    return mat4Multiply(translation, rotation);
}

Vector3 operator*(const Vector3& v, float scalar)
{
    return vec3Multiply(v, scalar);
}

Vector3 operator+(const Vector3& a, const Vector3& b)
{
    return vec3Add(a, b);
}

Vector3 operator-(const Vector3& a, const Vector3& b)
{
    return vec3Sub(a, b);
}
