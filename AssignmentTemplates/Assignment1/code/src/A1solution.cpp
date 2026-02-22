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
    uniform vec3 lightColor;

    void main()
    {
        vec3 norm = normalize(normal);

        // Ambient
        vec3 ambient = vec3(0.1, 0.05, 0.05) * lightColor;

        // Diffuse
        vec3 lightDir = normalize(lightPos - fragPos);    
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * vec3(1.0, 0.5, 0.5);

        // Specular
        vec3 viewDir = normalize(-fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5.0);
        vec3 specular = spec * lightColor * vec3(0.3, 0.3, 0.3);

        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
    )";
}

const char *getVertexShaderFlat()
{
    return R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;

        out vec3 fragPos;
        flat out vec3 normal;

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

const char *getFragmentShaderFlat()
{
    return R"(
    #version 330 core
    in vec3 fragPos;
    flat in vec3 normal;

    out vec4 FragColor;

    uniform vec3 lightPos;
    uniform vec3 lightColor;

    void main()
    {
        vec3 norm = normalize(normal);

        // Ambient
        vec3 ambient = vec3(0.1, 0.05, 0.05) * lightColor;

        // Diffuse
        vec3 lightDir = normalize(lightPos - fragPos);    
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * vec3(1.0, 0.5, 0.5);

        // Specular
        vec3 viewDir = normalize(-fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5.0);
        vec3 specular = spec * lightColor * vec3(0.3, 0.3, 0.3);

        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
    )";
}

const char *getVertexShaderCircle()
{
    return R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec3 aBary;

        out vec3 fragPos;
        out vec3 normal;
        out vec3 bary;

        uniform mat4 projection;
        uniform mat4 modelview;
        uniform mat3 normalMat;

        void main()
        {
            vec4 vertPos4 = modelview * vec4(aPos, 1.0);
            fragPos = vertPos4.xyz;
            normal = normalMat * aNormal;     
            bary = aBary;
            gl_Position = projection * vertPos4;
        }
    )";
}

const char *getFragmentShaderCircle()
{
    return R"(
    #version 330 core
    
    in vec3 fragPos;
    in vec3 normal;
    in vec3 bary;

    out vec4 FragColor;

    uniform vec3 lightPos;
    uniform vec3 lightColor;

    void main() 
    {        
        // Distance from centroid in barycentric space
        vec3 d = bary - vec3(1.0/3.0);
        float distFromCenter = dot(d, d); // squared distance

        float radius = 0.17; 
        bool insideCircle = distFromCenter < radius;
        
        vec3 norm = normalize(normal);

        vec3 ambient;
        vec3 diffuseColor;
        vec3 specular = vec3(0.0);

        if (insideCircle) 
        {
            ambient = vec3(0.05, 0.05, 0.1) * lightColor;
            diffuseColor = vec3(0.5, 0.5, 1.0);
        }
        else
        {
            ambient = vec3(0.1, 0.05, 0.05) * lightColor;
            diffuseColor = vec3(1.0, 0.5, 0.5);
        }

        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * diffuseColor;

        if (insideCircle)
        {
            specular = vec3(0.0);
        }
        else 
        {
            vec3 viewDir = normalize(-fragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5.0);
            specular = spec * lightColor * vec3(0.3);
        }

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

// Shader program IDs
int currentShader = 0;
int mode = 0; // 0 for fill, 1 for wireframe

// Key callback function to handle input
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Close window on Escape key press
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Switch shader on S key press
    // if (key == GLFW_KEY_S && action == GLFW_PRESS) 
    // {
    //     std::cout << "Switching shader..." << std::endl;
    //     currentShader = 1 - currentShader;
    //     std::cout << "Switched to " << (currentShader == 0 ? "Phong" : "Flat") << " shader." << std::endl;
    // }

    // Switch to wireframe mode on W key press
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {   
        mode = (mode + 1) % 2;
        if (mode == 0) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  
        }
    }
}

