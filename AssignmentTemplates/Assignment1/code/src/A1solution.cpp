#include "A1solution.h"
#include <iostream>
#include <fstream>

#define GLEW_STATIC 1

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// OpenGL error string conversion
const char *getErrorString(GLenum error)
{
    switch (error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNKNOWN_ERROR";
    }
}

// Debug OpenGL calls
void debug_gl(int place)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
    {
        std::cout << "We have an error! " << place << " " << e << " " << getErrorString(e) << std::endl;
    }
}

const char *getVertexShaderPhong()
{
    return R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;

        out vec3 fragPos;
        out vec3 normal;

        uniform mat4 projection;
        uniform mat4 modelview;
        uniform mat3 normalMat;

        void main()
        {
            vec4 vertPos4 = modelview * vec4(aPos, 1.0);
            fragPos = vertPos4.xyz;
            normal = normalMat * aNormal;     
            gl_Position = projection * vertPos4;
        }
    )";
}

const char *getFragmentShaderPhong()
{
    return R"(
    #version 330 core
    in vec3 fragPos;
    in vec3 normal;

    out vec4 FragColor;

    uniform vec3 lightPos;

    void main()
    {
        vec3 norm = normalize(normal);

        // Ambient
        vec3 ambient = vec3(0.1, 0.05, 0.05);

        // Diffuse
        vec3 lightDir = normalize(lightPos - fragPos);    
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 0.5, 0.5);

        // Specular
        vec3 viewDir = normalize(-fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5.0); // shininess fixed at 5
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

    if (!in.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Read matrices
    glm::mat4 modelview;
    glm::mat4 projection;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            in >> modelview[i][j];
        }
    }

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

    in.close();

    // Compute vertex normals
    std::vector<glm::vec3> vertexNormals(N, glm::vec3(0.0f));
    for (auto &t : triangles) {
        glm::vec3 v0 = vertices[static_cast<int>(t.x)];
        glm::vec3 v1 = vertices[static_cast<int>(t.y)];
        glm::vec3 v2 = vertices[static_cast<int>(t.z)]; 

        glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        vertexNormals[static_cast<int>(t.x)] += faceNormal;
        vertexNormals[static_cast<int>(t.y)] += faceNormal;
        vertexNormals[static_cast<int>(t.z)] += faceNormal;
    }
    for (auto &n : vertexNormals) n = glm::normalize(n);

    // Flatten vertex data for OpenGL
    std::vector<float> vertexData;
    for (size_t i = 0; i < vertices.size(); i++) {
        glm::vec3 &v = vertices[i];
        vertexData.push_back(v.x);
        vertexData.push_back(v.y);
        vertexData.push_back(v.z);

        // Placeholder normals (all pointing +z)
        vertexData.push_back(0.0f);
        vertexData.push_back(0.0f);
        vertexData.push_back(1.0f);
    }

    // Flatten triangle indices for OpenGL
    std::vector<unsigned int> indices;
    for (auto &t : triangles) {
        indices.push_back(static_cast<unsigned int>(t.x));
        indices.push_back(static_cast<unsigned int>(t.y));
        indices.push_back(static_cast<unsigned int>(t.z));
    }

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow *window = glfwCreateWindow(width, height, "Comp371 - Assignment 01", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
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

    glEnable(GL_DEPTH_TEST); // Enable depth testing

    // Grey background
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

    // Compile and load shaders
    int phongShader = compileAndLinkShaders(getVertexShaderPhong(), getFragmentShaderPhong());
    int shaderPrograms[] = {phongShader};
    int currentShader = 0;

    // Upload vertex data to GPU
    unsigned int VAO, VBO, CBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    // Entering Main Loop
    while (!glfwWindowShouldClose(window))
    {
        // Each frame, reset color and depth of each pixel
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderPrograms[currentShader]);

        unsigned int modelLoc = glGetUniformLocation(shaderPrograms[currentShader], "modelview");
        unsigned int projLoc  = glGetUniformLocation(shaderPrograms[currentShader], "projection");
        unsigned int normalLoc = glGetUniformLocation(shaderPrograms[currentShader], "normalMat");
        unsigned int lightLoc = glGetUniformLocation(shaderPrograms[currentShader], "lightPos");

        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelview)));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelview));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

        glUniform3f(lightLoc, 0.0f, 0.0f, 1.0f); // Light position in view space

        // Draw
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Shutdown GLFW
    glfwTerminate();
}
