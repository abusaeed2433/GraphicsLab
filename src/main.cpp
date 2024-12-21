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
#include "common/PointLight.h"
#include "common/sphere.h"
#include "common/Torus.h"

#include <unordered_map>
#include <memory>

#include <set>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

using namespace std;

//#define PI 3.14159265359

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

int drawAll(Shader shaderProgram, glm::mat4 parentTrans);
int drawRectDivider( float sx, float sy, float sz, float ex, float ey, float ez, 
    float r, float g, float b, float shininess, Shader ourShader, glm::mat4 identityMatrix,
    int divCountX = 1, int divCountY = 1, int divCountZ = 1);
int drawRect(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float shininess, Shader shaderProgram, glm::mat4 identityMatrix);


void ambienton_off(Shader& lightingShader);
void diffuse_on_off(Shader& lightingShader);
void specular_on_off(Shader& lightingShader);

void drawCube(
    Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans, 
    float posX = 0.0, float posY = 0.0, float posz = 0.0, 
    float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0,
    float scX = 1.0, float scY = 1.0, float scZ=1.0,
    float r = 0.0, float g = 0.0, float b = 0.0, float shininess = 32.0);

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
// Time management
double lastKeyPressTime = 0.0;
const double keyPressDelay = 0.2; // delay in seconds

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float eyeX = 500/1000.0f, eyeY = 500/1000.0f, eyeZ = 1500/1000.0f;
float lookAtX = 500/1000.0f, lookAtY = 500/1000.0f, lookAtZ = 0.0f;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
// BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);
Camera camera(glm::vec3(eyeX, eyeY, eyeZ));

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
    glfwSetKeyCallback(window, key_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

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

// lights var
//directional light
bool directionLightOn = true;
bool directionalAmbient = true;
bool directionalDiffuse = true;
bool directionalSpecular = true;

bool pointLightOnOne = true;
bool pointLightOnTwo = true;
bool AmbientON = true;
bool DiffusionON = true;
bool SpecularON = true;
bool ambientToggle = true;
bool diffuseToggle = true;
bool specularToggle = true;

//spot light
bool spotLightOn = true;

//point light
bool point1 = true;
bool point2 = true;

//custom projection matrix
float fov = glm::radians(camera.Zoom);
float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
float near = 0.1f;
float far = 100.0f;
float tanHalfFOV = tan(fov / 2.0f);
//positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(3.0f,  3.0f,  4.0f),
    glm::vec3(3.0f,  1.0f,  2.0f),
};

PointLight pointlight1(
    pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z,       // position
    0.2f, 0.2f, 0.2f,       //ambient
    0.8f, 0.8f, 0.8f,       //diffuse
    1.0f, 1.0f, 1.0f,       //specular
    1.0f,       //k_c
    0.09f,      //k_l
    0.032f,     //k_q
    1       //light number
);

PointLight pointlight2(
    pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z,
    0.2f, 0.2f, 0.2f,
    0.8f, 0.8f, 0.8f,
    1.0f, 1.0f, 1.0f,
    1.0f,
    0.09f,
    0.032f,
    2
);
// lights var above


