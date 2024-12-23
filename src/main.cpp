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
#include <bits/stdc++.h>

using namespace std;

//#define PI 3.14159265359

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

enum class Axis{
    X, Y, Z
};

enum class EndFill{
    LEFT, RIGHT, BOTH, NONE
};

Shader lightingShader = Shader();
//Shader ourShader;
glm::mat4 identityMatrix = glm::mat4(1.0f);

int drawAll();//Shader shaderProgram, glm::mat4 parentTrans);
int drawRectDivider(
    float sx, float sy, float sz, float ex, float ey, float ez, 
    float r, float g, float b, float shininess, 
    int divCountX = 1, int divCountY = 1, int divCountZ = 1,
    Shader ourShader = lightingShader, glm::mat4 identityMatrix = identityMatrix);

int drawRect(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float shininess, Shader shaderProgram, glm::mat4 identityMatrix);
int repeatAlongX( float startX, float endX, /**/ float fixedY, float fixedZ, /**/ float widthX, float widthY, float widthZ, 
    int repeatCount, EndFill endFill, Shader ourShader = lightingShader, glm::mat4 identityMatrix = identityMatrix);

int repeatAlongZ( float startZ, float endZ, /**/ float fixedX, float fixedY, /**/ float widthX, float widthY, float widthZ, 
    int repeatCount, EndFill endFill, Shader ourShader = lightingShader, glm::mat4 identityMatrix = identityMatrix);
int drawGrave(float startX, float startY, float startZ);

int drawGraveCurve(
    float sx, float sy, float sz, 
    float mx, float my, float mz, 
    float ex, float ey, float ez, 
    int r, int g, int b, int shinniness,
    int numSegments, int curveResolution=100, int angleSteps=36);

void ambienton_off(Shader& lightingShader);
void diffuse_on_off(Shader& lightingShader);
void specular_on_off(Shader& lightingShader);

