#pragma once

#include "../gmath.hpp"
#include <vector>
#include <stack>
#include <cassert>
#include <cstdio>

struct BaseNode {
private:
    Vector3 position = vec3Zero();
    Quaternion rotation = quatIdentity();
    Vector3 scale = vec3One();
    Matrix4 localMatrix = mat4Identity();
    Matrix4 worldMatrix = mat4Identity();

    void updateLocalMatrix()
    {
        Vector3 x = vec3Transform(vec3(1.0F, 0.0F, 0.0F), rotation);
        Vector3 y = vec3Transform(vec3(0.0F, 1.0F, 0.0F), rotation);
        Vector3 z = vec3Transform(vec3(0.0F, 0.0F, 1.0F), rotation);
        x = vec3Multiply(x, scale.x);
        y = vec3Multiply(y, scale.y);
        z = vec3Multiply(z, scale.z);
        Matrix4 newMatrix = {
            x.x, x.y, x.z, 0.0F,
            y.x, y.y, y.z, 0.0F,
            z.x, z.y, z.z, 0.0F,
            position.x, position.y, position.z, 1.0F};
        localMatrix = newMatrix;
    }

public:
    Vector3 getPosition()
    {
        return position;
    }

    void setPosition(const Vector3& pos)
    {
        position = pos;
        updateLocalMatrix();
    }

    Quaternion getRotation()
    {
        return rotation;
    }

    void setRotation(const Quaternion& rot)
    {
        rotation = rot;
        updateLocalMatrix();
    }

    Vector3 getScale()
    {
        return scale;
    }

    void setScale(const Vector3& s)
    {
        scale = s;
        updateLocalMatrix();
    }

    const Matrix4& getLocalMatrix()
    {
        return localMatrix;
    }

    const Matrix4& getWorldMatrix()
    {
        return worldMatrix;
    }

    void setWorldMatrix(const Matrix4& m)
    {
        worldMatrix = m;
    }
};

struct GameObject : BaseNode {
    GameObject* parent = nullptr;
    std::vector<GameObject*> objects;

    void addObject(GameObject* obj)
    {
        assert(obj);
        assert(!obj->parent);
        obj->parent = this;
        objects.push_back(obj);
    }

    const std::vector<GameObject*>& getObjects()
    {
        return objects;
    }

    virtual void onUpdate(float dt) = 0;
};

struct SceneTree {
    GameObject* root = nullptr;

    void setRoot(GameObject* obj)
    {
        assert(obj);
        assert(!obj->parent);
        root = obj;
    }

    void updateGameObjects(float dt)
    {
        assert(root);

        std::stack<GameObject*> nodeStack;
        nodeStack.push(root);
        root->onUpdate(dt);
        while (nodeStack.size())
        {
            GameObject* parent = nodeStack.top();
            nodeStack.pop();
            for (GameObject* child : parent->getObjects())
            {
                child->onUpdate(dt);
                nodeStack.push(child);
            }
        }
    }

    void propagateTransform()
    {
        assert(root);

        std::stack<GameObject*> nodeStack;
        nodeStack.push(root);
        root->setWorldMatrix(root->getLocalMatrix());
        while (nodeStack.size())
        {
            GameObject* parent = nodeStack.top();
            nodeStack.pop();
            for (GameObject* child : parent->getObjects())
            {
                const Matrix4& m = mat4Multiply(parent->getWorldMatrix(), child->getLocalMatrix());
                child->setWorldMatrix(m);
                nodeStack.push(child);
            }
        }
    }
};