int main()
{
    GLFWwindow* window = nullptr;
    if( initGlfw(window) ) return -1;

    glEnable(GL_DEPTH_TEST);
    
    Shader ourShader(
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\VertexShader.vs", 
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\FragmentShader.fs"
    );

    Shader lightingShader(
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\vertexShaderForGouraudShading.vs", 
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\fragmentShaderForGouraudShading.fs"
    );

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         
        { //lighting below
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        //point lights set up
        pointlight1.setUpPointLight(lightingShader);
        pointlight2.setUpPointLight(lightingShader);

        //directional light set up
        lightingShader.setVec3("directionalLight.direction", 0.0f, -1.0f, 0.0f);
        lightingShader.setVec3("directionalLight.ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("directionalLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("directionalLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setBool("directionLightOn", directionLightOn);

        //spot light set up
        lightingShader.setVec3("spotLight.position", 2.0f, 2.0f, 3.0f);
        lightingShader.setVec3("spotLight.direction", 0.0f, -1.0f, 0.0f);
        lightingShader.setVec3("spotLight.ambient", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.k_c", 1.0f);
        lightingShader.setFloat("spotLight.k_l", 0.09);
        lightingShader.setFloat("spotLight.k_q", 0.032);
        lightingShader.setFloat("spotLight.cos_theta", glm::cos(glm::radians(60.0f)));
        lightingShader.setBool("spotLightOn", spotLightOn);

        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
            ambienton_off(lightingShader);
        }

        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
            diffuse_on_off(lightingShader);
        }

        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
            specular_on_off(lightingShader);
        }

        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
            spotLightOn = !spotLightOn;
        }

        glm::mat4 projection(0.0f);
        projection[0][0] = 1.0f / (aspect * tanHalfFOV);
        projection[1][1] = 1.0f / tanHalfFOV;
        projection[2][2] = -(far + near) / (far - near);
        projection[2][3] = -1.0f;
        projection[3][2] = -(2.0f * far * near) / (far - near);
        
        lightingShader.setMat4("projection", projection);
        glm::mat4 view;
        
        //define matrices and vectors needed
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;
        glm::vec3 color;
        
        //initialize all elements as non-emissive
        lightingShader.setVec3("material.emissive", glm::vec3(0.0f, 0.0f, 0.0f)); }

        //define matrices and vectors needed
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;
        glm::vec3 color;

        glm::mat4 view;
        view = camera.GetViewMatrix();

        lightingShader.setMat4("view", view);
        lightingShader.setVec3("material.emissive", glm::vec3(0.0f, 0.0f, 0.0f));
        
        ourShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
    
        drawAll(lightingShader, identityMatrix);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

int drawAll(Shader ourShader, glm::mat4 identityMatrix){
    // draw rectangle

    // My canvas is of 1000 x 1000 x 1000
    
    // Ground
    drawRectDivider(
        0, 0, 0,
        1000, 20, 1000,

        112, 84, 62, 32, 
        ourShader, identityMatrix,
        1, 1, 10
    );

    // All walls
    // right wall
    drawRectDivider( /*start pos*/ 980, 20, 0, /*end pos*/ 1000, 200, 980, /*color*/ 139, 79, 57, /*shine*/ 32, 
        ourShader, identityMatrix, /*no of blocks*/ 1, 10, 1
    );
    // left wall
    drawRectDivider( /*start pos*/ 0, 20, 0, /*end pos*/ 20, 200, 980, /*color*/ 139, 79, 57, /*shine*/ 32, 
        ourShader, identityMatrix, /*no of blocks*/ 1, 10, 1
    );

    // back wall
    drawRectDivider( /*start pos*/ 20, 20, 0, /*end pos*/ 980, 200, 20, /*color*/ 139, 79, 57, /*shine*/ 32, 
        ourShader, identityMatrix, /*no of blocks*/ 1, 10, 1
    );

    // front wall - left half
    drawRectDivider( /*start pos*/ 0, 20, 980, /*end pos*/ 400, 200, 1000, /*color*/ 139, 79, 57, /*shine*/ 32, 
        ourShader, identityMatrix, /*no of blocks*/ 1, 10, 1
    );
    // left side piller
    drawRectDivider( /*start pos*/ 400, 20, 1000, /*end pos*/ 420, 200, 1000, /*color*/ 143, 127, 132, /*shine*/ 32, 
        ourShader, identityMatrix, /*no of blocks*/ 1, 1, 1
    );
    // right side piller
    drawRectDivider( /*start pos*/ 580, 20, 1000, /*end pos*/ 600, 200, 1000, /*color*/ 143, 127, 132, /*shine*/ 32, 
        ourShader, identityMatrix, /*no of blocks*/ 1, 1, 1
    );
    // front wall - right half
    drawRectDivider( /*start pos*/ 600, 20, 980, /*end pos*/ 1000, 200, 1000, /*color*/ 139, 79, 57, /*shine*/ 32, 
        ourShader, identityMatrix, /*no of blocks*/ 1, 10, 1
    );




}

