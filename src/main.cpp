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
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void drawFan(unsigned int VAO, Shader ourShader, glm::mat4 translateMatrix, glm::mat4 sm);

int drawAll(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans);
void drawCylinder(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
    float centerX, float centerY, float centerZ,
    float radius, float height, int segments,
    float r, float g, float b);

void drawCone(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
              float centerX, float centerY, float centerZ,
              float radius, float height, int segments,
              float r, float g, float b);

void drawFilledCircle(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
    float centerX, float centerY, float centerZ,
    float radius, int segmentsPerRing, int rings,
    float r, float g, float b);

void drawCube(
    Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans, 
    float posX = 0.0, float posY = 0.0, float posz = 0.0, 
    float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0,
    float scX = 1.0, float scY = 1.0, float scZ=1.0,
    float r = 0.0, float g = 0.0, float b = 0.0, float shininess = 32.0);

void ambienton_off(Shader& lightingShader);
void diffuse_on_off(Shader& lightingShader);
void specular_on_off(Shader& lightingShader);

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

float eyeX = 1.35, eyeY = 4.8, eyeZ = 10.0;
float lookAtX = 4.0, lookAtY = 4.0, lookAtZ = 6.0;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
// BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);
Camera camera(glm::vec3(eyeX, eyeY, eyeZ));

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

bool on = false;
// light settings
bool pointLightOn = true;
bool directionalLightOn = true;
bool SpotLightOn = true;
bool AmbientON = true;
bool DiffusionON = true;
bool SpecularON = true;
bool ambientToggle = true;
bool diffuseToggle = true;
bool specularToggle = true;

