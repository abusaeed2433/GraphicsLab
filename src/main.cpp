#define GLFW_DLL

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "common/MyClasses.cpp"
#include "common/Shader.h"
#include "common/BasicCamera.h"

#include <unordered_map>
#include <memory>

#include <set>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

using namespace std;

#define PI 3.14159265359

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void drawCube(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans, float posX = 0.0, float posY = 0.0, float posz = 0.0, float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0,float scX = 1.0, float scY = 1.0, float scZ=1.0);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// modelling transform
float rotateAngle_X = 45.0;
float rotateAngle_Y = 45.0;
float rotateAngle_Z = 45.0;
float rotateAxis_X = 0.0;
float rotateAxis_Y = 0.0;
float rotateAxis_Z = 1.0;
float translate_X = 0.0;
float translate_Y = 0.0;
float translate_Z = 0.0;
float scale_X = 1.0;
float scale_Y = 1.0;
float scale_Z = 1.0;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float eyeX = 0.0, eyeY = 0.0, eyeZ = 3.0;
float lookAtX = 0.0, lookAtY = 0.0, lookAtZ = 0.0;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
BasicCamera basicCamera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;


int initGlfw(GLFWwindow*& window){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LabOne", NULL, NULL);
    if (window == NULL){ cout << "Failed to create GLFW window" << endl; glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { cout << "Failed to initialize GLAD" << endl; return -1; }

    // build and compile our shader program
    return 0;
}

void safeTerminate(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
}

void initBinding(unsigned int &VAO, unsigned int &VBO, unsigned int &EBO, Shader &ourShader, float* cube_vertices, int verticesSize, unsigned int* cube_indices, int indicesSize){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
//
    glBufferData(GL_ARRAY_BUFFER, verticesSize, cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, cube_indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);
    ourShader.use();
}

std::vector<std::unique_ptr<Shape>> readShapes() {
    std::ifstream file(
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\points.txt"
    );

    if (!file.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return {};
    }

    std::vector<std::unique_ptr<Shape>> shapes;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line.substr(0, 6) == "#start"){// Start of a shape definition block
            printf("Reading shape\n");
            std::getline(file, line); // Read shape metadata line

            std::istringstream iss(line);
            int catId, id;
            std::string name;

            iss >> catId >> name >> id;

            if (catId == 0) { // Ignore block if id is 0
                while (line != "#end") std::getline(file, line);
                printf("Ignoring shape\n");
                continue;
            }

            // color
            float x, y, z;
            char ch;
            std::getline(file, line);
            std::istringstream pointStream(line);
            pointStream >> x >> ch >> y >> ch >> z;
            Point color(x, y, z);

            std::vector<Point> points;
            while (std::getline(file, line)) {
                if (line == "#end") break;                
                
                std::istringstream pointStream(line);
                pointStream >> x >> ch >> y >> ch >> z;
                points.emplace_back(x, y, z);
            }

            if (catId == 3) { // Triangle

                if (points.size() >= 3) { // Ensure there are enough points for a triangle
                    printf("Creating triangle\n");
                    shapes.push_back(std::make_unique<Triangle>(color,points[0], points[1], points[2]));
                }
            }
            else if(catId == 4){
                if (points.size() >= 4) { // Ensure there are enough points for a rectangle
                    shapes.push_back( std::make_unique<Rectangle>(color, points[0], points[1], points[2], points[3]) );
                }
            }
        }
    }

    return shapes;
}


