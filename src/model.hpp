#pragma once

#include "glad.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include "gmath.hpp"

struct Mesh {
    std::vector<float> positions;
    std::vector<uint32_t> indices;
};

struct Model {

    const aiScene* scene = nullptr;
    std::vector<Mesh> baseMeshes;
    std::vector<Mesh> displayMeshes;

    void load(const char* path)
    {
        assert(path);

        Assimp::Importer importer;
        scene = importer.ReadFile(path, 0);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            printf("ERROR::ASSIMP => %s\n", importer.GetErrorString());
            abort();
        }

        processNode(scene->mRootNode);

        displayMeshes = baseMeshes;
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
        }
        for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; ++j)
            {
                polygon.indices.push_back(face.mIndices[j]);
            }
        }
        return polygon;
    }

    void updateMesh(const Matrix4& mtx) {
        for (size_t i = 0; i < baseMeshes.size(); ++i) {
            const std::vector<float>& pos = baseMeshes[i].positions;
            std::vector<float>& outPos = displayMeshes[i].positions;
            for (size_t idx = 0; idx < pos.size(); idx += 3) {
                Vector4 v;
                v.x = pos[idx + 0];
                v.y = pos[idx + 1];
                v.z = pos[idx + 2];
                v.w = 1.0F;
                v = vec4Transform(v, mtx);
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
