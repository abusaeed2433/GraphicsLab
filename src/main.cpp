#define GLFW_DLL

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "common/MyClasses.cpp"

#include <unordered_map>
#include <memory>

#include <set>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

#define PI 3.14159265359

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void generateHexagonVertices(float* vertices, float radius, float height);

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

using namespace std;

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

void safeTerminate(unsigned int& VAO, unsigned int& VBO){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
}

void initBinding(unsigned int &VAO, unsigned int &VBO){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
    flush(cout);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
    flush(cout);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}
	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
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

    unsigned int VBO, VAO;
    initBinding(VAO, VBO);
    
    GLuint programID = LoadShaders(
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\vertex_shader.vs",
        "D:\\Documents\\COURSES\\4.2\\Lab\\Graphics\\project\\src\\fragment_shader.fs"
    );
    
    glUseProgram(programID);
    
    unsigned int transformLoc = glGetUniformLocation(programID, "transform");
    unsigned int colorLocation = glGetUniformLocation(programID, "color");

    // Point point1(0.0f, 0.5f); // Top vertex
    // Point point2(-0.5f, -0.5f); // Bottom-left vertex
    // Point point3(0.5f, -0.5f); // Bottom-right vertex
    // Triangle triangle(point1, point2, point3);

    std::vector<std::unique_ptr<Shape>> shapes = readShapes();

    printf("Shape size is: %d\n", shapes.size());
    for(int i=0; i<shapes.size(); i++){
        printf("Drawing shape %d\n", i);
    }
    flush(cout);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);    
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // apply color and transformation
        glm::mat4 translationMatrix, rotationMatrix, scaleMatrix, modelMatrix;
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        translationMatrix = glm::translate(identityMatrix, glm::vec3(translate_X, translate_Y , 0.0f));
        rotationMatrix = glm::rotate(identityMatrix, glm::radians(rotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(scale_X, scale_Y, 1.0f));
        modelMatrix = translationMatrix * rotationMatrix * scaleMatrix; // scale first, then rotate, then translate
        glUniform3f(colorLocation, 1,0,0);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        // draw here
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

        // draw
        for(int i=0; i<shapes.size(); i++){
            shapes[i]->draw(transformLoc, colorLocation, modelMatrix);
        }
        
        glDisableVertexAttribArray(0);
        // swap buffer
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    safeTerminate(VAO, VBO);
    return 0;
}

void generateHexagonVertices(float* vertices, float radius, float height) {
    int vertexIndex = 0;

    // Generate top and bottom face vertices
    for (int i = 0; i < 6; i++) {
        float angle = 2.0f * PI * i / 6.0f;
        float nextAngle = 2.0f * PI * (i + 1) / 6.0f;

        // Center top (0, height, 0)
        vertices[vertexIndex++] = 0.0f;
        vertices[vertexIndex++] = height;
        vertices[vertexIndex++] = 0.0f;

        // Current vertex top
        vertices[vertexIndex++] = radius * cos(angle);
        vertices[vertexIndex++] = height;
        vertices[vertexIndex++] = radius * sin(angle);

        // Next vertex top
        vertices[vertexIndex++] = radius * cos(nextAngle);
        vertices[vertexIndex++] = height;
        vertices[vertexIndex++] = radius * sin(nextAngle);

        // Center bottom (0, -height, 0)
        vertices[vertexIndex++] = 0.0f;
        vertices[vertexIndex++] = -height;
        vertices[vertexIndex++] = 0.0f;

        // Current vertex bottom
        vertices[vertexIndex++] = radius * cos(angle);
        vertices[vertexIndex++] = -height;
        vertices[vertexIndex++] = radius * sin(angle);

        // Next vertex bottom
        vertices[vertexIndex++] = radius * cos(nextAngle);
        vertices[vertexIndex++] = -height;
        vertices[vertexIndex++] = radius * sin(nextAngle);

        // Side faces
        // Triangle 1
        vertices[vertexIndex++] = radius * cos(angle);
        vertices[vertexIndex++] = height;
        vertices[vertexIndex++] = radius * sin(angle);

        vertices[vertexIndex++] = radius * cos(angle);
        vertices[vertexIndex++] = -height;
        vertices[vertexIndex++] = radius * sin(angle);

        vertices[vertexIndex++] = radius * cos(nextAngle);
        vertices[vertexIndex++] = -height;
        vertices[vertexIndex++] = radius * sin(nextAngle);

        // Triangle 2
        vertices[vertexIndex++] = radius * cos(angle);
        vertices[vertexIndex++] = height;
        vertices[vertexIndex++] = radius * sin(angle);

        vertices[vertexIndex++] = radius * cos(nextAngle);
        vertices[vertexIndex++] = -height;
        vertices[vertexIndex++] = radius * sin(nextAngle);

        vertices[vertexIndex++] = radius * cos(nextAngle);
        vertices[vertexIndex++] = height;
        vertices[vertexIndex++] = radius * sin(nextAngle);
    }
}


std::unordered_map<int, bool> keyState;
bool isKeyPressedOnce(GLFWwindow* window, int key) {
    if (glfwGetKey(window, key) == GLFW_PRESS) {
        if (!keyState[key]) {
            keyState[key] = true;
            return true;
        }
    } else {
        keyState[key] = false;
    }
    return false;
}
void processInput(GLFWwindow* window)
{
    float unit = 0.05;
    float scaleFactor = 0.2f;
    float rotate_angle = 30;
    
    if ( glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    else if( isKeyPressedOnce(window, GLFW_KEY_W) == GLFW_PRESS ) // translate-x negative
        translate_X -= unit;
    else if( isKeyPressedOnce(window, GLFW_KEY_Q) == GLFW_PRESS ) // translate-x positive
        translate_X += unit;
    else if( isKeyPressedOnce(window, GLFW_KEY_E) == GLFW_PRESS ) // translate-y negative
        translate_Y -= unit;
    else if( isKeyPressedOnce(window, GLFW_KEY_R) == GLFW_PRESS ) // translate-y positive
        translate_Y += unit;
    else if( isKeyPressedOnce(window, GLFW_KEY_T) == GLFW_PRESS ){ // scale up
        scale_X += scaleFactor;
        scale_Y += scaleFactor;
        std::cout << "Scale Up: scale_X = " << scale_X << ", scale_Y = " << scale_Y << std::flush;
    }
    else if( isKeyPressedOnce(window, GLFW_KEY_Y) == GLFW_PRESS ){ // scale down
        scale_X -= scaleFactor;
        scale_Y -= scaleFactor;
        std::cout << "Scale Down: scale_X = " << scale_X << ", scale_Y = " << scale_Y << std::flush;
    }
    else if( isKeyPressedOnce(window, GLFW_KEY_U) == GLFW_PRESS ) // rotate clockwise
        rotateAngle += rotate_angle * 3.14/180;
    else if( isKeyPressedOnce(window, GLFW_KEY_I) == GLFW_PRESS ) // rotate counter-clockwise
        rotateAngle -= rotate_angle * 3.14/180;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}
