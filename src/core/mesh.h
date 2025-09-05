#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
};

class Mesh {
public:
    GLuint VAO, VBO, EBO;
    glm::vec3 bmin, bmax;
    glm::vec3 color = glm::vec3(0.8f, 0.1f, 0.8f);

    Mesh(const string& path);

    int getIndexCount() const { return static_cast<int>(indices.size()); }
    
    void draw() const
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, getIndexCount(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    
    void initMesh(const aiMesh* mesh);
};

#endif // MESH_H