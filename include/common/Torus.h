#ifndef TORUS_H
#define TORUS_H

#include "glad/glad.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "common/shader.h"

#define PI 3.1416

class Torus {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    Torus(float innerRadius = .5f, float outerRadius = 1.5f, int sectorCount = 18, int stackCount = 9,
          glm::vec3 amb = glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3 diff = glm::vec3(0.8f, 0.8f, 0.8f),
          glm::vec3 spec = glm::vec3(1.0f, 1.0f, 1.0f), float shiny = 32.0f)
        : verticesStride(24) {
        set(innerRadius, outerRadius, sectorCount, stackCount, amb, diff, spec, shiny);
        
        // this->sectorCount = 18;
        // this->stackCount = 9;
        buildCoordinatesAndIndices();
        buildVertices();

        glGenVertexArrays(1, &torusVAO);
        glBindVertexArray(torusVAO);

        unsigned int torusVBO;
        glGenBuffers(1, &torusVBO);
        glBindBuffer(GL_ARRAY_BUFFER, torusVBO);
        glBufferData(GL_ARRAY_BUFFER, getVertexSize(), getVertices(), GL_STATIC_DRAW);

        unsigned int torusEBO;
        glGenBuffers(1, &torusEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torusEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, getIndexSize(), getIndices(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, verticesStride, (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, verticesStride, (void*)(sizeof(float) * 3));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    ~Torus() {}

    void drawTorus(Shader& shader, glm::mat4 model) const {
        shader.use();
        shader.setVec3("material.ambient", ambient);
        shader.setVec3("material.diffuse", diffuse);
        shader.setVec3("material.specular", specular);
        shader.setFloat("material.shininess", shininess);

        shader.setMat4("model", model);

        glBindVertexArray(torusVAO);
        glDrawElements(GL_TRIANGLES, getIndexCount(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
    }

private:
    void set(float innerR, float outerR, int sectors, int stacks, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float shiny) {
        innerRadius = innerR;
        outerRadius = outerR;
        sectorCount = (sectors >= 3) ? sectors : 3;
        stackCount = (stacks >= 2) ? stacks : 2;
        ambient = amb;
        diffuse = diff;
        specular = spec;
        shininess = shiny;
    }

    
    void buildCoordinatesAndIndices() {
        //std::vector<float> tempVertixes;
        float sectorStep = 2 * PI / sectorCount;
        float stackStep = 2 * PI / stackCount;

        for (int i = 0; i <= stackCount; ++i) {
            float stackAngle = i * stackStep;
            float cosStack = cos(stackAngle);
            float sinStack = sin(stackAngle);

            for (int j = 0; j <= sectorCount; ++j) {
                float sectorAngle = j * sectorStep;
                float cosSector = cos(sectorAngle);
                float sinSector = sin(sectorAngle);

                // Compute the vertex position
                float x = (outerRadius + innerRadius * cosStack) * cosSector;
                float y = (outerRadius + innerRadius * cosStack) * sinSector;
                float z = innerRadius * sinStack;

                // Debug: Check calculated values
                //std::cout << "Vertex: (" << x << ", " << y << ", " << z << ")" << std::endl;

                // vertixes.push_back(x);
                // vertixes.push_back(y);
                // vertixes.push_back(z);

                coordinates.push_back(x);
                coordinates.push_back(y);
                coordinates.push_back(z);


                // Compute the normals
                float nx = cosStack * cosSector;
                float ny = cosStack * sinSector;
                float nz = sinStack;
                normals.push_back(nx);
                normals.push_back(ny);
                normals.push_back(nz);

                // Generate indices
                if (i != stackCount && j != sectorCount) {
                    int cur = i * (sectorCount + 1) + j;
                    int next = (i + 1) * (sectorCount + 1) + j;

                    indices.push_back(cur);
                    indices.push_back(next);
                    indices.push_back(cur + 1);

                    indices.push_back(cur + 1);
                    indices.push_back(next);
                    indices.push_back(next + 1);
                }
            }
        }
        //vertixes.insert(vertixes.end(), tempVertixes.begin(), tempVertixes.end());
    }

    void buildVertices() {
        for (size_t i = 0; i < coordinates.size(); i += 3) {
            vertixes.push_back(coordinates[i]);
            vertixes.push_back(coordinates[i + 1]);
            vertixes.push_back(coordinates[i + 2]);
            vertixes.push_back(coordinates[i]);
            vertixes.push_back(coordinates[i + 1]);
            vertixes.push_back(coordinates[i + 2]);
        }
    }

    unsigned int getVertexCount() const { return vertixes.size() / 3; }
    unsigned int getVertexSize() const { return vertixes.size() * sizeof(float); }
    const float* getVertices() const { return vertixes.data(); }
    unsigned int getIndexSize() const { return indices.size() * sizeof(unsigned int); }
    const unsigned int* getIndices() const { return indices.data(); }
    unsigned int getIndexCount() const { return indices.size(); }

    unsigned int torusVAO;
    float innerRadius, outerRadius;
    int sectorCount, stackCount;
    std::vector<float> vertixes;
    std::vector<float> coordinates;
    std::vector<float> normals;
    std::vector<unsigned int> indices;
    int verticesStride;
};

#endif /* TORUS_H */
