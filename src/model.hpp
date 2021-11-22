#pragma once

#include "glad.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <unordered_map>
#include <stack>
#include <string>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include "gmath.hpp"

#define MODEL_BONE_INFLUENCE_MAX 4

inline aiMatrix4x4 myMat4ToAssimpMat4(const Matrix4& my)
{
    aiMatrix4x4 m(
        my.m11, my.m21, my.m31, my.m41,
        my.m12, my.m22, my.m32, my.m42,
        my.m13, my.m23, my.m33, my.m43,
        my.m14, my.m24, my.m34, my.m44);
    return m;
}

struct Bone {
    uint8_t parent;
    aiMatrix4x4 offsetMatrix;
    aiMatrix4x4 localMatrix;
    aiMatrix4x4 globalMatrix;
};

struct AnimKey {
    aiVector3D location;
    aiQuaternion rotation;
    aiVector3D scale;
};

struct AnimKeyFrame {
    std::vector<AnimKey> keyPerBone;
    double timeStamp;
};

struct AnimAction {
    std::vector<AnimKeyFrame> keyframes;
    double duration;
};

struct Animation {
    std::unordered_map<std::string, AnimAction> actions;
    const AnimAction* currentAction;
    double elapsedTime;
    double prevTime;

    Animation()
    {
        currentAction = nullptr;
        elapsedTime = 0.0;
    }

    void setCurrentAction(std::string name)
    {
        assert(actions.size());
        assert(actions.count(name) > 0);
        currentAction = &actions[name];
        elapsedTime = 0.0F;
    }

    void updateAnimation(double dt, std::vector<Bone>& lerpBones, std::vector<aiMatrix4x4>& outBoneTable)
    {
        assert(currentAction);

        elapsedTime += dt;
        if (elapsedTime > currentAction->duration)
        {
            elapsedTime -= currentAction->duration;
        }

        const AnimKeyFrame* lastFrame = nullptr;
        const AnimKeyFrame* nextFrame = nullptr;
        for (uint32_t i = 0; i < currentAction->keyframes.size(); ++i)
        {
            const AnimKeyFrame* frame = &currentAction->keyframes[i];
            if (elapsedTime > frame->timeStamp)
            {
                lastFrame = frame;
                if (i + 1 < currentAction->keyframes.size())
                {
                    nextFrame = &currentAction->keyframes[i + 1];
                }
                else
                {
                    nextFrame = &currentAction->keyframes[0];
                }
            }
        }

        double midWayLength = elapsedTime - lastFrame->timeStamp;
        double frameDiff = nextFrame->timeStamp - lastFrame->timeStamp;
        float scaleFactor = float(midWayLength / frameDiff);

        const uint32_t boneFirst = 1;
        for (uint32_t i = boneFirst; i < lastFrame->keyPerBone.size(); ++i)
        {
            const AnimKey* k0 = &lastFrame->keyPerBone[i];
            const AnimKey* k1 = &nextFrame->keyPerBone[i];
            aiVector3D location = k0->location + (k1->location - k0->location) * scaleFactor;
            aiQuaternion rotation;
            aiQuaternion::Interpolate(rotation, k0->rotation, k1->rotation, scaleFactor);
            aiVector3D scaling = k0->scale + (k1->scale - k0->scale) * scaleFactor;
            lerpBones[i].localMatrix = aiMatrix4x4(scaling, rotation, location);
        }

        for (uint32_t i = boneFirst; i < lerpBones.size(); ++i)
        {
            Bone* bone = &lerpBones[i];
            aiMatrix4x4 parentGlobalTransform = aiMatrix4x4();
            if (bone->parent > 0)
            {
                parentGlobalTransform = lerpBones[bone->parent].globalMatrix;
            }

            bone->globalMatrix = parentGlobalTransform * bone->localMatrix;
            outBoneTable[i] = bone->globalMatrix * bone->offsetMatrix;
        }
    }
};

struct VertexWeight {
    uint8_t boneIndices[MODEL_BONE_INFLUENCE_MAX];
    float weights[MODEL_BONE_INFLUENCE_MAX];

    VertexWeight()
    {
        for (int i = 0; i < MODEL_BONE_INFLUENCE_MAX; ++i)
        {
            boneIndices[i] = 0;
            weights[i] = 0.0F;
        }
    }
};

struct Mesh {
    std::vector<float> positions;
    std::vector<VertexWeight> weights;
    std::vector<uint32_t> indices;
};

struct Model {
    static Assimp::Importer importer;

