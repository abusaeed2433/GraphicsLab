#ifndef sphere_h
#define sphere_h

#include "glad/glad.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "common/shader.h"

# define PI 3.1416

using namespace std;

const int MIN_SECTOR_COUNT = 3;
const int MIN_STACK_COUNT = 2;

class Cone
{
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    // ctor/dtor
    Cone(float radius = 1.0f, int sectorCount = 36, int stackCount = 18, glm::vec3 amb = glm::vec3(0.98, 0.847, 0.69), glm::vec3 diff = glm::vec3(0.98, 0.847, 0.69), glm::vec3 spec = glm::vec3(0.5f, 0.5f, 0.5f), float shiny = 32.0f) : verticesStride(24)
    {
        set(radius, sectorCount, stackCount, amb, diff, spec, shiny);
        buildCoordinatesAndIndices();
        buildVertices();

        glGenVertexArrays(1, &sphereVAO);
        glBindVertexArray(sphereVAO);

        // create VBO to copy vertex data to VBO
        unsigned int sphereVBO;
        glGenBuffers(1, &sphereVBO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);           // for vertex data
        glBufferData(GL_ARRAY_BUFFER,                   // target
            this->getVertexSize(), // data size, # of bytes
            this->getVertices(),   // ptr to vertex data
            GL_STATIC_DRAW);                   // usage

        // create EBO to copy index data
        unsigned int sphereEBO;
        glGenBuffers(1, &sphereEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);   // for index data
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,           // target
            this->getIndexSize(),             // data size, # of bytes
            this->getIndices(),               // ptr to index data
            GL_STATIC_DRAW);                   // usage

        // activate attrib arrays
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        // set attrib arrays with stride and offset
        int stride = this->getVerticesStride();     // should be 24 bytes
        glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, false, stride, (void*)(sizeof(float) * 3));

        // unbind VAO and VBOs
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    ~Cone() {}

    // getters/setters

    void set(float radius, int sectors, int stacks, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float shiny)
    {
        if (radius > 0)
            this->radius = radius;
        this->sectorCount = sectors;
        if (sectors < MIN_SECTOR_COUNT)
            this->sectorCount = MIN_SECTOR_COUNT;
        this->stackCount = stacks;
        if (stacks < MIN_STACK_COUNT)
            this->stackCount = MIN_STACK_COUNT;
        this->ambient = amb;
        this->diffuse = diff;
        this->specular = spec;
        this->shininess = shiny;
    }

    void setRadius(float radius)
    {
        if (radius != this->radius)
            set(radius, sectorCount, stackCount, ambient, diffuse, specular, shininess);
    }

    void setSectorCount(int sectors)
    {
        if (sectors != this->sectorCount)
            set(radius, sectors, stackCount, ambient, diffuse, specular, shininess);
    }

    void setStackCount(int stacks)
    {
        if (stacks != this->stackCount)
            set(radius, sectorCount, stacks, ambient, diffuse, specular, shininess);
    }

    // for interleaved vertices
    unsigned int getVertexCount() const
    {
        return (unsigned int)coordinates.size() / 3;     // # of vertices
    }

    unsigned int getVertexSize() const
    {
        return (unsigned int)vertices.size() * sizeof(float);  // # of bytes
    }

    int getVerticesStride() const
    {
        return verticesStride;   // should be 24 bytes
    }
    const float* getVertices() const
    {
        return vertices.data();
    }

    unsigned int getIndexSize() const
    {
        return (unsigned int)indices.size() * sizeof(unsigned int);
    }

    const unsigned int* getIndices() const
    {
        return indices.data();
    }

    unsigned int getIndexCount() const
    {
        return (unsigned int)indices.size();
    }

    // draw in VertexArray mode
    void drawCone(Shader& lightingShader, glm::mat4 model) const      // draw surface
    {
        lightingShader.use();

        lightingShader.setVec3("material.ambient", this->ambient);
        lightingShader.setVec3("material.diffuse", this->diffuse);
        lightingShader.setVec3("material.specular", this->specular);
        lightingShader.setFloat("material.shininess", this->shininess);

        lightingShader.setMat4("model", model);

        // draw a sphere with VAO
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES,                    // primitive type
            this->getIndexCount(),          // # of indices
            GL_UNSIGNED_INT,                 // data type
            (void*)0);                       // offset to indices

        // unbind VAO
        glBindVertexArray(0);
    }