//birds eye
bool birdEye = false;
glm::vec3 cameraPos(3.5f, 5.0f, 6.0f);
glm::vec3 target(3.5f, 0.1f, 3.0f);  
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
// lighting
//directional light
bool directionLightOn = true;
bool directionalAmbient = true;
bool directionalDiffuse = true;
bool directionalSpecular = true;

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
    glm::vec3(3.0f,  3.0f,  4.0f),
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

    Shader lightingShader(
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\vertexShaderForGouraudShading.vs", 
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\fragmentShaderForGouraudShading.fs"
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

    Cone cone = Cone();

    unsigned int VBO, VAO, EBO;
    initBinding(VAO, VBO, EBO, ourShader, cube_vertices, sizeof(cube_vertices), cube_indices, sizeof(cube_indices));

    //second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    float r = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //lighting
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

        //handle for changes in directional light directly from shedder
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
            if (directionLightOn) {
                lightingShader.setBool("ambientLight", !directionalAmbient);
                directionalAmbient = !directionalAmbient;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
            if (directionLightOn) {
                lightingShader.setBool("diffuseLight", !directionalDiffuse);
                directionalDiffuse = !directionalDiffuse;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
            if (directionLightOn) {
                lightingShader.setBool("specularLight", !directionalSpecular);
                directionalSpecular = !directionalSpecular;
            }
        }

        glm::mat4 projection(0.0f);
        projection[0][0] = 1.0f / (aspect * tanHalfFOV);
        projection[1][1] = 1.0f / tanHalfFOV;
        projection[2][2] = -(far + near) / (far - near);
        projection[2][3] = -1.0f;
        projection[3][2] = -(2.0f * far * near) / (far - near);
        //pass projection matrix to shader (note that in this case it could change every frame)
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::ortho(-2.0f, +2.0f, -1.5f, +1.5f, 0.1f, 100.0f);
        //ourShader.setMat4("projection", projection);
        //constantShader.setMat4("projection", projection);
        lightingShader.setMat4("projection", projection);

        //camera view transformation
        //constantShader.setMat4("view", view);
        //ourShader.setMat4("view", view);

        glm::mat4 view;
        


        //define matrices and vectors needed
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;
        glm::vec3 color;
        
        //initialize all elements as non-emissive
        lightingShader.setVec3("material.emissive", glm::vec3(0.0f, 0.0f, 0.0f));
        //lighting above


        // glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        constantShader.setMat4("projection", projection);

        // camera/view transformation
        //glm::mat4 view;

        if (birdEye) {
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            view = glm::lookAt(cameraPos, target, up);
        }
        else {
            view = camera.GetViewMatrix();
        }
        
        lightingShader.setMat4("view", view);
        ourShader.setMat4("view", view);
        //constantShader.setMat4("view", view);
        // glm::mat4 identityMatrix = glm::mat4(1.0f);
        // glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered, translateMatrixprev;
        // translateMatrix = identityMatrix;
        
        //initialize all elements as non-emissive
        lightingShader.setVec3("material.emissive", glm::vec3(0.0f, 0.0f, 0.0f));
    
        // drawing
        drawAll(lightingShader, VAO, identityMatrix);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.5f, 1.6f, 4.5f));
        model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        ourShader.setMat4("model", model);
        ourShader.setVec3("color", glm::vec3(0.8f, 0.8f, 0.8f));
        cone.drawCone(lightingShader, model);
        // drawing above

        //light holder 1 with emissive material property
        translateMatrix = glm::translate(identityMatrix, glm::vec3(2.08f, 3.5f, 2.08f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.04f, -1.5f, 0.04f));
        model = translateMatrix * scaleMatrix;
        color = glm::vec3(0.5f, 0.0f, 0.0f);

        lightingShader.setVec3("material.ambient", color);
        lightingShader.setVec3("material.diffuse", color);
        lightingShader.setVec3("material.specular", color);
        lightingShader.setVec3("material.emissive", color);
        lightingShader.setFloat("material.shininess", 32.0f);

        lightingShader.setMat4("model", model);

        // glBindVertexArray(VAO);
        // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // //light holder 2 with emissive material property
        // translateMatrix = glm::translate(identityMatrix, glm::vec3(2.08f, 3.5f, 8.08f));
        // scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.04f, -1.5f, 0.04f));
        // model = translateMatrix * scaleMatrix;
        // color = glm::vec3(0.0f, 0.0f, 0.5f);

        // lightingShader.setVec3("material.ambient", color);
        // lightingShader.setVec3("material.diffuse", color);
        // lightingShader.setVec3("material.specular", color);
        // lightingShader.setVec3("material.emissive", color);
        // lightingShader.setFloat("material.shininess", 32.0f);

        // lightingShader.setMat4("model", model);

        // glBindVertexArray(VAO);
        // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // //draw the lamp object(s)
        // ourShader.use();
        // ourShader.setMat4("projection", projection);
        // ourShader.setMat4("view", view);

        // //we now draw as many light bulbs as we have point lights.
        // glBindVertexArray(lightCubeVAO);

        for (unsigned int i = 0; i < 2; i++)
        {
            // drawCube(lightingShader, VAO, identityMatrix, 
            //     pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z,
            //     0,0,0, 
            //     .2,.2,.2,
            //     1,1,1,
            //     100
            // );
            // translateMatrix = translate(identityMatrix, pointLightPositions[i]);
            // scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
            // model = translateMatrix * scaleMatrix;
            // ourShader.setMat4("model", model);
            // ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

            r = 0;
            translateMatrix = glm::translate(identityMatrix, glm::vec3(2,2,2));//pointLightPositions[i]);//::vec3(-0.2, 0.0, -0.2));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(.2f, 0.2f, 0.2f));
            rotateYMatrix = glm::rotate(identityMatrix, glm::radians(r), glm::vec3(0.0, 0.0, 0.0));
            model = translateMatrix * rotateYMatrix * translateMatrix * scaleMatrix;
            lightingShader.setMat4("model", model);
            lightingShader.setVec4("color", glm::vec4(0.5, 0.6, 0.5, 1.0));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        //glfw swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    safeTerminate(VAO, VBO, EBO);
    return 0;
}

