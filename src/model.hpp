#pragma once

#include "glad.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <cassert>
#include <cstdint>
#include <cstdlib>

struct Mesh {
    std::vector<float> positions;
    std::vector<uint32_t> indices;
};

struct Model {

    const aiScene* scene = nullptr;
    std::vector<Mesh> polygonMeshes;

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
    }

    void processNode(aiNode* node)
    {
        for (uint32_t i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            polygonMeshes.push_back(processMesh(mesh));
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

    void draw()
    {
        glBegin(GL_TRIANGLES);
        for (const Mesh& mesh : polygonMeshes)
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
