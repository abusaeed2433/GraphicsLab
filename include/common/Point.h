#pragma once
#include "common/Shape.h"
#include "common/Point.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Point {
public:
    float x, y, z;

    // Constructor
    Point(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {
        
    }

};
