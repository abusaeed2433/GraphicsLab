#define GLFW_DLL

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// #include "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\opengl-cpp-template\\include\\glad\\glad.h"
// #include "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\opengl-cpp-template\\include\\GLFW\\glfw3.h"
#include <set>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float translate_X = 0.0;
float translate_Y = 0.0;
float rotateAngle = 0.0;
float scale_X = 1.0;
float scale_Y = 1.0;

float translate_X_Cream = 0.0f, translate_Y_Cream = 0.0f, rotateAngle_Cream = 0.0f, scale_X_Cream = 1.0f, scale_Y_Cream = 1.0f;
float translate_X_Cone = 0.0f, translate_Y_Cone = 0.0f, rotateAngle_Cone = 0.0f, scale_X_Cone = 1.0f, scale_Y_Cone = 1.0f;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = transform * vec4(aPos, 1.0);\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 color;\n"  // Define the color uniform
"void main()\n"
"{\n"
"   FragColor = vec4(color, 1.0f);\n"  // Use the color uniform here
"}\n\0";



using namespace std;



float** readPoints(const std::string& filename, int& numLists, int*& vertexCounts) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return nullptr;
    }

    std::vector<std::vector<float>> nestedPoints; // Store each list of points as a vector
    std::vector<float> points;                    // Current list of points
    std::string line;

    int lineCount = 0;

    while (std::getline(file, line)) {
        if (line == "=") {
            lineCount++;
            // End of a nested list, add current points to nestedPoints
            if (!points.empty()) {

                if(lineCount == 3){ // calculate the centroid
                    float x = 0, y = 0;
                    for(int i = 0; i < points.size(); i+=3){
                        x += points[i];
                        y += points[i+1];
                    }
                    x /= points.size()/3;
                    y /= points.size()/3;
                    points[0] = x, points[1] = y, points[2] = 0.0f;
                    // points.push_back(x);
                    // points.push_back(y);
                    // points.push_back(0.0f);
                }

                nestedPoints.push_back(points);
                points.clear(); // Reset for the next list
            }
        } else {
            std::istringstream iss(line);
            float x, y;
            char ch;
            if (iss >> x >> ch >> y && ch == ',') {
                points.push_back(x*0.6f);
                points.push_back(y);
                points.push_back(0.0f); // z-coordinate
            }
        }
    }

    // Add the last set of points, if any, after the last "=" line
    if (!points.empty()) {
        nestedPoints.push_back(points);
    }

    // Convert nestedPoints to a 2D float array
    numLists = nestedPoints.size();
    vertexCounts = new int[numLists]; // Track vertex count for each list
    float** vertexArrays = new float*[numLists];

    for (int i = 0; i < numLists; ++i) {
        int size = nestedPoints[i].size();
        vertexCounts[i] = size; // Total number of floats in each list
        vertexArrays[i] = new float[size];

        // Copy points to 2D array
        for (int j = 0; j < size; ++j) {
            vertexArrays[i][j] = nestedPoints[i][j];
        }
    }

    return vertexArrays;
}

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

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { cout << "Failed to initialize GLAD" << endl; return -1; }

    // build and compile our shader program
    return 0;
}

int initVertexShader(unsigned int &vertexShader){
    // vertex shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) { 
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog); 
        cout << "VERTEX::COMPILATION_FAILED\n" << infoLog << endl; 
        return -1;
    }
    return 0;
}

int initFragmentShader(unsigned int &fragmentShader){

    int success;
    char infoLog[512];
    
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    
    if (!success){ 
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog); 
        cout << "FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl; 

        return -1;
    }
    return 0;
}

int initLinkShader(unsigned int &shaderProgram, unsigned int vertexShader, unsigned int fragmentShader){
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    int success;
    char infoLog[512];
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) { 
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog); 
        cout << "LINKING_FAILED\n" << infoLog << endl; 
        return -1;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return 0;
}