float lengthX = 1000.0f;
float lengthY = 1000.0f;
float lengthZ = 1000.0f;

int drawRectDivider(
    float sx, float sy, float sz, float ex, float ey, float ez, 
    float r, float g, float b, 
    float shininess, Shader ourShader, glm::mat4 identityMatrix,
    int divCountX, int divCountY, int divCountZ){

    r /= 255.0f;
    g /= 255.0f;
    b /= 255.0f;

    float dx = (ex - sx) / divCountX;
    float dy = (ey - sy) / divCountY;
    float dz = (ez - sz) / divCountZ;

    for(int i = 0; i < divCountX; i++){
        for(int j = 0; j < divCountY; j++){
            for(int k = 0; k < divCountZ; k++){
                drawRect(
                    sx + i*dx, sy + j*dy, sz + k*dz, 
                    sx + (i+1)*dx, sy + (j+1)*dy, sz + (k+1)*dz, 
                    r, g, b,
                    shininess, ourShader, identityMatrix);
            }
        }
    }

    return 1;
}

int drawRect(
    float sx, float sy, float sz, float ex, float ey, float ez, 
    float r, float g, float b, float shininess, Shader ourShader, glm::mat4 identityMatrix){
    // top left point (sx,sy,sz)
    // bottom right point (ex,ey,ez)
    // color properties
    
    GLfloat vertices[] = {
        // Positions           // Colors
        sx/lengthX, sy/lengthY, sz/lengthZ,  1.0f, 0.0f, 0.0f, // Bottom-left
        ex/lengthX, sy/lengthY, sz/lengthZ,  0.0f, 1.0f, 0.0f, // Bottom-right
        ex/lengthX, ey/lengthY, sz/lengthZ,  0.0f, 0.0f, 1.0f, // Top-right
        sx/lengthX, ey/lengthY, sz/lengthZ,  1.0f, 1.0f, 0.0f,  // Top-left

        sx/lengthX, sy/lengthY, ez/lengthZ,  1.0f, 0.0f, 0.0f, // Bottom-left
        ex/lengthX, sy/lengthY, ez/lengthZ,  0.0f, 1.0f, 0.0f, // Bottom-right
        ex/lengthX, ey/lengthY, ez/lengthZ,  0.0f, 0.0f, 1.0f, // Top-right
        sx/lengthX, ey/lengthY, ez/lengthZ,  1.0f, 1.0f, 0.0f  // Top-left        
    };

    GLuint indices[] = { 
        /* Front */ 0, 1, 2, 2, 3, 0, /* Back */ 4, 5, 6, 6, 7, 4, /*  Left  */ 0, 4, 7, 7, 3, 0,
        /* Right */ 1, 5, 6, 6, 2, 1, /* Top  */ 3, 7, 6, 6, 2, 3, /* Bottom */ 0, 1, 5, 5, 4, 0
    };

    ourShader.use();
    ourShader.use();
    ourShader.setVec3("material.ambient", glm::vec3(r, g, b));
    ourShader.setVec3("material.diffuse", glm::vec3(r, g, b));
    ourShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    ourShader.setFloat("material.shininess", shininess);

    // Generate and bind VAO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex buffer object (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Element buffer object (EBO)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex attributes (position and color)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    

    glm::mat4 model = glm::mat4(1.0f);
    ourShader.setMat4("model", identityMatrix);
    ourShader.setVec4("shapeColor", glm::vec4(1.0, 1.0, 1.0, 1.0));

    // Draw the cube
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    return 0;
}



// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// If you are confused with it's usage, then pass an identity matrix to it, and everything will be fine 
void drawCube(Shader lightingShader, unsigned int VAO, glm::mat4 parentTrans, 
    float posX, float posY, float posZ, 
    float rotX , float rotY, float rotZ, 
    float scX, float scY, float scZ,
    float r, float g, float b, float shininess){   
    

    int colorLoc = glGetUniformLocation(lightingShader.ID, "shapeColor");
    glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
    glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(r,g,b)));

    lightingShader.use();
    lightingShader.setVec3("material.ambient", glm::vec3(r, g, b));
    lightingShader.setVec3("material.diffuse", glm::vec3(r, g, b));
    lightingShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    lightingShader.setFloat("material.shininess", shininess);

    

    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered;
    translateMatrix = glm::translate(parentTrans, glm::vec3(posX, posY, posZ));
    rotateXMatrix = glm::rotate(translateMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(rotateXMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(rotateYMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
    //modelCentered = glm::translate(model, glm::vec3(-0.25, -0.25, -0.25));

    lightingShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (rotateAxis_X) rotateAngle_X -= 0.1;
        else if (rotateAxis_Y) rotateAngle_Y -= 0.1;
        else rotateAngle_Z -= 0.1;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera.ProcessKeyboard(UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        camera.ProcessKeyboard(DOWN, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        camera.ProcessKeyboard(Y_LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        camera.ProcessKeyboard(Y_RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        camera.ProcessKeyboard(R_LEFT, deltaTime);

    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        camera.ProcessKeyboard(R_RIGHT, deltaTime);

    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        camera.ProcessKeyboard(P_UP, deltaTime);

    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        camera.ProcessKeyboard(P_DOWN, deltaTime);
    }
}


void ambienton_off(Shader& lightingShader)
{
    double currentTime = glfwGetTime();
    if (currentTime - lastKeyPressTime < keyPressDelay) return;
    lightingShader.use();
    if (AmbientON)
    {
        pointlight1.turnAmbientOff();
        pointlight2.turnAmbientOff();
        lightingShader.setVec3("directionalLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        AmbientON = !AmbientON;
        lastKeyPressTime = currentTime;
    }
    else
    {
        pointlight1.turnAmbientOn();
        pointlight2.turnAmbientOn();
        lightingShader.setVec3("directionalLight.ambient", 0.2f, 0.2f, 0.2f);
        lightingShader.setVec3("spotLight.ambient", 0.2f, 0.2f, 0.2f);
        AmbientON = !AmbientON;
        lastKeyPressTime = currentTime;
    }
}
void diffuse_on_off(Shader& lightingShader)
{
    double currentTime = glfwGetTime();
    if (currentTime - lastKeyPressTime < keyPressDelay) return;
    lightingShader.use();
    if (DiffusionON)
    {
        pointlight1.turnDiffuseOff();
        pointlight2.turnDiffuseOff();
        lightingShader.setVec3("directionalLight.diffuse", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);
        DiffusionON = !DiffusionON;
        lastKeyPressTime = currentTime;
    }
    else
    {
        pointlight1.turnDiffuseOn();
        pointlight2.turnDiffuseOn();
        lightingShader.setVec3("directionalLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
        DiffusionON = !DiffusionON;
        lastKeyPressTime = currentTime;
    }
}
void specular_on_off(Shader& lightingShader)
{
    double currentTime = glfwGetTime();
    if (currentTime - lastKeyPressTime < keyPressDelay) return;
    lightingShader.use();
    if (SpecularON)
    {
        pointlight1.turnSpecularOff();
        pointlight2.turnSpecularOff();
        lightingShader.setVec3("directionalLight.specular", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);
        SpecularON = !SpecularON;
        lastKeyPressTime = currentTime;
    }
    else
    {
        pointlight1.turnSpecularOn();
        pointlight2.turnSpecularOn();
        lightingShader.setVec3("directionalLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        SpecularON = !SpecularON;
        lastKeyPressTime = currentTime;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){    
}

// glfw: whenever the mouse moves, this callback is called
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