int size(float* arr);

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
    
    Shader ourShader = Shader(
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\VertexShader.vs", 
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\FragmentShader.fs"
    );

    lightingShader = Shader(
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
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;
        glm::vec3 color;

        glm::mat4 view;
        view = camera.GetViewMatrix();

        lightingShader.setMat4("view", view);
        lightingShader.setVec3("material.emissive", glm::vec3(0.0f, 0.0f, 0.0f));
        
        ourShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
    
        drawAll();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

// constant
const float WALL_WIDTH = 5.0f;
const float BASE_HEIGHT = 40.0f;
const float FLOOR_HEIGHT = 300.0f;
const float UPTO_BASE_HEIGHT = 20.0f + BASE_HEIGHT;
const float DOOR_HEIGHT = 220.0f;
const float GAP = 4.0f;
const float KEY_WIDTH = 15.0f;

const float MIN_X = -800.0f, MAX_X = 1700.0f;
const float MIN_Y = 0.0f, MAX_Y = 1000.0f;
const float MIN_Z = 0.0f, MAX_Z = 1500.0f;

int drawAll(){
    // draw rectangle

    // My canvas is of 1000 x 1000 x 1000
    
    // Ground
    drawRectDivider(/*start*/ -800, 0, 0,/*end*/ 1700, 20, 1500, /*color-shine*/ 112, 84, 62, 32, /*repeat*/1, 1, 10 );

    // All walls
    if(0)
    {
        // right wall
        drawRectDivider( /*start*/ MAX_X-WALL_WIDTH, WALL_WIDTH, MIN_Z, /*end pos*/ MAX_X, 200, MAX_Z-WALL_WIDTH, /*color*/ 139, 79, 57, /*shine*/ 32,
            1,10,1
        );
        // left wall
        drawRectDivider( /*start*/ MIN_X, 20, MIN_Z, /*end*/ MIN_X+WALL_WIDTH, 200, MAX_Z-WALL_WIDTH, /*color*/ 139, 79, 57, /*shine*/ 32, 1,10,1);

        // back wall
        drawRectDivider( /*start*/ MIN_X+WALL_WIDTH, 20, MIN_Z, /*end*/ MAX_X-WALL_WIDTH, 200, MIN_Z+WALL_WIDTH, /*color*/ 139, 79, 57, /*shine*/ 32, 
            1,10,1
        );

        // front wall - left half
        drawRectDivider( /*start*/ MIN_X, 20, MAX_Z-WALL_WIDTH, /*end*/ 400, 200, MAX_Z, /*color*/ 139, 79, 57, /*shine*/ 32, 1,10,1);
        // left side piller
        drawRectDivider( /*start*/ 400, 20, MAX_Z, /*end*/ 400+WALL_WIDTH, 200, MAX_Z, /*color*/ 143, 127, 132, /*shine*/ 32);
        // right side piller
        drawRectDivider( /*start*/ 600-WALL_WIDTH, 20, MAX_Z, /*end*/ 600, 200, MAX_Z, /*color*/ 143, 127, 132, /*shine*/ 32);
        // front wall - right half
        drawRectDivider( /*start*/ 600, 20, MAX_Z-WALL_WIDTH, /*end*/ MAX_X, 200, MAX_Z, /*color*/ 139, 79, 57, /*shine*/ 32, 1, 10, 1);
    }

    // building base & stairs
    if(0){
        // building base - main
        drawRectDivider( /*start pos*/ 100,20,100, /*end pos*/ 900, 20.0f+BASE_HEIGHT, 850, /*color*/ 90, 60, 40, /*shine*/ 10);

        // building base - little extra
        // drawRectDivider( /*start pos*/ 700,20,850, /*end pos*/ 900, 20+BASE_HEIGHT, 900, /*color*/ 90, 60, 40, /*shine*/ 10, 
        //     ourShader, identityMatrix, /*no of blocks*/ 1, 1, 1
        // );

        // stairs-1
        drawRectDivider( /*start pos*/ 500,20,850, /*end pos*/ 650, 20+BASE_HEIGHT-10.0f, 870, /*color*/ 105,105,105, /*shine*/ 15);
        // stairs-2
        drawRectDivider( /*start pos*/ 500,20,870, /*end pos*/ 650, 20+BASE_HEIGHT/2-5.0f, 890, /*color*/ 105,105,105, /*shine*/ 15);
    }

    // Ceiling of two stores
    if(0){
        // ceiling of first store
        drawRectDivider( /*start pos*/ 100,20+BASE_HEIGHT+FLOOR_HEIGHT,100, /*end pos*/ 900, 20+BASE_HEIGHT+FLOOR_HEIGHT+20, 850, /*color*/ 60, 50, 40, /*shine*/ 5);

        // // ceiling of second store
        // drawRectDivider( /*start pos*/ 100,20+BASE_HEIGHT+2*FLOOR_HEIGHT,100, /*end pos*/ 900, 20+BASE_HEIGHT+2*FLOOR_HEIGHT+20, 850, /*color*/ 60, 50, 40, /*shine*/ 5, 
        //     ourShader, identityMatrix, /*no of blocks*/ 1, 1, 1
        // );
    }

    // Piller && Supporting pillers
    if(0){
        float pillerWidthX = 10.0f;
        float pillerWidthZ = 15.0f;

        float supportinPillerWidthX = 5.0f;
        float supportinPillerWidthZ = 10.0f;
        
        float xPositions[] = {100,300,500-pillerWidthX,650};
        int xSize = 4;

        float minY = 20+BASE_HEIGHT;
        float maxY = minY+FLOOR_HEIGHT;
        float smallPillerHeight = minY + FLOOR_HEIGHT/4;

        float horizWoodHeightX = 5.0f;
        float horizWoodHeightY = 10.0f;
        float horizWoodHeightZ = 15.0f;

        float maxZ = 850;
        float minZ = maxZ - pillerWidthZ;
        int noOfSmallPillers = 3 + 1; // 5 piller. 1 for two side


        // front supporting pillers
        if(1){
            for(int i=0; i<xSize; i++){
                float x = xPositions[i];
                // Piller
                drawRectDivider( /*start*/ x,minY,minZ, /*end*/ x+pillerWidthX, maxY, maxZ, /*color*/ 80, 70, 60, /*shine*/ 12);

                // supporting small pillers
                if(i == 0) continue;
                if(i != 3){ // door
                    repeatAlongX(xPositions[i-1], xPositions[i], minY, minZ, supportinPillerWidthX, FLOOR_HEIGHT/4, supportinPillerWidthZ, 4, EndFill::NONE);
                    // horiz wooden bar
                    drawRectDivider( /*start*/ xPositions[i-1]+pillerWidthX,smallPillerHeight,minZ,
                        /*end*/ x, smallPillerHeight+horizWoodHeightY, maxZ, /*color*/ 50, 40, 30, /*shine*/ 12);
                }
            }
            // for the last piller to the end
            // repeatAlongX(xPositions[xSize-1], 900, minY, minZ, supportinPillerWidthX, FLOOR_HEIGHT/4, supportinPillerWidthZ, 3, EndFill::NONE);
            // drawRectDivider( /*start*/ xPositions[xSize-1]+pillerWidthX,smallPillerHeight,minZ,
            //    /*end*/ 900-pillerWidthX, smallPillerHeight+horizWoodHeightY, maxZ, /*color*/ 50, 40, 30, /*shine*/ 12);
        }

        // back supporting pillers
        if(1){
            minZ = 100;
            maxZ = minZ+pillerWidthZ;
            for(int i=0; i<xSize; i++){
                float x = xPositions[i];
                // Piller
                drawRectDivider( /*start*/ x,minY,minZ, /*end*/ x+pillerWidthX, maxY, maxZ, /*color*/ 80, 70, 60, /*shine*/ 12);

                // supporting small pillers
                if(i == 0) continue;
                repeatAlongX(xPositions[i-1], xPositions[i], minY, minZ, supportinPillerWidthX, FLOOR_HEIGHT/4, supportinPillerWidthZ, 4, EndFill::NONE);
                // horiz wooden bar
                drawRectDivider( /*start*/ xPositions[i-1]+pillerWidthX,smallPillerHeight,minZ,
                    /*end*/ x, smallPillerHeight+horizWoodHeightY, maxZ, /*color*/ 50, 40, 30, /*shine*/ 12);
            }
            // for the last piller to the end
            // repeatAlongX(xPositions[xSize-1], 900, minY, minZ, supportinPillerWidthX, FLOOR_HEIGHT/4, supportinPillerWidthZ, 3, EndFill::NONE);
            // drawRectDivider( /*start*/ xPositions[xSize-1]+pillerWidthX,smallPillerHeight,minZ,
            //    /*end*/ 900-pillerWidthX, smallPillerHeight+horizWoodHeightY, maxZ, /*color*/ 50, 40, 30, /*shine*/ 12);
        }

        // left supporting pillers
        if(1){
            float x = 100;
            float step = 150;
            float z = 100;
            while(z <= 850){
                float tempZ = z;
                if(z == 850){
                    z -= pillerWidthZ;
                }
                
                // left
                x = 100;
                drawRectDivider( /*start*/ x,minY,z, /*end*/ x+pillerWidthX, maxY, z+pillerWidthZ, /*color*/ 80, 70, 60, /*shine*/ 12);
                
                // right
                // x = 900 - pillerWidthX;
                // drawRectDivider( /*start*/ x,minY,z, /*end*/ x+pillerWidthX, maxY, z+pillerWidthZ, /*color*/ 80, 70, 60, /*shine*/ 12);

                if(tempZ == 100) { z += step; continue; }
                
                // left supporting small pillers
                x = 100;
                repeatAlongZ(tempZ-step, tempZ, x, minY, supportinPillerWidthX, FLOOR_HEIGHT/4, supportinPillerWidthZ, 4, EndFill::NONE);
                // right supporting small pillers
                // x = 900 - pillerWidthX;
                // repeatAlongZ(tempZ-step, tempZ, x, minY, supportinPillerWidthX, FLOOR_HEIGHT/4, supportinPillerWidthZ, 4, EndFill::NONE);

                // left wooden bar
                x = 100;
                if(tempZ == 850) tempZ -= pillerWidthZ;

                drawRectDivider( /*start*/ x, smallPillerHeight, tempZ-step+pillerWidthZ,
                    /*end*/ x+horizWoodHeightX, smallPillerHeight+horizWoodHeightY, tempZ, /*color*/ 50, 40, 30, /*shine*/ 12);
                
                // right wooden bar
                // x = 900 - horizWoodHeightX;
                // drawRectDivider( /*start*/ x, smallPillerHeight, tempZ-step+pillerWidthZ,
                //    /*end*/ x+horizWoodHeightX, smallPillerHeight+horizWoodHeightY, tempZ, /*color*/ 50, 40, 30, /*shine*/ 12);

                z += step;
            }
        }

    }

    // Room - first floor
    if(0){
        int wr = 70, wg = 55, wb = 45;
        // left wall
        drawRectDivider(/*st*/ 200, UPTO_BASE_HEIGHT, 250, /*e*/ 200+WALL_WIDTH, UPTO_BASE_HEIGHT + FLOOR_HEIGHT, 600,/*cs*/ wr,wg,wb,6);

        // back wall - left
        drawRectDivider(/*st*/ 200, UPTO_BASE_HEIGHT, 250-WALL_WIDTH, /*e*/ 650, UPTO_BASE_HEIGHT + FLOOR_HEIGHT, 250,/*cs*/ wr,wg,wb,6);
        // back wall - right
        drawRectDivider(/*st*/ 650, UPTO_BASE_HEIGHT, 100, /*e*/ 900, UPTO_BASE_HEIGHT + FLOOR_HEIGHT, 100+WALL_WIDTH,/*cs*/ wr,wg,wb,6);

        // front wall - left-top
        drawRectDivider(/*st*/ 200, UPTO_BASE_HEIGHT+DOOR_HEIGHT, 600, /*e*/ 650, UPTO_BASE_HEIGHT + FLOOR_HEIGHT, 600+WALL_WIDTH,/*cs*/ wr,wg,wb,6);
        // front wall - left-bottom
        drawRectDivider(/*st*/ 200, UPTO_BASE_HEIGHT, 600, /*e*/ 550, UPTO_BASE_HEIGHT + DOOR_HEIGHT, 600+WALL_WIDTH,/*cs*/ wr,wg,wb,6);
        // front wall - right
        drawRectDivider(/*st*/ 650, UPTO_BASE_HEIGHT, 850-WALL_WIDTH, /*e*/ 900, UPTO_BASE_HEIGHT + FLOOR_HEIGHT, 850,/*cs*/ wr,wg,wb,6);

        // middle wall - back part
        drawRectDivider(/*st*/ 650, UPTO_BASE_HEIGHT, 100, /*e*/ 650+WALL_WIDTH, UPTO_BASE_HEIGHT + FLOOR_HEIGHT, 250,/*cs*/ wr,wg,wb,6);
        // middle wall - front part
        drawRectDivider(/*st*/ 650, UPTO_BASE_HEIGHT, 600, /*e*/ 650+WALL_WIDTH, UPTO_BASE_HEIGHT + FLOOR_HEIGHT, 850,/*cs*/ wr,wg,wb,6);

        // right wall
        drawRectDivider(/*st*/ 900-WALL_WIDTH, UPTO_BASE_HEIGHT, 100, /*e*/ 900, UPTO_BASE_HEIGHT + FLOOR_HEIGHT, 850,/*cs*/ wr,wg,wb,6);
    }

    // Building door - first floor
    if(0){
        int wr = 45, wg = 30, wb = 20, sh = 10;
        // left bar
        drawRectDivider(/*st*/ 550, UPTO_BASE_HEIGHT, 600, /*e*/ 550+WALL_WIDTH, UPTO_BASE_HEIGHT + DOOR_HEIGHT, 600+WALL_WIDTH,/*cs*/ wr,wg,wb,sh);
        // right bar
        drawRectDivider(/*st*/ 650-WALL_WIDTH, UPTO_BASE_HEIGHT, 600, /*e*/ 650, UPTO_BASE_HEIGHT + DOOR_HEIGHT, 600+WALL_WIDTH,/*cs*/ wr,wg,wb,sh);
        // top bar
        drawRectDivider(/*st*/ 550+WALL_WIDTH, UPTO_BASE_HEIGHT+DOOR_HEIGHT-WALL_WIDTH, 600, /*e*/ 650-WALL_WIDTH, UPTO_BASE_HEIGHT + DOOR_HEIGHT, 600+WALL_WIDTH,/*cs*/ wr,wg,wb,sh);
        
        // door body
        drawRectDivider(/*st*/ 550+WALL_WIDTH, UPTO_BASE_HEIGHT+GAP, 600, /*e*/ 650-WALL_WIDTH, UPTO_BASE_HEIGHT + DOOR_HEIGHT-GAP, 600+WALL_WIDTH,/*cs*/ 30, 20, 15, 5);
        
        // door key hole holder. -> Slap key hole image
        drawRectDivider(/*st*/ 650-4*GAP-KEY_WIDTH, UPTO_BASE_HEIGHT+DOOR_HEIGHT/2, 600+WALL_WIDTH, /*e*/ 650-4*GAP, UPTO_BASE_HEIGHT+DOOR_HEIGHT/2+KEY_WIDTH, 600+WALL_WIDTH+5,/*cs*/ 0, 0, 0, 5);
    }

    // Outside graveyard
    drawGrave( /*start*/ MIN_X+100, 20, MIN_Z+100);

    return 0;
}

const float GRAVE_X = 80.0f;
const float GRAVE_BASE_Y = 5.0f;
const float GRAVE_Z = 200.0f;
int drawGrave(float startX, float startY, float startZ){
    float gap = 4.0f;
    if(0){
        // base
        drawRectDivider( /*st*/ startX, startY, startZ, /*e*/ startX+GRAVE_X, startY+GRAVE_BASE_Y, startZ+GRAVE_Z, /*cs*/ 182, 166, 151, 12);

        // top base - left & right
        drawRectDivider( /*st*/ startX+gap, startY+GRAVE_BASE_Y, startZ+gap, /*e*/ startX+gap+.1*GRAVE_X, startY+4*GRAVE_BASE_Y, startZ+GRAVE_Z-gap, /*cs*/ 137, 139, 159, 12);
        drawRectDivider( /*st*/ startX+.9*GRAVE_X-gap, startY+GRAVE_BASE_Y, startZ+gap, /*e*/ startX-gap+GRAVE_X, startY+4*GRAVE_BASE_Y, startZ+GRAVE_Z-gap, /*cs*/ 137, 139, 159, 12);
        
        // top base - front & back
        drawRectDivider( /*st*/ startX+gap+.1*GRAVE_X, startY+GRAVE_BASE_Y, startZ+gap, /*e*/ startX+.9*GRAVE_X-gap, startY+4*GRAVE_BASE_Y, startZ+gap+.1*GRAVE_X, /*cs*/ 137, 139, 159, 12);
        drawRectDivider( /*st*/ startX+gap+.1*GRAVE_X, startY+GRAVE_BASE_Y, startZ+GRAVE_Z-gap-.1*GRAVE_X, /*e*/ startX+.9*GRAVE_X-gap, startY+4*GRAVE_BASE_Y, startZ+GRAVE_Z-gap, /*cs*/ 137, 139, 159, 12);

        // base mud
        drawRectDivider( /*st*/ startX+gap+.1*GRAVE_X, startY+GRAVE_BASE_Y, startZ+gap+.1*GRAVE_X, /*e*/ startX+GRAVE_X-gap, startY+3*GRAVE_BASE_Y, startZ+GRAVE_Z-gap, /*cs*/ 54, 50, 149, 12);
    }    
    if(1){
        // square
        drawRectDivider( /*st*/ startX+gap+.2*GRAVE_X, startY+4*GRAVE_BASE_Y, startZ+gap+.1*GRAVE_X, 
                        /*e*/ startX+.8*GRAVE_X-gap, startY+10*GRAVE_BASE_Y, startZ+gap+.2*GRAVE_X, /*cs*/ 137, 139, 159, 12);
        // curved top on square
        drawGraveCurve(
            /*st*/ startX+.2*GRAVE_X, startY+10*GRAVE_BASE_Y, startZ+gap+.1*GRAVE_X,
            /*mid*/ startX+.5*GRAVE_X, startY+14*GRAVE_BASE_Y, startZ+gap+.1*GRAVE_X,
            /*end*/ startX+.8*GRAVE_X, startY+10*GRAVE_BASE_Y, startZ+gap+.1*GRAVE_X, 
            /*rgbs*/ 255,255, 255, 32, /*noOfSegments*/ 36
        );
    }
    return 0;
}

float lengthX = 1000.0f;
float lengthY = 1000.0f;
float lengthZ = 1000.0f;

int drawGraveCurve( /*st*/ float sx, float sy, float sz, /*mid*/ float mx, float my, float mz, 
    /*end*/ float ex, float ey, float ez, /*cs*/ int r, int g, int b, int shininess,
    int numSegments, int curveResolution, int angleSteps){

    sx /= lengthX; sy /= lengthY; sz /= lengthZ; 
    mx /= lengthX; my /= lengthY; mz /= lengthZ;
    ex /= lengthX; ey /= lengthY; ez /= lengthZ;

    glm::vec3 P0 = glm::vec3(sx, sy, sz); // Start point
    glm::vec3 P1 = glm::vec3(mx, my, mz); // Control/Mid point
    glm::vec3 P2 = glm::vec3(ex, ey, ez); // End point

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind VAO and VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Generate vertices for the curve and create triangle fan vertices
    std::vector<GLfloat> vertVec;
    std::vector<GLuint> indices;

    int noOfZSegments = 2;
    float zLength = WALL_WIDTH/lengthZ;
    float stepZ = zLength / noOfZSegments;

    int midPointIndex = (numSegments+1) * (noOfZSegments+1);

    for (int i = 0; i <= numSegments; i++) {
        float t = static_cast<float>(i) / numSegments;
        glm::vec3 point = (1 - t) * (1 - t) * P0 + 2 * (1 - t) * t * P1 + t * t * P2; // Bezier formula

        int nos = noOfZSegments+1;
        if(i != 0){
            indices.push_back(midPointIndex); indices.push_back((i-1)*nos); indices.push_back(i*nos);
            indices.push_back(midPointIndex+1); indices.push_back(i*nos-1); indices.push_back((i+1)*nos-1);
        }

        for(int j=0; j<=noOfZSegments; j++){
            // point
            vertVec.push_back(point.x); vertVec.push_back(point.y); vertVec.push_back(point.z + j*stepZ);
            // normal
            vertVec.push_back(0); vertVec.push_back(0); vertVec.push_back(1);

            if(i == 0) continue;


            
            // triangle index
            if(j != noOfZSegments){
                indices.push_back(i*nos+j); indices.push_back((i-1)*nos+j); indices.push_back((i-1)*nos+j+1);
            }
            if(j != 0){
                indices.push_back(i*nos+j); indices.push_back((i-1)*nos+j); indices.push_back(i*nos+j-1);
            }
        }
        // if(i==2) break;
    }

    // middle point front
    vertVec.push_back((sx+ex)/2); vertVec.push_back((sy+ey)/2); vertVec.push_back((sz+ez)/2);
    vertVec.push_back(0); vertVec.push_back(0); vertVec.push_back(1);
    // middle point back
    vertVec.push_back((sx+ex)/2); vertVec.push_back((sy+ey)/2); vertVec.push_back((sz+ez)/2 + zLength);
    vertVec.push_back(0); vertVec.push_back(0); vertVec.push_back(1);

    // Set up the color
    glm::vec3 color(r / 255.0f, g / 255.0f, b / 255.0f);

    // Shader setup
    lightingShader.use();
    lightingShader.setVec3("material.ambient", color);
    lightingShader.setVec3("material.diffuse", color);
    lightingShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    lightingShader.setFloat("material.shininess", shininess);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertVec.size() * sizeof(GLfloat), vertVec.data(), GL_STATIC_DRAW);

    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);  // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); // normal
    glEnableVertexAttribArray(1);

    // Create and bind EBO (Element Buffer Object)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Draw the filled triangle fan
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // Unbind VAO, VBO, EBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Delete buffers
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    return 0;
}