void safeTerminate(unsigned int& VAO, unsigned int& VBO, unsigned int& shaderProgram){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void initBinding(unsigned int &VAO, unsigned int &VBO){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
}

int main()
{
    GLFWwindow* window = nullptr;
    if( initGlfw(window) ) return -1;

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if( initVertexShader(vertexShader) ) return -1;

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if( initFragmentShader(fragmentShader) ) return -1;
    
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    if( initLinkShader(shaderProgram, vertexShader, fragmentShader) ) return -1;

    int numLists;
    int* vertexCounts;
    float** vertices = readPoints(
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\opengl-cpp-template\\src\\points.txt",
        numLists, vertexCounts
    );
    if (vertices == nullptr) return -1;

    // Set up the VAO and VBO
    unsigned int VBO, VAO;
    initBinding(VAO, VBO);

    set<int> fillSet = {0,1,2,3,4};
    float colors[][3] = {
        {0.12, 0.12, 0.12}, // background
        {40/255.0, 0/255.0, 189/255.0}, // blue
        {183/255.0, 199/255.0, 6/255.0}, // top yellow
        {26/255.0, 161/255.0, 5/255.0}, // green
        {219/255.0, 44/255.0, 173/255.0}, // pink
        {204/255.0, 112/255.0, 4/255.0} // muddy
    };

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Input
        processInput(window);

        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the shader program
        glm::mat4 translationMatrix;
        glm::mat4 rotationMatrix;
        glm::mat4 scaleMatrix;
        glm::mat4 modelMatrix;
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        translationMatrix = glm::translate(identityMatrix, glm::vec3(translate_X, translate_Y , 0.0f));
        rotationMatrix = glm::rotate(identityMatrix, glm::radians(rotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(scale_X, scale_Y, 1.0f));
        modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        //modelMatrix = rotationMatrix * scaleMatrix;

        glm::mat4 translationMatrixCream;
        glm::mat4 rotationMatrixCream;
        glm::mat4 scaleMatrixCream;
        glm::mat4 modelMatrixCream;

        translationMatrixCream = glm::translate(modelMatrix, glm::vec3(translate_X_Cream, translate_Y_Cream , 0.0f));
        rotationMatrixCream = glm::rotate(modelMatrix, glm::radians(rotateAngle_Cream), glm::vec3(0.0f, 0.0f, 1.0f));
        scaleMatrixCream = glm::scale(modelMatrix, glm::vec3(scale_X_Cream, scale_Y_Cream, 1.0f));
        modelMatrixCream = translationMatrixCream * rotationMatrixCream * scaleMatrixCream;

        glm::mat4 translationMatrixCone;
        glm::mat4 rotationMatrixCone;
        glm::mat4 scaleMatrixCone;
        glm::mat4 modelMatrixCone;

        translationMatrixCone = glm::translate(modelMatrix, glm::vec3(translate_X_Cone, translate_Y_Cone , 0.0f));
        rotationMatrixCone = glm::rotate(modelMatrix, glm::radians(rotateAngle_Cone), glm::vec3(0.0f, 0.0f, 1.0f));
        scaleMatrixCone = glm::scale(modelMatrix, glm::vec3(scale_X_Cone, scale_Y_Cone, 1.0f));
        modelMatrixCone = translationMatrixCone * rotationMatrixCone * scaleMatrixCone;
        
        // get matrix's uniform location and set matrix
        glUseProgram(shaderProgram);
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        // transformation above
        
        glBindVertexArray(VAO);
        int colorLocation = glGetUniformLocation(shaderProgram, "color");

        // Draw each sublist separately
        for (int i = 0; i < numLists; ++i) {

            if(i < 1){
                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrixCone));
            }
            else{
                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrixCream));
            }
            
            int index = (i >= 4) ? 5 : i;
            
            glUniform3f(colorLocation, colors[index][0], colors[index][1], colors[index][2]);
            if(i == numLists-1){
                glUniform3f(colorLocation, colors[4][0], colors[4][1], colors[4][2]);
            }

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertexCounts[i] * sizeof(float), vertices[i], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // Draw lines for each sublist (vertex count / 3 because each point has 3 floats)
            if(fillSet.find(i) != fillSet.end() || i >= 5){
                glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCounts[i] / 3);
            }
            else{
                glDrawArrays(GL_LINE_STRIP, 0, vertexCounts[i] / 3);
            }
        }

        // Unbind for safety
        glBindVertexArray(0);
        // GLFW: swap buffers and poll IO events (keys pressed/released, mouse moved, etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup memory
    for (int i = 0; i < numLists; ++i) {
        delete[] vertices[i];
    }
    delete[] vertices;
    delete[] vertexCounts;
    safeTerminate(VAO, VBO, shaderProgram);
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    float unit = 0.001;
    float scaleFactor = 0.01f;
    float rotate_angle = 20;

    if ( glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if( glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ) // translate-x negative
        translate_X -= unit;
    else if( glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ) // translate-x positive
        translate_X += unit;
    else if( glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ) // translate-y negative
        translate_Y -= unit;
    else if( glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS ) // translate-y positive
        translate_Y += unit;
    else if( glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS ){ // scale up
        scale_X += scaleFactor;
        scale_Y += scaleFactor;
    }
    else if( glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS ){ // scale down
        scale_X -= scaleFactor;
        scale_Y -= scaleFactor;
    }
    else if( glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS ) // rotate clockwise
        rotateAngle += rotate_angle * 3.14/180;
    else if( glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS ) // rotate counter-clockwise
        rotateAngle -= rotate_angle * 3.14/180;

    // for cone
    else if( glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ) // translate-x negative
        translate_X_Cone -= unit;
    else if( glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ) // translate-x positive
        translate_X_Cone += unit;
    else if( glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ) // translate-y negative
        translate_Y_Cone -= unit;
    else if( glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS ) // translate-y positive
        translate_Y_Cone += unit;
    else if( glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS ){ // scale up
        scale_X_Cone += scaleFactor;
        scale_Y_Cone += scaleFactor;
    }
    else if( glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS ){ // scale down
        scale_X_Cone -= scaleFactor;
        scale_Y_Cone -= scaleFactor;
    }
    else if( glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS ) // rotate clockwise
        rotateAngle_Cone += rotate_angle * 3.14/180;
    else if( glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS ) // rotate counter-clockwise
        rotateAngle_Cone -= rotate_angle * 3.14/180;

    // for cream
    else if( glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS ) // translate-x negative
        translate_X_Cream -= unit;
    else if( glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS ) // translate-x positive
        translate_X_Cream += unit;
    else if( glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS ) // translate-y negative
        translate_Y_Cream -= unit;
    else if( glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS ) // translate-y positive
        translate_Y_Cream += unit;
    else if( glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS ){ // scale up
        scale_X_Cream += scaleFactor;
        scale_Y_Cream += scaleFactor;
    }
    else if( glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS ){ // scale down
        scale_X_Cream -= scaleFactor;
        scale_Y_Cream -= scaleFactor;
    }
    else if( glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS ) // rotate clockwise
        rotateAngle_Cream += rotate_angle * 3.14/180;
    else if( glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS ) // rotate counter-clockwise
        rotateAngle_Cream -= rotate_angle * 3.14/180;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
