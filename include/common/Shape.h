#ifndef SHAPE_H
#define SHAPE_H

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "common/Point.h"

class Shape {
public:
    Point color;
    
    Shape(Point color) : color(color) {}
    virtual void draw(GLuint transformLoc, GLuint colorLoc, glm::mat4 modelMatrix) = 0;
    virtual ~Shape() {}
};

#endif