// int drawQuadraticBezierCurve(
//     float sx, float sy, float sz, 
//     float mx, float my, float mz, 
//     float ex, float ey, float ez, 
//     int r, int g, int b, int shinniness,
//     int numSegments, int curveResolution, int angleSteps){
//     // Generate vertices for the Bézier curve
//     sx /= lengthX; sy /= lengthY; sz /= lengthZ; 
//     mx /= lengthX; my /= lengthY; mz /= lengthZ;
//     ex /= lengthX; ey /= lengthY; ez /= lengthZ;

//     glm::vec3 P0 = glm::vec3(sx,sy,sz); // Start point
//     glm::vec3 P1 = glm::vec3(mx,my,mz);   // Control point
//     glm::vec3 P2 = glm::vec3(ex,ey,ez);  // End point

//     std::vector<GLfloat> vertVec;

//     vertVec.push_back((sx+ex)/2);
//     vertVec.push_back((sy+ey)/2);
//     vertVec.push_back((sz+ez)/2);

//     for (int i = 0; i <= numSegments; i++) {
//         float t = static_cast<float>(i) / numSegments;
//         glm::vec3 point = (1 - t) * (1 - t) * P0 + 2 * (1 - t) * t * P1 + t * t * P2; // Bézier formula
//         vertVec.push_back(point.x);
//         vertVec.push_back(point.y);
//         vertVec.push_back(point.z);
//     }

