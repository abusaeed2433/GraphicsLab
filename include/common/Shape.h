#ifndef SHAPE_H
#define SHAPE_H

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

class Shape {
public:
    virtual void draw(GLuint transformLoc, GLuint colorLoc, glm::mat4 modelMatrix) = 0;
    virtual ~Shape() {}
};

#endif