int main()
{
    GLFWwindow* window = nullptr;
    if( initGlfw(window) ) return -1;

    glEnable(GL_DEPTH_TEST);
    
    Shader ourShader(
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\VertexShader.vs", 
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\FragmentShader.fs"
    );

    Shader constantShader(
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\VertexShader.vs", 
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\FragmentShaderV2.fs"
    );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cube_vertices[] = {
        0.0f, 0.0f, 0.0f, 0.3f, 0.8f, 0.5f,
        0.5f, 0.0f, 0.0f, 0.5f, 0.4f, 0.3f,
        0.5f, 0.5f, 0.0f, 0.2f, 0.7f, 0.3f,
        0.0f, 0.5f, 0.0f, 0.6f, 0.2f, 0.8f,
        0.0f, 0.0f, 0.5f, 0.8f, 0.3f, 0.6f,
        0.5f, 0.0f, 0.5f, 0.4f, 0.4f, 0.8f,
        0.5f, 0.5f, 0.5f, 0.2f, 0.3f, 0.6f,
        0.0f, 0.5f, 0.5f, 0.7f, 0.5f, 0.4f
    };
    unsigned int cube_indices[] = {
       0, 3, 2,
       2, 1, 0,

       1, 2, 6,
       6, 5, 1,

       5, 6, 7,
       7 ,4, 5,

       4, 7, 3,
       3, 0, 4,

       6, 2, 3,
       3, 7, 6,

       1, 5, 4,
       4, 0, 1
   };

    unsigned int VBO, VAO, EBO;
    initBinding(VAO, VBO, EBO, ourShader, cube_vertices, sizeof(cube_vertices), cube_indices, sizeof(cube_indices));

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::mat4 projection = glm::perspective(glm::radians(basicCamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::ortho(-2.0f, +2.0f, -1.5f, +1.5f, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        constantShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = basicCamera.createViewMatrix();
        ourShader.setMat4("view", view);
        constantShader.setMat4("view", view);



        // Modelling Transformation
        glm::mat4 identityMatrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        
        // drawing
        // drawCube(ourShader, VAO, identityMatrix, translate_X, translate_Y, translate_Z, rotateAngle_X, rotateAngle_Y, rotateAngle_Z, scale_X, scale_Y, scale_Z);

        // drawing
        // Floor
        drawCube(ourShader, VAO, identityMatrix, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 0.1f, 20.0f); // Room floor

        // Walls
        drawCube(ourShader, VAO, identityMatrix, 0.0f, 5.0f, -10.0f, 0.0f, 0.0f, 0.0f, 20.0f, 10.0f, 0.1f); // Back wall
        drawCube(ourShader, VAO, identityMatrix, -10.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 10.0f, 20.0f); // Left wall
        drawCube(ourShader, VAO, identityMatrix, 10.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 10.0f, 20.0f);  // Right wall
        drawCube(ourShader, VAO, identityMatrix, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 0.1f, 20.0f);  // Ceiling

        // Tables (4 rows, 2 columns)
        float tableWidth = 2.0f, tableHeight = 1.0f, tableDepth = 1.5f;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 2; ++col) {
                float x = (col * 5.0f) - 2.5f; // Offset for columns
                float z = (row * -3.0f) + 2.0f; // Offset for rows
                drawCube(ourShader, VAO, identityMatrix, x, 0.5f, z, 0.0f, 0.0f, 0.0f, tableWidth, tableHeight, tableDepth); // Table top
            }
        }

        // Chairs (placed with offset near tables)
        float chairWidth = 1.0f, chairHeight = 1.0f, chairDepth = 1.0f;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 2; ++col) {
                float x = (col * 5.0f) - 3.0f; // Offset for columns
                float z = (row * -3.0f) + 3.0f; // Offset for rows
                drawCube(ourShader, VAO, identityMatrix, x, 0.5f, z, 0.0f, 0.0f, 0.0f, chairWidth, chairHeight, chairDepth); // Chair seat
                drawCube(ourShader, VAO, identityMatrix, x, 1.5f, z - 0.5f, 0.0f, 0.0f, 0.0f, chairWidth, chairHeight, 0.2f); // Chair back
            }
        }

        // Teacher's Table
        drawCube(ourShader, VAO, identityMatrix, 0.0f, 0.5f, 8.0f, 0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 2.0f); // Teacher's table

        // Chair for Teacher
        drawCube(ourShader, VAO, identityMatrix, 0.0f, 0.5f, 6.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);   // Seat
        drawCube(ourShader, VAO, identityMatrix, 0.0f, 1.5f, 6.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.2f);   // Back

        // Blackboard
        drawCube(ourShader, VAO, identityMatrix, 0.0f, 5.0f, 9.5f, 0.0f, 0.0f, 0.0f, 8.0f, 4.0f, 0.1f); // Blackboard

        // drawing above
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    safeTerminate(VAO, VBO, EBO);
    return 0;
}


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        basicCamera.eye.z -= 0.1f; // Move forward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        basicCamera.eye.z += 0.1f; // Move backward
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        basicCamera.eye.x -= 0.1f; // Move left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        basicCamera.eye.x += 0.1f; // Move right

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// Track whether the mouse button is pressed
bool isMousePressed = false;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isMousePressed = true;
        } else if (action == GLFW_RELEASE) {
            isMousePressed = false;
        }
    }
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    static float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if (isMousePressed) {
        float xOffset = xpos - lastX;
        float yOffset = lastY - ypos;  // Inverted because y-coordinates go from top to bottom in window
        lastX = xpos;
        lastY = ypos;

        // Adjust sensitivity for smoother motion
        xOffset *= basicCamera.MouseSensitivity;
        yOffset *= basicCamera.MouseSensitivity;

        // Update yaw and pitch
        basicCamera.Yaw += xOffset;  // Left drag moves camera right
        basicCamera.Pitch += yOffset;  // Up drag moves camera down

        // Constrain pitch to prevent flipping
        if (basicCamera.Pitch > 89.0f)
            basicCamera.Pitch = 89.0f;
        if (basicCamera.Pitch < -89.0f)
            basicCamera.Pitch = -89.0f;

        // Calculate new camera direction
        glm::vec3 front;
        front.x = cos(glm::radians(basicCamera.Yaw)) * cos(glm::radians(basicCamera.Pitch));
        front.y = sin(glm::radians(basicCamera.Pitch));
        front.z = sin(glm::radians(basicCamera.Yaw)) * cos(glm::radians(basicCamera.Pitch));
        basicCamera.direction = glm::normalize(front);

        // Update the camera's lookAt point
        basicCamera.lookAt = basicCamera.eye + basicCamera.direction;
    }
}



// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    basicCamera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// The parentTrans parameter is here for hiererchical modeling,
// If you are confused with it's usage, then pass an identity matrix to it, and everything will be fine 
void drawCube(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans, float posX, float posY, float posZ, float rotX , float rotY, float rotZ, float scX, float scY, float scZ)
{
    shaderProgram.use();

    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered;
    translateMatrix = glm::translate(parentTrans, glm::vec3(posX, posY, posZ));
    rotateXMatrix = glm::rotate(translateMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(rotateXMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(rotateYMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
    modelCentered = glm::translate(model, glm::vec3(-0.25, -0.25, -0.25));

    shaderProgram.setMat4("model", modelCentered);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}