//     GLfloat vertices[2*vertVec.size()];
//     GLuint indices[vertVec.size()+1];


//     for(int i=0; i<vertVec.size(); i+=3){
//         vertices[2*i] = vertVec[i];
//         vertices[2*i+1] = vertVec[i+1];
//         vertices[2*i+2] = vertVec[i+2];
//         // normal
//         vertices[2*i+3] = 0; vertices[2*i+4] = 0; vertices[2*i+5] = 1;

//         indices[i] = i;
//         indices[i+1] = i+1;
//         indices[i+2] = i+2;
//     }
//     indices[vertVec.size()] = 0;

//     // Shader setup
//     lightingShader.use();
//     lightingShader.setVec3("material.ambient", glm::vec3(r, g, b));
//     lightingShader.setVec3("material.diffuse", glm::vec3(r, g, b));
//     lightingShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
//     lightingShader.setFloat("material.shininess", shinniness);
//     lightingShader.setMat4("model", identityMatrix);

//     // Generate and bind VAO
//     GLuint VAO, VBO, EBO;
//     glGenVertexArrays(1, &VAO);
//     glGenBuffers(1, &VBO);
//     glGenBuffers(1, &EBO);

//     glBindVertexArray(VAO);

//     // Vertex buffer object (VBO)
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//     // Element buffer object (EBO)
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

