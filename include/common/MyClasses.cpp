#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Point {
public:
    float x, y, z;

    Point(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z){}
};

class Shape {
public:
    Point color;
    
    Shape(Point color) : color(color) {}
    virtual void draw(GLuint transformLoc, GLuint colorLoc, glm::mat4 modelMatrix) = 0;
    virtual ~Shape() {}
};

class Triangle : public Shape {
private:
    Point p1, p2, p3;
    GLuint VBO, VAO;

public:
    Triangle(Point color, Point p1, Point p2, Point p3) : Shape(color) {
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;

        setupBuffer();
    }

    void setupBuffer() {
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

    void draw(GLuint transformLoc, GLuint colorLoc, glm::mat4 modelMatrix) {
        // Send transformation matrix to the shader
        // printf("Drawing inside triangle draw\n");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        // Set triangle color
        glUniform3f(colorLoc, color.x, color.y, color.z);

        // Bind VAO and draw the triangle
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0); // Unbind VAO to prevent side effects
    }

    ~Triangle() {
        // Cleanup OpenGL resources
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

};

class Rectangle : public Shape {
private:
    Triangle triangle1, triangle2;
public:
    Rectangle(Point color, Point p1, Point p2, Point p3, Point p4) : Shape(color),
        triangle1(color, p1, p2, p3), triangle2(color, p1, p3, p4)
    {

    }

    void draw(GLuint transformLoc, GLuint colorLoc, glm::mat4 modelMatrix) {
        triangle1.draw(transformLoc, colorLoc, modelMatrix);
        triangle2.draw(transformLoc, colorLoc, modelMatrix);
    }
};
