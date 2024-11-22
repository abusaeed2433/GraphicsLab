#include "Triangle.h"

Triangle::Triangle(Point p1, Point p2, Point p3) : p1(p1), p2(p2), p3(p3) {
    setupBuffer();
}

void Triangle::setupBuffer() {
    GLfloat vertices[] = {
        p1.x, p1.y, p1.z,
        p2.x, p2.y, p2.z,
        p3.x, p3.y, p3.z,
    };

    // Generate and bind VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate and bind VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Define vertex attribute layout
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Unbind VAO
    glBindVertexArray(0);
}

void Triangle::draw(GLuint transformLoc, GLuint colorLoc, glm::mat4 modelMatrix) {
    // Send transformation matrix to the shader
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Set triangle color
    glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);

    // Bind VAO and draw the triangle
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0); // Unbind VAO to prevent side effects
}

Triangle::~Triangle() {
    // Cleanup OpenGL resources
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}