// Function to read input file and populate data structures
void readInputFile(const char *filename, glm::mat4 &modelview, glm::mat4 &projection, int &width, int &height, std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &triangles)
{
    std::ifstream in(filename);

    if (!in.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Read matrices
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
    in >> width >> height;

    // Read vertices
    int N;
    in >> N;
    vertices.resize(N);
    for (int i = 0; i < N; i++)
    {
        in >> vertices[i].x >> vertices[i].y >> vertices[i].z;
    }

    // Read triangles
    int M;
    in >> M;
    triangles.resize(M);
    for (int i = 0; i < M; i++)
    {
        in >> triangles[i].x >> triangles[i].y >> triangles[i].z;
    }

    in.close();
}

void phongShader(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &triangles, std::vector<glm::vec3> &vertexNormals, std::vector<float> &vertexData, std::vector<unsigned int> &indices) {
    vertexData.clear();
    indices.clear();
    vertexNormals.clear();

    vertexNormals.resize(vertices.size(), glm::vec3(0.0f));
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

    vertexData.reserve(vertices.size() * 6);
    for (size_t i = 0; i < vertices.size(); i++) {
        const glm::vec3 &v = vertices[i];
        const glm::vec3 &n = vertexNormals[i];
        vertexData.push_back(v.x);
        vertexData.push_back(v.y);
        vertexData.push_back(v.z);

        vertexData.push_back(n.x);
        vertexData.push_back(n.y);
        vertexData.push_back(n.z);
    }

    for (const auto &t : triangles) {
        indices.push_back(static_cast<unsigned int>(t.x));
        indices.push_back(static_cast<unsigned int>(t.y));
        indices.push_back(static_cast<unsigned int>(t.z));
    }
}

void flatShader(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &triangles, std::vector<glm::vec3> &faceNormals, std::vector<float> &vertexData, std::vector<unsigned int> &indices) {
    vertexData.clear();
    indices.clear();
    faceNormals.clear();

    unsigned int indexCounter = 0;

    for (size_t i = 0; i < triangles.size(); i++)
    {
        // Get triangle vertex indices
        int i0 = static_cast<int>(triangles[i].x);
        int i1 = static_cast<int>(triangles[i].y);
        int i2 = static_cast<int>(triangles[i].z);

        glm::vec3 v0 = vertices[i0];
        glm::vec3 v1 = vertices[i1];
        glm::vec3 v2 = vertices[i2];

        // Compute face normal
        glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        faceNormals.push_back(faceNormal);

        // Duplicate vertices with same normal
        glm::vec3 triangleVerts[3] = {v0, v1, v2};

        for (int j = 0; j < 3; j++)
        {
            const glm::vec3 &v = triangleVerts[j];

            // Position
            vertexData.push_back(v.x);
            vertexData.push_back(v.y);
            vertexData.push_back(v.z);

            // Same face normal for all 3 vertices
            vertexData.push_back(faceNormal.x);
            vertexData.push_back(faceNormal.y);
            vertexData.push_back(faceNormal.z);

            indices.push_back(indexCounter++);
        }
    }
}

void circleShader(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &triangles, std::vector<glm::vec3> &vertexNormals, std::vector<float> &vertexData, std::vector<unsigned int> &indices) {
    vertexData.clear();
    indices.clear();

    for (size_t i = 0; i < triangles.size(); i++) {
        // Get triangle vertices
        int i0 = static_cast<int>(triangles[i].x);
        int i1 = static_cast<int>(triangles[i].y);
        int i2 = static_cast<int>(triangles[i].z);

        glm::vec3 verts[3]   = { vertices[i0], vertices[i1], vertices[i2] };
        glm::vec3 normals[3] = { vertexNormals[i0], vertexNormals[i1], vertexNormals[i2] };

        float barycentricCoordinates[3][3] = {
            {1.0f, 0.0f, 0.0f}, // Entry 1 (1,0,0)
            {0.0f, 1.0f, 0.0f}, // Entry 2 (0,1,0)
            {0.0f, 0.0f, 1.0f}  // Entry 3 (0,0,1)
        };

        for (int j = 0; j < 3; j++) {
            vertexData.push_back(verts[j].x);
            vertexData.push_back(verts[j].y);
            vertexData.push_back(verts[j].z);
            vertexData.push_back(normals[j].x);
            vertexData.push_back(normals[j].y);
            vertexData.push_back(normals[j].z);
            vertexData.push_back(barycentricCoordinates[j][0]);
            vertexData.push_back(barycentricCoordinates[j][1]);
            vertexData.push_back(barycentricCoordinates[j][2]);
        }

        // Add indices for the triangle 
        unsigned int baseIndex = static_cast<unsigned int>(i) * 3;
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
    }

}

void createRenderingData(unsigned int *VAOs, unsigned int *VBOs, unsigned int *EBOs, const std::vector<float> &phongVertexData, const std::vector<float> &flatVertexData, const std::vector<float> &circleVertexData, const std::vector<unsigned int> &phongIndices, const std::vector<unsigned int> &flatIndices, const std::vector<unsigned int> &circleIndices) {
    glGenVertexArrays(3, VAOs);
    glGenBuffers(3, VBOs);
    glGenBuffers(3, EBOs);

    // Phong VAO
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, phongVertexData.size() * sizeof(float), phongVertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, phongIndices.size() * sizeof(unsigned int), phongIndices.data(), GL_STATIC_DRAW);


    // Flat VAO
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, flatVertexData.size() * sizeof(float), flatVertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size() * sizeof(unsigned int), flatIndices.data(), GL_STATIC_DRAW);


    // Circle VAO
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, circleVertexData.size() * sizeof(float), circleVertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0); // position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float))); // normal
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float))); // barycentric
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, circleIndices.size() * sizeof(unsigned int), circleIndices.data(), GL_STATIC_DRAW);
    
    glBindVertexArray(0);
}