//     // Vertex attributes (position and normal)
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
//     glEnableVertexAttribArray(0);

//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(1);

//     // Draw the cuboid
//     glBindVertexArray(VAO);
//     glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

//     // Cleanup
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);

//     glDeleteBuffers(1, &VBO);
//     glDeleteBuffers(1, &EBO);
//     glDeleteVertexArrays(1, &VAO);
//     glDeleteVertexArrays(1, &VAO);
    
//     return 0;
// }


int repeatAlongX(
    float startX, float endX, 
    float fixedY, float fixedZ, 
    float widthX, float widthY, float widthZ, 
    int repeatCount, EndFill endFill, Shader ourShader, glm::mat4 identityMatrix)
{

    float step = (endX - startX) / (repeatCount+1);
    float x = startX + step;
    if(endFill == EndFill::LEFT || endFill == EndFill::BOTH){ // no left fill is needed
        x = startX;
    }

    while(x < endX){
        drawRectDivider( /*start pos*/ x,fixedY,fixedZ, /*end pos*/ x+widthX, fixedY+widthY, fixedZ+widthZ, /*color*/ 80, 70, 60, /*shine*/ 12);
        x += step;
    }
    // no right fill is needed
    if(endFill !=  EndFill::RIGHT && endFill != EndFill::BOTH) return 1;
    
    // Filling right
    drawRectDivider( /*start pos*/ endX-widthX, fixedY,fixedZ, /*end pos*/ endX, fixedY+widthY, fixedZ+widthZ, /*color*/ 80, 70, 60, /*shine*/ 12);

    return 0;
}

