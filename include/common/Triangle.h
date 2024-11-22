#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "common/Shape.h"
#include "common/Point.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Triangle : public Shape {
private:
    Point p1, p2, p3; // Vertices of the triangle
    GLuint VBO, VAO;  // Vertex Buffer Object and Vertex Array Object

    void setupBuffer(); // Private method to initialize OpenGL buffers

public:
    Triangle(Point p1, Point p2, Point p3);
    void draw(GLuint transformLoc, GLuint colorLoc, glm::mat4 modelMatrix) override;
    ~Triangle();
};

#endif // TRIANGLE_H