    const aiScene* scene = nullptr;
    std::vector<Bone> boneHierarchy;
    std::unordered_map<std::string, uint8_t> boneIndexMap;
    Animation animation;
    std::vector<aiMatrix4x4> boneTable;
    std::vector<Mesh> baseMeshes;
    std::vector<Mesh> animatedMeshes;
    std::vector<Mesh> displayMeshes;

    void load(const char* path)
    {
        assert(path);

        scene = importer.ReadFile(path, 0);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode || !scene->HasMeshes())
        {
            printf("ERROR::ASSIMP => %s\n", importer.GetErrorString());
            abort();
        }

        processNode(scene->mRootNode);
        processAnimationNode();

        boneTable.resize(boneHierarchy.size());
        animatedMeshes = baseMeshes;
        displayMeshes = baseMeshes;

        scene = nullptr;
        importer.FreeScene();
    }

    void processNode(aiNode* node)
    {
        for (uint32_t i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            baseMeshes.push_back(processMesh(mesh));
        }
        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            processNode(node->mChildren[i]);
        }
    }

    Mesh processMesh(aiMesh* mesh)
    {
        Mesh polygon;
        for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
        {
            aiVector3D v = mesh->mVertices[i];
            polygon.positions.push_back(v.x);
            polygon.positions.push_back(v.y);
            polygon.positions.push_back(v.z);

            if (mesh->HasBones())
            {
                polygon.weights.push_back(VertexWeight());
            }
        }
        for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; ++j)
            {
                polygon.indices.push_back(face.mIndices[j]);
            }
        }
        if (mesh->HasBones())
        {
            processBone(mesh, polygon);
        }
        return polygon;
    }

    void processBone(aiMesh* mesh, Mesh& polygon)
    {
        std::unordered_map<std::string, const aiBone*> boneNames;

        for (uint32_t i = 0; i < mesh->mNumBones; ++i)
        {
            const aiBone* bone = mesh->mBones[i];
            boneNames[std::string(bone->mName.C_Str())] = bone;
        }

        const aiNode* rootBone = nullptr;
        std::stack<const aiNode*> nodeStack;
        nodeStack.push(scene->mRootNode);
        while (nodeStack.size())
        {
            const aiNode* node = nodeStack.top();
            nodeStack.pop();
            for (uint32_t i = 0; i < node->mNumChildren; ++i)
            {
                const aiNode* child = node->mChildren[i];
                if (boneNames.count(std::string(child->mName.C_Str())) > 0)
                {
                    rootBone = child;
                    goto foundRoot;
                }
                nodeStack.push(child);
            }
        }
        assert(rootBone);
    foundRoot:

        boneHierarchy.push_back(Bone());
        nodeStack = std::stack<const aiNode*>();
        nodeStack.push(rootBone);
        uint8_t boneCounter = 1;
        while (nodeStack.size())
        {
            const aiNode* node = nodeStack.top();
            nodeStack.pop();
            if (boneNames.count(std::string(node->mName.C_Str())) < 1)
            {
                continue;
            }
            const aiBone* bone = boneNames[std::string(node->mName.C_Str())];
            Bone b;
            b.parent = 0;
            b.offsetMatrix = bone->mOffsetMatrix;
            boneHierarchy.push_back(b);
            boneIndexMap[std::string(node->mName.C_Str())] = boneCounter;
            ++boneCounter;
            for (uint32_t i = 0; i < node->mNumChildren; ++i)
            {
                const aiNode* child = node->mChildren[i];
                nodeStack.push(child);
            }
        }

        nodeStack = std::stack<const aiNode*>();
        nodeStack.push(rootBone);
        while (nodeStack.size())
        {
            const aiNode* node = nodeStack.top();
            nodeStack.pop();
            if (boneNames.count(std::string(node->mName.C_Str())) < 1)
            {
                continue;
            }
            else
            {
                if (rootBone->mName != node->mName)
                {
                    uint8_t parent = boneIndexMap[std::string(node->mParent->mName.C_Str())];
                    uint8_t current = boneIndexMap[std::string(node->mName.C_Str())];
                    boneHierarchy[current].parent = parent;
                }
            }

            for (uint32_t i = 0; i < node->mNumChildren; ++i)
            {
                const aiNode* child = node->mChildren[i];
                nodeStack.push(child);
            }
        }

        for (uint32_t i = 0; i < mesh->mNumBones; ++i)
        {
            const aiBone* bone = mesh->mBones[i];
            uint8_t idx = boneIndexMap[std::string(bone->mName.C_Str())];
            for (uint32_t j = 0; j < bone->mNumWeights; ++j)
            {
                aiVertexWeight w = bone->mWeights[j];
                for (uint32_t k = 0; k < MODEL_BONE_INFLUENCE_MAX; ++k)
                {
                    if (w.mWeight > 0.0 && polygon.weights[w.mVertexId].boneIndices[k] == 0)
                    {
                        polygon.weights[w.mVertexId].boneIndices[k] = idx;
                        polygon.weights[w.mVertexId].weights[k] = w.mWeight;
                        break;
                    }
                }
            }
        }
    }

    void processAnimationNode()
    {
        for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
        {
            const aiAnimation* animAction = scene->mAnimations[i];
            AnimAction action;
            action.duration = scene->mAnimations[i]->mDuration / 1000.0;
            for (uint32_t j = 0; j < animAction->mNumChannels; ++j)
            {
                const aiNodeAnim* animChannel = animAction->mChannels[j];
                if (boneIndexMap.count(std::string(animChannel->mNodeName.C_Str())) == 0)
                {
                    continue;
                }
                assert(animChannel->mNumPositionKeys == animChannel->mNumRotationKeys && animChannel->mNumPositionKeys == animChannel->mNumScalingKeys);
                action.keyframes.resize(animChannel->mNumPositionKeys);
                uint8_t boneID = boneIndexMap[std::string(animChannel->mNodeName.C_Str())];
                for (uint32_t k = 0; k < animChannel->mNumPositionKeys; ++k)
                {
                    aiVectorKey keyPos = animChannel->mPositionKeys[k];
                    aiQuatKey keyRot = animChannel->mRotationKeys[k];
                    aiVectorKey keyScale = animChannel->mScalingKeys[k];
                    action.keyframes[k].keyPerBone.resize(boneHierarchy.size());
                    action.keyframes[k].keyPerBone[boneID].location = keyPos.mValue;
                    action.keyframes[k].keyPerBone[boneID].rotation = keyRot.mValue;
                    action.keyframes[k].keyPerBone[boneID].scale = keyScale.mValue;
                    action.keyframes[k].timeStamp = keyPos.mTime / 1000.0;
                }
            }
            animation.actions[std::string(animAction->mName.C_Str())] = action;
        }
    }

    void updateAnimation(double dt)
    {
        animation.updateAnimation(dt, boneHierarchy, boneTable);
        for (size_t i = 0; i < baseMeshes.size(); ++i)
        {
            const std::vector<float>& pos = baseMeshes[i].positions;
            const std::vector<VertexWeight>& vertexWeights = baseMeshes[i].weights;
            std::vector<float>& outPos = animatedMeshes[i].positions;
            for (size_t idx = 0; idx < vertexWeights.size(); ++idx)
            {
                aiVector3D v;
                v.x = pos[idx * 3 + 0];
                v.y = pos[idx * 3 + 1];
                v.z = pos[idx * 3 + 2];
                aiVector3D totalPosition;
                for (int k = 0; k < MODEL_BONE_INFLUENCE_MAX; ++k)
                {
                    //if (vertexWeights[idx].boneIndices[k] == 0) { break; }
                    //if (!(vertexWeights[idx].weights[k] > 0.0F)) { break; }
                    aiVector3D localPosition = v;
                    localPosition *= boneTable[vertexWeights[idx].boneIndices[k]];
                    localPosition *= vertexWeights[idx].weights[k];
                    totalPosition += localPosition;
                }
                outPos[idx * 3 + 0] = totalPosition.x;
                outPos[idx * 3 + 1] = totalPosition.y;
                outPos[idx * 3 + 2] = totalPosition.z;
            }
        }
    }

    void updateMesh(const Matrix4& mtx)
    {
        aiMatrix4x4 m = myMat4ToAssimpMat4(mtx);
        const std::vector<Mesh>& meshes = animatedMeshes;
        for (size_t i = 0; i < meshes.size(); ++i)
        {
            const std::vector<float>& pos = meshes[i].positions;
            std::vector<float>& outPos = displayMeshes[i].positions;
            for (size_t idx = 0; idx < pos.size(); idx += 3)
            {
                aiVector3D v;
                //Vector4 v;
                v.x = pos[idx + 0];
                v.y = pos[idx + 1];
                v.z = pos[idx + 2];
                //v.w = 1.0F;
                v *= m;
                //v = vec4Transform(v, mtx);
                outPos[idx + 0] = v.x;
                outPos[idx + 1] = v.y;
                outPos[idx + 2] = v.z;
            }
        }
    }

    void draw()
    {
        glBegin(GL_TRIANGLES);
        for (const Mesh& mesh : displayMeshes)
        {
            for (uint32_t idx : mesh.indices)
            {
                float x = mesh.positions[3 * idx + 0];
                float y = mesh.positions[3 * idx + 1];
                float z = mesh.positions[3 * idx + 2];
                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }
};