int repeatAlongZ(
    float startZ, float endZ, 
    float fixedX, float fixedY, 
    float widthX, float widthY, float widthZ, 
    int repeatCount, EndFill endFill, Shader ourShader, glm::mat4 identityMatrix)
{

    float step = (endZ - startZ) / (repeatCount+1);
    float z = startZ + step;
    if(endFill == EndFill::LEFT || endFill == EndFill::BOTH){ // no left fill is needed
        z = startZ;
    }

    while(z < endZ){
        drawRectDivider( /*start pos*/ fixedX,fixedY,z, /*end pos*/ fixedX+widthX, fixedY+widthY, z+widthZ, /*color*/ 80, 70, 60, /*shine*/ 12);
        z += step;
    }
    // no right fill is needed
    if(endFill !=  EndFill::RIGHT && endFill != EndFill::BOTH) return 1;
    
    // Filling right
    drawRectDivider( /*start pos*/ fixedX, fixedY, endZ-widthZ, /*end pos*/ fixedX+widthX, fixedY+widthY, endZ, /*color*/ 80, 70, 60, /*shine*/ 12);

    return 0;
}

int drawRectDivider(
    float sx, float sy, float sz, float ex, float ey, float ez, 
    float r, float g, float b, 
    float shininess, int divCountX, int divCountY, int divCountZ,
    Shader ourShader, glm::mat4 identityMatrix){

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

int drawRectOld(
    float sx, float sy, float sz, float ex, float ey, float ez, 
    float r, float g, float b, float shininess, Shader ourShader, glm::mat4 identityMatrix){
    // top left point (sx,sy,sz)
    // bottom right point (ex,ey,ez)
    // color properties
    
    GLfloat vertices[] = {
        sx/lengthX, sy/lengthY, sz/lengthZ,  1.0f, 0.0f, 0.0f, // Bottom-left
        ex/lengthX, sy/lengthY, sz/lengthZ,  0.0f, 1.0f, 0.0f, // Bottom-right
        ex/lengthX, ey/lengthY, sz/lengthZ,  0.0f, 0.0f, 1.0f, // Top-right
        sx/lengthX, ey/lengthY, sz/lengthZ,  1.0f, 1.0f, 0.0f,  // Top-left

        // back face
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

int drawRect(
    float sx, float sy, float sz, float ex, float ey, float ez, 
    float r, float g, float b, float shininess, Shader ourShader, glm::mat4 modelMatrix) {

    sx /= lengthX; sy /= lengthY; sz /= lengthZ;
    ex /= lengthX; ey /= lengthY; ez /= lengthZ;
    
    // Vertices for the cuboid (position + normals)
    GLfloat vertices[] = {
        // Front face
        sx, sy, sz,  0.0f,  0.0f, -1.0f,  // Bottom-left
        ex, sy, sz,  0.0f,  0.0f, -1.0f,  // Bottom-right
        ex, ey, sz,  0.0f,  0.0f, -1.0f,  // Top-right
        sx, ey, sz,  0.0f,  0.0f, -1.0f,  // Top-left

        // Back face
        sx, sy, ez,  0.0f,  0.0f,  1.0f,  // Bottom-left
        ex, sy, ez,  0.0f,  0.0f,  1.0f,  // Bottom-right
        ex, ey, ez,  0.0f,  0.0f,  1.0f,  // Top-right
        sx, ey, ez,  0.0f,  0.0f,  1.0f,  // Top-left

        // Left face
        sx, sy, sz, -1.0f,  0.0f,  0.0f,  // Bottom-left
        sx, sy, ez, -1.0f,  0.0f,  0.0f,  // Bottom-right
        sx, ey, ez, -1.0f,  0.0f,  0.0f,  // Top-right
        sx, ey, sz, -1.0f,  0.0f,  0.0f,  // Top-left

        // Right face
        ex, sy, sz,  1.0f,  0.0f,  0.0f,  // Bottom-left
        ex, sy, ez,  1.0f,  0.0f,  0.0f,  // Bottom-right
        ex, ey, ez,  1.0f,  0.0f,  0.0f,  // Top-right
        ex, ey, sz,  1.0f,  0.0f,  0.0f,  // Top-left

        // Top face
        sx, ey, sz,  0.0f,  1.0f,  0.0f,  // Bottom-left
        ex, ey, sz,  0.0f,  1.0f,  0.0f,  // Bottom-right
        ex, ey, ez,  0.0f,  1.0f,  0.0f,  // Top-right
        sx, ey, ez,  0.0f,  1.0f,  0.0f,  // Top-left

        // Bottom face
        sx, sy, sz,  0.0f, -1.0f,  0.0f,  // Bottom-left
        ex, sy, sz,  0.0f, -1.0f,  0.0f,  // Bottom-right
        ex, sy, ez,  0.0f, -1.0f,  0.0f,  // Top-right
        sx, sy, ez,  0.0f, -1.0f,  0.0f   // Top-left
    };

    GLuint indices[] = { 
        /* Front */ 0, 1, 2, 2, 3, 0, 
        /* Back  */ 4, 5, 6, 6, 7, 4, 
        /* Left  */ 8, 9, 10, 10, 11, 8,
        /* Right */ 12, 13, 14, 14, 15, 12, 
        /* Top   */ 16, 17, 18, 18, 19, 16, 
        /* Bottom*/ 20, 21, 22, 22, 23, 20
    };

    // Shader setup
    ourShader.use();
    ourShader.setVec3("material.ambient", glm::vec3(r, g, b));
    ourShader.setVec3("material.diffuse", glm::vec3(r, g, b));
    ourShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    ourShader.setFloat("material.shininess", shininess);
    ourShader.setMat4("model", modelMatrix);

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

    // Vertex attributes (position and normal)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Draw the cuboid
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


int size(float* arr){
    return sizeof(arr)/sizeof(arr[0]);
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
