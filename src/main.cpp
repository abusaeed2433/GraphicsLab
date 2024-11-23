#define GLFW_DLL

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "common/MyClasses.cpp"
#include "common/Shader.h"
#include "common/BasicCamera.h"
#include "common/Camera.h"

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

int drawAll(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans);

void drawCube(
    Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans, 
    float posX = 0.0, float posY = 0.0, float posz = 0.0, 
    float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0,
    float scX = 1.0, float scY = 1.0, float scZ=1.0,
    float r = 0.0, float g = 0.0, float b = 0.0);


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

float eyeX = 2.0f, eyeY = 0.0f, eyeZ = 13.5f;
float lookAtX = 2.0f, lookAtY = 1.5f, lookAtZ = 4.75f;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 eye = glm::vec3(eyeX, eyeY, eyeZ);
glm::vec3 lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);
Camera camera(glm::vec3(eyeX, eyeY, eyeZ));

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

bool on = false;

//birds eye
bool birdEye = false;
glm::vec3 cameraPos(-2.0f, 5.0f, 13.0f); 
glm::vec3 target(-2.0f, 0.0f, 5.5f);   
float birdEyeSpeed = 1.0f;

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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    
    glBufferData(GL_ARRAY_BUFFER, verticesSize, cube_vertices, GL_STATIC_DRAW);
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
        1.0f, 0.0f, 0.0f, 0.5f, 0.4f, 0.3f,
        1.0f, 1.0f, 0.0f, 0.2f, 0.7f, 0.3f,
        0.0f, 1.0f, 0.0f, 0.6f, 0.2f, 0.8f,
        0.0f, 0.0f, 1.0f, 0.8f, 0.3f, 0.6f,
        1.0f, 0.0f, 1.0f, 0.4f, 0.4f, 0.8f,
        1.0f, 1.0f, 1.0f, 0.2f, 0.3f, 0.6f,
        0.0f, 1.0f, 1.0f, 0.7f, 0.5f, 0.4f
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

    float r = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::mat4 projection = glm::perspective(glm::radians(basic_camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        constantShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view;

        if (birdEye) {
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            view = glm::lookAt(cameraPos, target, up);
        }
        else {
            view = basic_camera.createViewMatrix();
        }

        //glm::mat4 view = basic_camera.createViewMatrix();
        ourShader.setMat4("view", view);
        //constantShader.setMat4("view", view);
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered, translateMatrixprev;
        translateMatrix = identityMatrix;
    
        // drawing
        drawAll(ourShader, VAO, identityMatrix);

        if (on)
        {
            r += 1;
        }
        else
        {
            r = 0.0f;
        }

        // drawing above
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    safeTerminate(VAO, VBO, EBO);
    return 0;
}

int drawAll(Shader ourShader, unsigned int VAO, glm::mat4 identityMatrix){

        // drawCube(ourShader, VAO, identityMatrix, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 0.1f, 20.0f);
        // drawCube(ourShader, VAO, identityMatrix, )
        // translateMatrix *= glm::translate(identityMatrix, glm::vec3(-5.0f, -1.05f, -4.0));
        // scaleMatrix = glm::scale(identityMatrix, glm::vec3(13.0f, 0.1, 30.0));
        // model = translateMatrix* scaleMatrix;
        // ourShader.setMat4("model", model);
        // ourShader.setVec4("color", glm::vec4(0.65, 0.70, 0.73, 1.0));
        // glBindVertexArray(VAO);
        // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // floor
    drawCube(ourShader, VAO, identityMatrix, 0,0,0, 0,0,0, 10,.1,10, 0.65, 0.70, 0.73);
    
    
    // right wall
    drawCube(ourShader, VAO, identityMatrix, 0,2, -2.5, 0,0,0,  10,8,.1, 128/255.0, 128/255.0, 128/255.0);
    
    return -1;
    // left wall
    drawCube(ourShader, VAO, identityMatrix, -2.5,2, 0, 0,0,0,  .1,8,10, 255/255.0, 200/255.0, 220/255.0);

    // right shelf
    drawCube(ourShader, VAO, identityMatrix, -.5, 2.5, -2, 0,0,0, 8, .1, 2, 227/255.0, 193/255.0, 166/255.0);
    // left shelf
    drawCube(ourShader, VAO, identityMatrix, -2, 2.5, .5, 0,0,0, 2, .1, 9, 227/255.0, 193/255.0, 166/255.0);
    
    // left wall shelf
    int total = 7;
    for(int i=0; i<total; i++){
        float gap = (1 / 5.0);
        float width = 1.0;

        drawCube(
            ourShader, VAO, identityMatrix,
            -2.25, 4.5, -2+(i * width + i*gap), 
            0,0,0,
            .8,1.5, width, 
            241/255.0, 112/255.0, 4/255.0
        );
        
        if(i == total-1) continue;

        drawCube(
            ourShader, VAO, identityMatrix,
            -2.25, 4.5, -2+(i * width + i*gap) + width - gap,
            0,0,0,
            .8,1.5, .2,
            162/255.0, 52/255.0, 0/255.0
        );
    }

    // right wall shelf
    drawCube( ourShader, VAO, identityMatrix, -1.25, 4.5, -2.25, 0,0,0, .8,1.5, 1,  241/255.0, 112/255.0, 4/255.0 );
    // right wall shelf white
    drawCube( ourShader, VAO, identityMatrix, -1.25, 4.5, -1.5, 0,0,0, .7,1.3, .1,  212/255.0, 164/255.0, 141/255.0 );
    
    // right wall window?
    drawCube( ourShader, VAO, identityMatrix, 1.5, 4.5, -2.25, 0,0,0, 3,1.5, .1,  241/255.0, 112/255.0, 4/255.0 );
    // right wall window? white
    drawCube( ourShader, VAO, identityMatrix, 1.25, 4.62, -2.0, 0,0,0, 1.32,1.3, .1,  212/255.0, 164/255.0, 141/255.0 );
    drawCube( ourShader, VAO, identityMatrix, 2.6, 4.62, -2.0, 0,0,0, 1.32,1.3, .1,  212/255.0, 164/255.0, 141/255.0 );

    // table
    drawCube( ourShader, VAO, identityMatrix, 5, 3, 6, 0,0,0, 3, .1, 2,  25/255.0, 21/255.0, 18/255.0 );
    drawCube( ourShader, VAO, identityMatrix, 4, 3, 6, 0,0,0, .1,3,.1,  236/255.0, 28/255.0, 36/255.0 );

    // lower shelf left
    total = 8;
    for(int i=0; i<total; i++){
        float gap = (1 / 5.0);
        float width = 1;

        drawCube( ourShader, VAO, identityMatrix, -2, 0.5, -2+(i * width + i*gap), 
            0,0,0, 2,2.5, width,  240/255.0, 108/255.0, 36/255.0
        );
        
        if(i == total-1) continue;
        drawCube( ourShader, VAO, identityMatrix, -2, 0.5, -2+(i * width + i*gap) + width - gap,
            0,0,0, 2,2.5, .2, 162/255.0, 52/255.0, 0/255.0
        );
    }

    // right wall shelf bottom
    total = 5;
    for(int i=0; i<total; i++){
        float gap = (1 / 5.0);
        float width = 1;
        
        drawCube( ourShader, VAO, identityMatrix,  -.25 + (i*width + i*gap), 0.75, -2, 
            0,0,0, 1, 2.3, 2,  240/255.0, 108/255.0, 36/255.0
        );
        
        if(i == total-1) continue;
        drawCube( ourShader, VAO, identityMatrix, -.25+(i * width + i*gap + width - gap), .75, -2,
            0,0,0, .2,2.3, 2, 162/255.0, 52/255.0, 0/255.0
        );
    }

    // fridge
    drawCube( ourShader, VAO, identityMatrix,  6, 1.25, -2, 
        0,0,0, 2, 4.5, 2,  178/255.0, 157/255.0, 136/255.0
    );
    drawCube( ourShader, VAO, identityMatrix, 5.8, 1.25, -.5, 0,0,0, .95,4.5, .1,  63/255.0, 72/255.0, 204/255.0 );
    drawCube( ourShader, VAO, identityMatrix, 6.8, 1.25, -.5, 0,0,0, .95,4.5, .1,  63/255.0, 72/255.0, 204/255.0 );

    drawCube( ourShader, VAO, identityMatrix, 6.9, 2, -.4, 0,0,0, .1, 1, .1,  255/255.0, 255/255.0, 255/255.0 );

    return 0;

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
            firstMouse = true;
            isMousePressed = true;
        } else if (action == GLFW_RELEASE) {
            isMousePressed = false;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    basic_camera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    basic_camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


// If you are confused with it's usage, then pass an identity matrix to it, and everything will be fine 
void drawCube(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans, 
    float posX, float posY, float posZ, 
    float rotX , float rotY, float rotZ, 
    float scX, float scY, float scZ,
    float r, float g, float b){   
    

    int colorLoc = glGetUniformLocation(shaderProgram.ID, "shapeColor");
    glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
    glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(r,g,b)));

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

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float unit = 0.3;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) translate_Y += unit;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) translate_Y -= unit;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) translate_X += unit;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) translate_X -= unit;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) translate_Z += unit;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) translate_Z -= unit;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) scale_X += unit;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) scale_X -= unit;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) scale_Y += unit;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) scale_Y -= unit;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) scale_Z += unit;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) scale_Z -= unit;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) on = true;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) on = false;

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) birdEye = true;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) birdEye = false;
    

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        rotateAngle_X += 1;
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    {
        rotateAngle_Y += 1;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        rotateAngle_Z += 1;
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        eyeX += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        eyeX -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);

        //cout << "x: "<<eyeX << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        eyeZ += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        eyeZ -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
        //cout << "z: " << eyeZ << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        eyeY += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
        //cout << "y: " << eyeY << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        eyeY -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        lookAtX += 2.5 * deltaTime;
        basic_camera.lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        lookAtX -= 2.5 * deltaTime;
        basic_camera.lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        lookAtY += 2.5 * deltaTime;
        basic_camera.lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        lookAtY -= 2.5 * deltaTime;
        basic_camera.lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
    }

    if (birdEye) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos.z -= birdEyeSpeed * deltaTime; 
            target.z -= birdEyeSpeed * deltaTime;
            if (cameraPos.z <= 4.0) {
                cameraPos.z = 4.0;
            }
            
            if (target.z <= -3.5) {
                target.z = -3.5;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos.z += birdEyeSpeed * deltaTime; 
            target.z += birdEyeSpeed * deltaTime;
            /*cout << "tgt: " << target.z << endl;
            cout << "pos: " << cameraPos.z << endl;*/
            if (cameraPos.z >= 13.5) {
                cameraPos.z = 13.5;
            }
            if (target.z >= 6.0) {
                target.z = 6.0;
            }
        }
    }

    
}
