#pragma once

#include "../gmath.hpp"
#include <vector>
#include <stack>
#include <cassert>
#include <cstdio>

struct BaseNode {
private:
    Vector3 position = vec3Zero();
    Matrix4 localTransform = mat4Identity();
    Matrix4 worldTransform = mat4Identity();

    void updateLocalTransform()
    {
        localTransform = mat4CreateTranslation(position);
    }

public:
    Vector3 getPosition()
    {
        return position;
    }

    void setPosition(const Vector3& pos)
    {
        position = pos;
        updateLocalTransform();
    }

    const Matrix4& getLocalTransform()
    {
        return localTransform;
    }

    const Matrix4& getWorldTransform()
    {
        return worldTransform;
    }

    void setWorldTransform(const Matrix4& m)
    {
        worldTransform = m;
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
        root->setWorldTransform(root->getLocalTransform());
        while (nodeStack.size())
        {
            GameObject* parent = nodeStack.top();
            nodeStack.pop();
            for (GameObject* child : parent->getObjects())
            {
                const Matrix4& m = mat4Multiply(parent->getWorldTransform(), child->getLocalTransform());
                child->setWorldTransform(m);
                nodeStack.push(child);
            }
        }
    }
};