private:
    // member functions
    void buildCoordinatesAndIndices()
    {
        float height = 2.0;
        float sectorStep = 2 * PI / sectorCount;  // Step size for sector
        float slantHeight = sqrtf(radius * radius + height * height); // Slant height of the cone
        float nx, ny, nz;

        // Base circle vertices and normals
        for (int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = j * sectorStep;
            float x = radius * cosf(sectorAngle);
            float z = radius * sinf(sectorAngle);

            // Base vertex position
            coordinates.push_back(x);
            coordinates.push_back(0.0f);
            coordinates.push_back(z);

            // Normal pointing down for base
            normals.push_back(0.0f);
            normals.push_back(-1.0f);
            normals.push_back(0.0f);
        }

        // Apex vertex
        coordinates.push_back(0.0f);
        coordinates.push_back(height);
        coordinates.push_back(0.0f);

        normals.push_back(0.0f);
        normals.push_back(1.0f);
        normals.push_back(0.0f);

        int apexIndex = coordinates.size() / 3 - 1;

        // Side surface normals and indices
        for (int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = j * sectorStep;
            float x = radius * cosf(sectorAngle);
            float z = radius * sinf(sectorAngle);

            // Side normal vector components
            nx = x / slantHeight;
            ny = radius / slantHeight;  // Slope of cone's side
            nz = z / slantHeight;

            normals[j * 3 + 0] = nx;
            normals[j * 3 + 1] = ny;
            normals[j * 3 + 2] = nz;
        }

        // Side indices
        for (int j = 0; j < sectorCount; ++j)
        {
            indices.push_back(j);          // Base current vertex
            indices.push_back(apexIndex);  // Apex vertex
            indices.push_back(j + 1);      // Base next vertex
        }

        //// Bottom center vertex
        //coordinates.push_back(0.0f);
        //coordinates.push_back(0.0f);
        //coordinates.push_back(0.0f);

        //normals.push_back(0.0f);
        //normals.push_back(-1.0f);
        //normals.push_back(0.0f);

        //int bottomCenterIndex = coordinates.size() / 3 - 1;

        //// Bottom cap indices
        //for (int j = 0; j < sectorCount; ++j)
        //{
        //    indices.push_back(bottomCenterIndex);
        //    indices.push_back(j + 1);
        //    indices.push_back(j);
        //}
    }

    void buildVertices()
    {
        size_t i, j;
        size_t count = coordinates.size();
        for (i = 0, j = 0; i < count; i += 3, j += 2)
        {
            vertices.push_back(coordinates[i]);
            vertices.push_back(coordinates[i + 1]);
            vertices.push_back(coordinates[i + 2]);

            vertices.push_back(normals[i]);
            vertices.push_back(normals[i + 1]);
            vertices.push_back(normals[i + 2]);
        }
    }

    vector<float> computeFaceNormal(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
    {
        const float EPSILON = 0.000001f;

        vector<float> normal(3, 0.0f);     // default return value (0,0,0)
        float nx, ny, nz;

        // find 2 edge vectors: v1-v2, v1-v3
        float ex1 = x2 - x1;
        float ey1 = y2 - y1;
        float ez1 = z2 - z1;
        float ex2 = x3 - x1;
        float ey2 = y3 - y1;
        float ez2 = z3 - z1;

        // cross product: e1 x e2
        nx = ey1 * ez2 - ez1 * ey2;
        ny = ez1 * ex2 - ex1 * ez2;
        nz = ex1 * ey2 - ey1 * ex2;

        // normalize only if the length is > 0
        float length = sqrtf(nx * nx + ny * ny + nz * nz);
        if (length > EPSILON)
        {
            // normalize
            float lengthInv = 1.0f / length;
            normal[0] = nx * lengthInv;
            normal[1] = ny * lengthInv;
            normal[2] = nz * lengthInv;
        }

        return normal;
    }

    // memeber vars
    unsigned int sphereVAO;
    float radius;
    int sectorCount;                        // longitude, # of slices
    int stackCount;                         // latitude, # of stacks
    vector<float> vertices;
    vector<float> normals;
    vector<unsigned int> indices;
    vector<float> coordinates;
    int verticesStride;                 // # of bytes to hop to the next vertex (should be 24 bytes)

};
#endif /* sphere_h */


