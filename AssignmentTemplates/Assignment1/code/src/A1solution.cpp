#include "A1solution.h"
#include <iostream>
#include <fstream>

#define GLEW_STATIC 1

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Debug OpenGL calls
void debug_gl(int place)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
    {
        // std::cout<<"We have an error! "<<place<<" "<<e<<" "<<getErrorString(e)<<std::endl;
        std::cout << "We have an error! " << place << " " << (void *)e << std::endl;
    }
}

const char *getVertexShaderPhong()
{
    return R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;

    out vec3 FragPos;
    out vec3 Normal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;  
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
    )";
}

const char *getFragmentShaderPhong()
{
    return R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;

    out vec4 FragColor;

    uniform vec3 lightPos;
    uniform vec3 viewPos;

    void main()
    {
        // Ambient
        vec3 ambient = vec3(0.1, 0.05, 0.05);

        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 0.5, 0.5);

        // Specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5.0);
        vec3 specular = spec * vec3(0.3, 0.3, 0.3);

        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
    )";
}

int compileAndLinkShaders(const char *vertexShaderSource, const char *fragmentShaderSource)
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    return shaderProgram;
}

void createRenderingData(unsigned int &VAO, unsigned int &VBO, unsigned int &CBO, unsigned int PBO[], unsigned int &EBO)
{

    // Define and upload geometry to the GPU here ...
    // create VAO + VBO
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f};

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0); // position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float))); // normal
    glEnableVertexAttribArray(1);

    unsigned int indices[] = {0, 1, 2};
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

// Shader program IDs
int currentShader = 0;
int shaderPrograms[2];

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        currentShader = (currentShader + 1) % 2;
    }
}

void A1solution::run(char *filename)
{
    std::ifstream in(filename);

    // Read modelview matrix
    glm::mat4 modelview;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            in >> modelview[i][j];
        }
    }

    // Read projection matrix
    glm::mat4 projection;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            in >> projection[i][j];
        }
    }

    // Read width and height
    int width, height;
    in >> width >> height;

    // Read vertices
    int N;
    in >> N;
    std::vector<glm::vec3> vertices(N);
    for (int i = 0; i < N; i++)
    {
        in >> vertices[i].x >> vertices[i].y >> vertices[i].z;
    }

    // Read triangles
    int M;
    in >> M;
    std::vector<glm::vec3> triangles(M);
    for (int i = 0; i < M; i++)
    {
        in >> triangles[i].x >> triangles[i].y >> triangles[i].z;
    }

    // i. Open an OpenGL window (similarly with the capsules provided)
    // Initialize GLFW and OpenGL version
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow *window = glfwCreateWindow(width, height, "Comp371 - Assignment 01", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        // return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        // return -1;
    }
    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // ii. Loads and Renders a 3D model in an interactive loop
    // Compile and link shaders here ...
    int vertexColorProgram = compileAndLinkShaders(getVertexShaderPhong(), getFragmentShaderPhong());
    // int voronoiProgram = compileAndLinkShaders(getVertexShaderVoronoi(), getFragmentShaderVoronoi());

    int shaderPrograms[] = {vertexColorProgram};
    int currentShader = 0;

    debug_gl(0);

    unsigned int VAO, VBO, CBO, EBO;
    unsigned int PBO[3];
    createRenderingData(VAO, VBO, CBO, PBO, EBO);

    glViewport(0, 0, 800, 600);

    // S key press for shader mode
    bool sKeyPressedLastFrame = false;
    // W key press for wireframe mode
    bool wKeyPressedLastFrame = false;

    // Entering Main Loop
    while (!glfwWindowShouldClose(window))
    {
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderPrograms[currentShader]);

        unsigned int modelLoc = glGetUniformLocation(shaderPrograms[currentShader], "model");
        unsigned int viewLoc = glGetUniformLocation(shaderPrograms[currentShader], "view");
        unsigned int projLoc = glGetUniformLocation(shaderPrograms[currentShader], "projection");

        glm::mat4 model = glm::mat4(1.0f);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &modelview[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glUniform3f(glGetUniformLocation(shaderPrograms[currentShader], "lightPos"), 1.0f, 1.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shaderPrograms[currentShader], "viewPos"), 0.0f, 0.0f, 2.0f);

        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);

        // Detect inputs
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        // Listens to the keyboard and upon oppressing ‘s’ it will toggle between 4 different
        // rendering modes and

        // pressing ‘s’ it will toggle between 4 different rendering modes
        bool sKeyPressedNow = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

        if (sKeyPressedNow && !sKeyPressedLastFrame)
        {
            currentShader = (currentShader + 1) % 2; // change to 4
        }
        sKeyPressedLastFrame = sKeyPressedNow;

        // pressing ‘w’ will toggle between the regular shading and a
        // wireframe mode.

        bool wKeyPressedNow = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;

        if (wKeyPressedNow && !wKeyPressedLastFrame)
        {
            // currentShader = (currentShader + 1) % 2; // change to 4
        }
    }

    // Shutdown GLFW
    glfwTerminate();
}