float r = 0.0f;

int drawAll(Shader ourShader, unsigned int VAO, glm::mat4 identityMatrix){
    // floor
    drawCube(ourShader, VAO, identityMatrix, 0,0,0, 0,0,0, 6,.1,6, 0.65, 0.70, 0.73, 32.0);

    // ceiling
    drawCube(ourShader, VAO, identityMatrix, 0,5,0, 0,0,0, 6,.1,6, 0.8, 0.80, 0.80,32.0);
    
    // right wall
    drawCube(ourShader, VAO, identityMatrix, 0,.1, -.05, 0,0,0,  6,5,.1, 128/255.0, 128/255.0, 128/255.0,32.0);
    // left wall
    drawCube(ourShader, VAO, identityMatrix, -0.05,.1, 0, 0,0,0,  .1,5,6, 255/255.0, 200/255.0, 220/255.,32.00);

    // right shelf
    drawCube(ourShader, VAO, identityMatrix, 0.1, 1.5, .1, 0,0,0, 4, .1, 1.2, 227/255.0, 193/255.0, 166/255.,32.00);
    // left shelf
    drawCube(ourShader, VAO, identityMatrix, 0.1, 1.5, .1, 0,0,0, 1.2, .1, 5.9, 227/255.0, 193/255.0, 166/255.,32.00);
    
    // left wall shelf
    int total = 4;
    for(int i=0; i<total; i++){
        float gap = (1 / 10.0);
        float width = .8;

        drawCube( ourShader, VAO, identityMatrix, 0, 2.5, (i * width + i*gap), 
            0,0,0, .6,1, width, 241/255.0, 112/255.0, 4/255.0,32.0
        );
        
        if(i == total-1) continue;
        drawCube( ourShader, VAO, identityMatrix, 0, 2.5, (i * width + i*gap) + width,
            0,0,0, .6,1,gap, 162/255.0, 52/255.0, 0/255.0, 32
        );
    }
    // right wall shelf
    drawCube( ourShader, VAO, identityMatrix, .65, 2.5,0, 0,0,0, .8,1,.6,  241/255.0, 112/255.0, 4/255.0,32.0 );
    // right wall shelf white
    drawCube( ourShader, VAO, identityMatrix, .65, 2.55,.6, 0,0,0, .7,.9,.05,  212/255.0, 164/255.0, 141/255.0,32.0 );
    
    // right wall window?
    drawCube( ourShader, VAO, identityMatrix, 2, 2, .1, 0,0,0, 2,1.5,.1,  241/255.0, 112/255.0, 4/255.0,32.0 );
    // right wall window? white
    drawCube( ourShader, VAO, identityMatrix, 2.05, 2.05, .15, 0,0,0, .9,1.4, .1,  212/255.0, 164/255.0, 141/255.0,32.0 );
    drawCube( ourShader, VAO, identityMatrix, 3.05, 2.05, .15, 0,0,0, .9,1.4, .1,  212/255.0, 164/255.0, 141/255.0,32.0 );

    // lower shelf left
    total = 6;
    for(int i=0; i<total; i++){
        float gap = (1 / 10.0);
        float width = .8;

        drawCube( ourShader, VAO, identityMatrix, 0, 0, .5+(i * width + i*gap), 
            0,0,0, 1.2,1.5, width, 241/255.0, 112/255.0, 4/255.0,32.0
        );
        
        if(i == total-1) continue;
        drawCube( ourShader, VAO, identityMatrix, 0, 0, .5+(i * width + i*gap) + width,
            0,0,0, 1.2,1.5,gap, 162/255.0, 52/255.0, 0/255.0,32.0
        );
    }

    // right wall shelf bottom
    total = 4;
    for(int i=0; i<total; i++){
        float gap = (1 / 10.0);
        float width = .6;
        
        drawCube( ourShader, VAO, identityMatrix,  1.2 + (i*width + i*gap), 0, 0, 
            0,0,0, width, 1.5, 1.2,  240/255.0, 108/255.0, 36/255.0,32.0
        );
        
        if(i == total-1) continue;
        drawCube( ourShader, VAO, identityMatrix,  1.2 + (i*width + i*gap + width), 0, 0, 
            0,0,0, gap, 1.5, 1.2, 162/255.0, 52/255.0, 0/255.0,32.0
        );
    }

    // fridge
    drawCube( ourShader, VAO, identityMatrix,  4,0,0, 0,0,0, 2, 3.5, 1.5,  178/255.0, 157/255.0, 136/255.0,32.0);
    drawCube( ourShader, VAO, identityMatrix, 4.05,0,1.5, 0,0,0, .95,3.5, .05,  163/255.0, 143/255.0, 88/255.0 ,32.0);
    drawCube( ourShader, VAO, identityMatrix, 5.05,0,1.5, 0,0,0, .95,3.5, .05,  163/255.0, 143/255.0, 88/255.0 ,32.0);
    // fridge handle
    drawCube( ourShader, VAO, identityMatrix, 4.9,1.5,1.55, 0,0,0, .05,1.1, .05,  20/255.0, 20/255.0, 20/255.0 ,32.0);
    drawCube( ourShader, VAO, identityMatrix, 5.1,1.5,1.55, 0,0,0, .05,1.1, .05,  20/255.0, 20/255.0, 20/255.0 ,32.0);

    // table-top
    drawCube( ourShader, VAO, identityMatrix, 3, 1.5, 4, 0,0,0, 2, .1, 1.5,  25/255.0, 21/255.0, 18/255.0 ,32.0);
    // left top leg
    drawCube( ourShader, VAO, identityMatrix, 3, 0, 4, 0,0,0, .1, 1.5, .1,  255/255.0, 21/255.0, 18/255.0 ,32.0);
    // right top leg
    drawCube( ourShader, VAO, identityMatrix, 4.9, 0, 4, 0,0,0, .1, 1.5, .1,  255/255.0, 21/255.0, 18/255.0 ,32.0);
    // left bottom leg
    drawCube( ourShader, VAO, identityMatrix, 3, 0, 5.4, 0,0,0, .1, 1.5, .1,  255/255.0, 21/255.0, 18/255.0 ,32.0);
    // right bottom leg
    drawCube( ourShader, VAO, identityMatrix, 4.9, 0, 5.4, 0,0,0, .1, 1.5, .1,  255/255.0, 21/255.0, 18/255.0 ,32.0);

    for(int z=0; z<2; z++){
        for(int x=0; x<2; x++){
            float width = 0.5;
            float gap = 0.4;
            int zz = (z == 0) ? 1 : 0;

            // chairs
            drawCube( ourShader, VAO, identityMatrix, (width+gap)*x+3.2,.8,(z*2+3), 0,0,0, .5, .1, .5,  75/255.0, 62/255.0, 53/255.0 ,32.0);
            // left top leg
            drawCube( ourShader, VAO, identityMatrix, (width+gap)*x+3.2,0,(z*2+3), 0,0,0, .1, (.8 + zz*.7), .1,  75/255.0, 62/255.0, 53/255.0 ,32.0);

            
            // left left leg
            drawCube( ourShader, VAO, identityMatrix, (width+gap)*x+3.33,0.9,(z*2+3+.5*z - z*.1), 0,0,0, .05, .6, .1,  75/255.0, 62/255.0, 53/255.0 ,32.0);
            // left mid leg
            drawCube( ourShader, VAO, identityMatrix, (width+gap)*x+3.44,0.9,(z*2+3+.5*z - z*.1), 0,0,0, .05, .6, .1,  75/255.0, 62/255.0, 53/255.0 ,32.0);
            // left right leg
            drawCube( ourShader, VAO, identityMatrix, (width+gap)*x+3.55,0.9,(z*2+3+.5*z - z*.1), 0,0,0, .05, .6, .1,  75/255.0, 62/255.0, 53/255.0 ,32.0);


            // right top leg
            drawCube( ourShader, VAO, identityMatrix, (width+gap)*x+3.6,0,(z*2+3), 0,0,0, .1, (.8+.7*zz), .1,  75/255.0, 62/255.0, 53/255.0 ,32.0);

            zz = !zz;
            // left bottom leg
            drawCube( ourShader, VAO, identityMatrix, (width+gap)*x+3.2,0,(z*2+3.4), 0,0,0, .1, (.8+.7*zz), .1,  75/255.0, 62/255.0, 53/255.0 ,32.0);
            // right bottom leg
            drawCube( ourShader, VAO, identityMatrix, (width+gap)*x+3.6,0,(z*2+3.4), 0,0,0, .1, (.8+.7*zz), .1,  75/255.0, 62/255.0, 53/255.0 ,32.0);
        }
    }

    // tap
    drawCube( ourShader, VAO, identityMatrix,  2,1.6,.1, 0,0,0, 1.5,.02,1.3,  24/255.0, 21/255.0, 22/255.0,32.0);
    total = 20;
    float unit = (1.3 / (2*total));
    for(int z=0; z<total; z++){
        drawCube( ourShader, VAO, identityMatrix,  2,1.63, (z+1)*2*unit, 0,0,0, 1,.01,unit/2,  60/255.0, 60/255.0, 60/255.0,32.0);
    }
    // the real tap
    drawCube( ourShader, VAO, identityMatrix,  3.2,1.6,.3, 0,0,0, .05,.5,.05,  200/255.0, 200/255.0, 200/255.0,32.0);
    drawCube( ourShader, VAO, identityMatrix,  3.2,2.1,.3, 0,0,0, .05,.05,.3,  200/255.0, 200/255.0, 200/255.0,32.0);
    drawCube( ourShader, VAO, identityMatrix,  3.2,2.0,.6, 0,0,0, .05,.2,.05,  200/255.0, 200/255.0, 200/255.0,32.0);

    // microwave
    drawCube(ourShader, VAO, identityMatrix, 0.1, 1.6, 4, 0,0,0, .8, .5, 1.2, 154/255.0, 134/255.0, 108/255.0,32.0);
    drawCube(ourShader, VAO, identityMatrix, 0.9, 1.6, 4.35, 0,0,0, .01, .5, .8, 20/255.0, 20/255.0, 20/255.0,32.0);

    total = 15;
    unit = (.5 / (2*total));
    for(int z=0; z<total; z++){
        drawCube(ourShader, VAO, identityMatrix, 0.9, 1.6+(z+1)*2*unit, 4.05, 0,0,0, .01, unit/4, .3, 255/255.0, 255/255.0, 255/255.0,32.0);
    }

    // fan, 6, 5, 6
    //on = true;
    if (on){
        r += 1;
    }
    else
    {
        r = 0.0f;
    }

    glm::mat4 translateMatrix, scaleMatrix, model, translateMatrixprev, rotateYMatrix;
    //fan stick
    translateMatrix = glm::translate(identityMatrix, glm::vec3(3.0, 4.0, 3.0));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 0.9f, 0.1));
    model = translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("color", glm::vec4(0.0, 0.0, 0.0, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // fan rotation
    translateMatrixprev = translateMatrix;
    glm::mat4 translateMatrix2, translateMatrixBack, test;

    translateMatrix2 = glm::translate(identityMatrix, glm::vec3(3.025, 4.0, 3.025));
    translateMatrixBack = glm::translate(identityMatrix, glm::vec3(-3.025, -4.0, -3.02));
    rotateYMatrix = glm::rotate(identityMatrix, glm::radians(r), glm::vec3(0.0, 1.0, 0.0));
    model = translateMatrixBack * rotateYMatrix * translateMatrix2;
    //drawFan(VAO, ourShader, translateMatrix, rotateYMatrix);

    // basket
    drawCylinder(ourShader, VAO, glm::mat4(1.0f), 
            5.2, 0, 2.5,
            .5, 1,        // Radius and height
            144,
            120/255.0, 34/255.0, 36/255.0);

    
    drawCone(ourShader, VAO, glm::mat4(1.0f),
            5.2, 1.5f, 2.5f,  // Center position
            1.0f, 1.5f,        // Radius and height
            144,
            120/255.0, 34/255.0, 36/255.0);
    
    // drawFilledCircle(ourShader, VAO, glm::mat4(1.0f),
    //             5.2, 1.5f, 2.5f,  // Center position
    //             1.0f,              // Radius
    //             10, 10,             // Segments per ring, number of rings
    //             120/255.0, 34/255.0, 36/255.0);



    return 0;
}

void drawCone(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
              float centerX, float centerY, float centerZ,
              float radius, float height, int segments,
              float r, float g, float b) {
    
    // Draw cone using points and trinagles
    for (int i = 0; i < segments; ++i) {
        float angle = glm::radians(i * 360.0f / segments);
        float nextAngle = glm::radians((i + 1) * 360.0f / segments);

        // Calculate position of the segment on the circle
        float posX = centerX + radius * cos(angle);
        float posZ = centerZ + radius * sin(angle);

        // Calculate position of the next segment on the circle
        float nextPosX = centerX + radius * cos(nextAngle);
        float nextPosZ = centerZ + radius * sin(nextAngle);

        // Cube dimensions
        float cubeWidth = .02;
        float cubeHeight = height / segments;
        float cubeDepth = 0.02;

        // Calculate the rotation of the cube
        float rotY = glm::degrees(atan2(nextPosZ - posZ, nextPosX - posX));

        drawCube(shaderProgram, VAO, parentTrans,
                 posX, centerY + i * cubeHeight, posZ,   // Position
                 0.0f, rotY, 0.0f,     // Rotation
                 cubeWidth, cubeHeight, cubeDepth, // Scale
                 r, g, b);             // Color
    }

}

void drawCylinder(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
                  float centerX, float centerY, float centerZ,
                  float radius, float height, int segments,
                  float r, float g, float b) {
    // Angle step for each segment
    float angleStep = 360.0f / segments;

    for (int i = 0; i < segments; ++i) {
        float angle = glm::radians(i * angleStep);
        float nextAngle = glm::radians((i + 1) * angleStep);

        // Calculate position of the segment on the circle
        float posX = centerX + radius * cos(angle);
        float posZ = centerZ + radius * sin(angle);

        // Cube dimensions
        float cubeWidth = .02;
        float cubeHeight = height;
        float cubeDepth = 0.02;

        float rotY = glm::degrees(atan2(sin(angle), cos(angle)));

        drawCube(shaderProgram, VAO, parentTrans,
                 posX, centerY, posZ,   // Position
                 0.0f, rotY, 0.0f,     // Rotation
                 cubeWidth, cubeHeight, cubeDepth, // Scale
                 r, g, b);             // Color
    }
}

void drawFilledCircle(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
                      float centerX, float centerY, float centerZ,
                      float radius, int segmentsPerRing, int rings,
                      float r, float g, float b) {
    for (int ring = 0; ring <= rings; ++ring) {
        // Calculate the radius of the current ring
        float ringRadius = (float)ring / rings * radius;

        // Calculate the number of cubes in the ring
        int segmentCount = segmentsPerRing * (ring + 1);

        for (int i = 0; i < segmentCount; ++i) {
            // Calculate the angle for the current segment
            float angle = glm::radians(360.0f / segmentCount * i);

            // Position the cube on the current ring
            float posX = centerX + ringRadius * cos(angle);
            float posZ = centerZ + ringRadius * sin(angle);

            // Scale the cubes to be small for smoother approximation
            float cubeSize = radius / (segmentsPerRing * rings);

            // Draw the cube at the calculated position
            drawCube(shaderProgram, VAO, parentTrans,
                     posX, centerY, posZ,  // Position
                     0.0f, 0.0f, 0.0f,    // Rotation
                     cubeSize, cubeSize, cubeSize, // Scale
                     r, g, b);            // Color
        }
    }
}


void drawFan(unsigned int VAO, Shader ourShader, glm::mat4 translateMatrix, glm::mat4 sm)
{
    glm::mat4 identityMatrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    glm::mat4 rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered, translateMatrixprev;
    glm::mat4 middleTranslate, leftBladeTranslate, frontBladeTranslate, rightBladeTranslate, backBladeTranslate;
    //fan middle part
    //translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(-0.2, -1.5, -0.2));
    middleTranslate = glm::translate(identityMatrix, glm::vec3(-0.2, 0.0, -0.2));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, -0.1f, 0.5));
    model = translateMatrix * sm * middleTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(1.0, 1.0, 1.0, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    translateMatrixprev = translateMatrix;
    //left fan
    //translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(0.0, -0.075, 0.0));
    leftBladeTranslate = glm::translate(identityMatrix, glm::vec3(-0.2, 0.0, -0.2));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(-2.0f, -0.1f, 0.5));
    model = translateMatrix * sm * leftBladeTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(0.5, 0.6, 0.5, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //front fan
    //translateMatrix = translateMatrixprev * glm::translate(identityMatrix, glm::vec3(0.0, -0.075, 0.5));
    frontBladeTranslate = glm::translate(identityMatrix, glm::vec3(-0.2, 0.0, 0.3));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, -0.1f, 2.0));
    model = translateMatrix * sm * frontBladeTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(0.5, 0.6, 0.5, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //right fan
    //translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(0.5, 0.0, 0.0));
    rightBladeTranslate = glm::translate(identityMatrix, glm::vec3(0.25, 0.0, 0.25));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(2.0f, -0.1f, -0.5));
    model = translateMatrix * sm * rightBladeTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(0.5, 0.6, 0.5, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //back fan
    //translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(0.0, 0.0, -0.5));
    backBladeTranslate = glm::translate(identityMatrix, glm::vec3(0.25, 0.0, -0.25));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.5f, -0.1f, -2.0));
    model = translateMatrix * sm * backBladeTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(0.5, 0.6, 0.5, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
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
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    //{
    //    if (pointLightOn)
    //    {
    //        pointlight1.turnOff();
    //        pointlight2.turnOff();
    //        pointlight3.turnOff();
    //        pointlight4.turnOff();
    //        pointLightOn = !pointLightOn;
    //    }
    //    else
    //    {
    //        pointlight1.turnOn();
    //        pointlight2.turnOn();
    //        pointlight3.turnOn();
    //        pointlight4.turnOn();
    //        pointLightOn = !pointLightOn;
    //    }
    //}

    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        if (pointLightOn)
        {
            pointlight1.turnOff();
            pointlight2.turnOff();
            //pointlight3.turnOff();
            //pointlight4.turnOff();
            pointLightOn = !pointLightOn;
        }
        else
        {
            pointlight1.turnOn();
            pointlight2.turnOn();
            //pointlight3.turnOn();
            //pointlight4.turnOn();
            pointLightOn = !pointLightOn;
        }
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        if (directionalLightOn)
        {
            directionalLightOn = !directionalLightOn;
        }
        else
        {
            directionalLightOn = !directionalLightOn;
        }
    }
    
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        if (SpotLightOn)
        {
            SpotLightOn = !SpotLightOn;
        }
        else
        {
            SpotLightOn = !SpotLightOn;
        }
    }
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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
