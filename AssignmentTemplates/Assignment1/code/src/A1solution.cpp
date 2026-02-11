#include "A1solution.h"
#include <iostream>

#define GLEW_STATIC 1

#include <GL/glew.h>    
#include <GLFW/glfw3.h> 

#include <glm/glm.hpp>  


// Debug OpenGL calls
void debug_gl(int place){
    GLenum e = glGetError();
    if(e!=GL_NO_ERROR){
        // std::cout<<"We have an error! "<<place<<" "<<e<<" "<<getErrorString(e)<<std::endl;
        std::cout<<"We have an error! "<<place<<" "<<(void* )e<<std::endl;
    }
}

A1solution::A1solution(const std::string& filename) {

}

void A1solution::run() {
    // i. Open an OpenGL window (similarly with the capsules provided)
    // Initialize GLFW and OpenGL version
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(800, 600, "Comp371 - Lab 01", NULL, NULL); // A1 : Change 800 and 600px
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // ii. Loads and Renders a 3D model in an interactive loop
// Compile and link shaders here ...
    int vertexColorProgram = compileAndLinkShaders(getVertexShaderSource(), getFragmentShaderSource());
    int voronoiProgram = compileAndLinkShaders(getVertexShaderSource2(), getFragmentShaderSource2());

    int shaderProgram = voronoiProgram;

    debug_gl(0);

    unsigned int VAO, VBO, CBO, EBO;
    unsigned int PBO[3];
    createRenderingData(VAO, VBO, CBO, PBO, EBO);

    glViewport(0,0,800,600);

    // Entering Main Loop
    while(!glfwWindowShouldClose(window))
    {
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);

        // iii. Listens to the keyboard and upon oppressing ‘s’ it will toggle between 4 different
        // rendering modes and pressing ‘w’ will toggle between the regular shading and a
        // wireframe mode.

        // Detect inputs
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
            shaderProgram = vertexColorProgram;
        }

        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            shaderProgram = voronoiProgram;
        }
    }

    // Shutdown GLFW
    glfwTerminate();


}

int compileAndLinkShaders(const char* vertexShaderSource, const char* fragmentShaderSource) {}

void createRenderingData(unsigned int& VAO, unsigned int& VBO,unsigned int& CBO, unsigned int PBO[], unsigned int& EBO) {}