void A1solution::run(char *filename)
{
    glm::mat4 modelview, projection;
    int width, height;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> triangles;
    currentShader = 2;

    readInputFile(filename, modelview, projection, width, height, vertices, triangles);

    std::vector<glm::vec3> phongVertexNormals;
    std::vector<float> phongVertexData;
    std::vector<unsigned int> phongIndices;

    phongShader(vertices, triangles, phongVertexNormals, phongVertexData, phongIndices);

    std::vector<glm::vec3> flatFaceNormals;
    std::vector<float> flatVertexData;
    std::vector<unsigned int> flatIndices;
    flatShader(vertices, triangles, flatFaceNormals, flatVertexData, flatIndices);
 
    std::vector<float> circleVertexData;
    std::vector<unsigned int> circleIndices;
    circleShader(vertices, triangles, phongVertexNormals, circleVertexData, circleIndices); 
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // Set OpenGL version to 3.3 and use core profile
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

    // Make the window's context current and set key callback
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return;
    }

    glEnable(GL_DEPTH_TEST); // Enable depth testing

    // Grey background
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

    // Compile and load shaders
    int phongShader = compileAndLinkShaders(getVertexShaderPhong(), getFragmentShaderPhong());
    int flatShader = compileAndLinkShaders(getVertexShaderFlat(), getFragmentShaderFlat());
    int circleShader = compileAndLinkShaders(getVertexShaderCircle(), getFragmentShaderCircle());
    int shaderPrograms[] = {phongShader, flatShader, circleShader};
    int indexCount[] = {static_cast<int>(phongIndices.size()), static_cast<int>(flatIndices.size()), static_cast<int>(circleIndices.size())};

    // Upload vertex data to GPU
    unsigned int VAOs[3], VBOs[3], EBOs[3];
    createRenderingData(VAOs, VBOs, EBOs, phongVertexData, flatVertexData, circleVertexData, phongIndices, flatIndices, circleIndices);
    
    // Entering Main Loop
    while (!glfwWindowShouldClose(window))
    {
        // Each frame, reset color and depth of each pixel
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderPrograms[currentShader]); // Use the current shader program

        // Set shader uniforms
        unsigned int modelLoc = glGetUniformLocation(shaderPrograms[currentShader], "modelview");
        unsigned int projLoc  = glGetUniformLocation(shaderPrograms[currentShader], "projection");
        unsigned int normalLoc = glGetUniformLocation(shaderPrograms[currentShader], "normalMat");
        unsigned int lightLoc = glGetUniformLocation(shaderPrograms[currentShader], "lightPos");
        unsigned int lightColorLoc = glGetUniformLocation(shaderPrograms[currentShader], "lightColor");
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // Set light color to white at full intensity

        // Compute normal matrix (transpose of inverse of upper-left 3x3 of modelview)
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelview)));

        // Upload matrices to shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelview));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

        glUniform3f(lightLoc, 0.0f, 0.0f, 1.0f); // Light position in view space

        // Draw the triangles
        glBindVertexArray(VAOs[currentShader]);
        glDrawElements(GL_TRIANGLES, indexCount[currentShader], GL_UNSIGNED_INT, 0);

        // Swap front and back buffers and poll for events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Shutdown GLFW
    glfwTerminate();
}