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

struct Quaternion {
    float x, y, z, w;
};

inline float deg2Rad(float degree)
{
    float radian = degree * (3.14159265358979323846F / 180.0F);
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

inline Vector3 vec3Add(const Vector3& a, const Vector3& b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Vector3 vec3Subtract(const Vector3& a, const Vector3& b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline Vector3 vec3Multiply(const Vector3& v, float scalar)
{
    return {v.x * scalar, v.y * scalar, v.z * scalar};
}

inline float vec3Dot(const Vector3& a, const Vector3& b)
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

inline float vec3LengthSquared(const Vector3& v)
{
    return vec3Dot(v, v);
}

inline float vec3Length(const Vector3& v)
{
    return sqrtf(vec3LengthSquared(v));
}

inline Vector3 vec3Normalize(const Vector3& v)
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

inline Vector3 vec3Cross(const Vector3& a, const Vector3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

inline Vector3 vec3Negate(const Vector3& v)
{
    return {-v.x, -v.y, -v.z};
}

inline Vector3 vec3Transform(const Vector3& v, const Quaternion& q)
{
    Vector3 qv = vec3(q.x, q.y, q.z);
    return vec3Add(vec3Multiply(vec3Cross(v, qv), 2.0F * q.w), vec3Add(vec3Multiply(v, q.w * q.w - vec3Dot(qv, qv)), vec3Multiply(qv, 2.0F * vec3Dot(qv, v))));
}

inline Vector4 vec4Zero()
{
    return {0.0F, 0.0F, 0.0F, 0.0F};
}

inline Vector4 vec4One()
{
    return {1.0F, 1.0F, 1.0F, 1.0F};
}

inline Vector4 vec4Transform(const Vector4& v, const Matrix4& m)
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

inline Matrix4 mat4Identity()
{
    return {
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F};
}

inline const float* mat4Ptr(const Matrix4& m)
{
    return (const float*)&m;
}

inline Matrix4 mat4Multiply(const Matrix4& a, const Matrix4& b)
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

inline Matrix4 mat4CreateTranslation(Vector3 v)
{
    return {
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        v.x, v.y, v.z, 1.0F};
}

inline Matrix4 mat4CreateScale(Vector3 v)
{
    Matrix4 m = mat4Zero();
    m.m11 = v.x;
    m.m22 = v.y;
    m.m33 = v.z;
    m.m44 = 1.0F;
    return m;
}

inline Matrix4 mat4CreateFromAxisAngle(Vector3 axisUnit, float angleRadian)
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

inline Matrix4 mat4CreateFromQuaternion(const Quaternion& q)
{
    Vector3 r = vec3Transform(vec3(1.0F, 0.0F, 0.0F), q);
    Vector3 u = vec3Transform(vec3(0.0F, 1.0F, 0.0F), q);
    Vector3 f = vec3Transform(vec3(0.0F, 0.0F, 1.0F), q);

    Matrix4 m = {
        r.x, r.y, r.z, 0.0F,
        u.x, u.y, u.z, 0.0F,
        f.x, f.y, f.z, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F};
    return m;
}

inline Matrix4 mat4CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane)
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

inline Matrix4 mat4CreatePerspectiveFieldOfView(float fovYRadian, float aspect, float nearPlaneDistance, float farPlaneDistance)
{
    float f = 1.0F / tanf(fovYRadian / 2.0F);

    Matrix4 m = {
        f / aspect, 0.0F, 0.0F, 0.0F,
        0.0F, f, 0.0F, 0.0F,
        0.0F, 0.0F, (farPlaneDistance + nearPlaneDistance) / (nearPlaneDistance - farPlaneDistance), -1.0F,
        0.0F, 0.0F, (2.0F * farPlaneDistance * nearPlaneDistance) / (nearPlaneDistance - farPlaneDistance), 0.0F};

    return m;
}

inline Matrix4 mat4LookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
{
    Vector3 cameraDirection = vec3Normalize(vec3Subtract(eye, center));
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

inline Quaternion quatCreateAxisAngle(Vector3 axisUnit, float angleRadian)
{
    float s = sinf(angleRadian / 2.0F);
    return {axisUnit.x * s, axisUnit.y * s, axisUnit.z * s, cosf(angleRadian / 2.0F)};
}

inline Quaternion quatAdd(const Quaternion& a, const Quaternion& b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

inline Quaternion quatSubtraction(const Quaternion& a, const Quaternion& b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

inline Quaternion quatMultiply(const Quaternion& a, const Quaternion& b)
{
    // clang-format off
    return {
        ( a.x * b.w) + (a.y * b.z) - (a.z * b.y) + (a.w * b.x),
        (-a.x * b.z) + (a.y * b.w) + (a.z * b.x) + (a.w * b.y),
        ( a.x * b.y) - (a.y * b.x) + (a.z * b.w) + (a.w * b.z),
        (-a.x * b.x) - (a.y * b.y) - (a.z * b.z) + (a.w * b.w)};
    // clang-format on
}

inline Quaternion quatMultiply(const Quaternion& q, float scalar)
{
    return {q.x * scalar, q.y * scalar, q.z * scalar, q.w * scalar};
}

inline Quaternion quatNegate(const Quaternion& q)
{
    return {-q.x, -q.y, -q.z, -q.w};
}

inline float quatDot(const Quaternion& a, const Quaternion& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline float quatLengthSquared(const Quaternion& q)
{
    return quatDot(q, q);
}

inline float quatLength(const Quaternion& q)
{
    return sqrtf(quatLengthSquared(q));
}

inline Quaternion quatConjugate(const Quaternion& q)
{
    return {-q.x, -q.y, -q.z, q.w};
}

inline Vector3 operator*(const Vector3& v, float scalar)
{
    return vec3Multiply(v, scalar);
}

inline Vector3 operator+(const Vector3& a, const Vector3& b)
{
    return vec3Add(a, b);
}

inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    return vec3Subtract(a, b);
}
