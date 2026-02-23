#include "A1solution.h"
#include <iostream>
#include <fstream>
#include <vector>

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

// Vertex shader source for Phong
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
        flat out vec3 normal;  // <-- must match fragment shader

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

// Vertex shader source for Circle and Voronoi 
const char *getVertexShaderBary()
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

const char *getFragmentShaderVoronoi()
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
        // Closest vertex = highest barycentric coordinate
        vec3 diffuseColor;
        vec3 ambientColor;
        if (bary.x >= bary.y && bary.x >= bary.z)
        {
            diffuseColor = vec3(1.0, 0.5, 0.5);
            ambientColor = vec3(0.1, 0.05, 0.05);
        }
        else if (bary.y >= bary.z)
        {
            diffuseColor = vec3(0.5, 1.0, 0.5);
            ambientColor = vec3(0.05, 0.1, 0.05);
        }
        else
        {
            diffuseColor = vec3(0.5, 0.5, 1.0);
            ambientColor = vec3(0.05, 0.05, 0.1);
        }

        vec3 norm = normalize(normal);
        vec3 lightDir = normalize(lightPos - fragPos);

        vec3 ambient = ambientColor * lightColor;

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * diffuseColor;

        vec3 viewDir = normalize(-fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5.0);
        vec3 specular = spec * lightColor * vec3(0.3);

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

// Function to read input file and populate data structures
void readInputFile(
    const char *filename, 
    glm::mat4 &modelview, 
    glm::mat4 &projection, 
    int &width, 
    int &height, 
    std::vector<glm::vec3> &vertices, 
    std::vector<glm::vec3> &triangles)
{
    std::ifstream in(filename);

    if (!in.is_open())
    {
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

void buildPhongData(
    const std::vector<glm::vec3> &vertices, 
    const std::vector<glm::vec3> &triangles, 
    std::vector<glm::vec3> &vertexNormals,
    std::vector<float> &vertexData, 
    std::vector<unsigned int> &indices)
{
    vertexNormals.assign(vertices.size(), glm::vec3(0.0f));

    for (auto &t : triangles)
    {
        glm::vec3 v0 = vertices[static_cast<int>(t.x)];
        glm::vec3 v1 = vertices[static_cast<int>(t.y)];
        glm::vec3 v2 = vertices[static_cast<int>(t.z)];

        glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        vertexNormals[static_cast<int>(t.x)] += faceNormal;
        vertexNormals[static_cast<int>(t.y)] += faceNormal;
        vertexNormals[static_cast<int>(t.z)] += faceNormal;
    }
    for (auto &n : vertexNormals)
        n = glm::normalize(n);

    vertexData.reserve(vertices.size() * 6);
    for (size_t i = 0; i < vertices.size(); i++)
    {
        vertexData.push_back(vertices[i].x);
        vertexData.push_back(vertices[i].y);
        vertexData.push_back(vertices[i].z);
        vertexData.push_back(vertexNormals[i].x);
        vertexData.push_back(vertexNormals[i].y);
        vertexData.push_back(vertexNormals[i].z);
    }

    for (const auto &t : triangles)
    {
        indices.push_back(static_cast<unsigned int>(t.x));
        indices.push_back(static_cast<unsigned int>(t.y));
        indices.push_back(static_cast<unsigned int>(t.z));
    }
}

void buildFlatData(
    const std::vector<glm::vec3> &vertices, 
    const std::vector<glm::vec3> &triangles, 
    std::vector<float> &vertexData, 
    std::vector<unsigned int> &indices)
{
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

// Circle and Voronoi shaders will use the same vertex data, so we can generate it together
void buildBaryData(
    const std::vector<glm::vec3> &vertices, 
    const std::vector<glm::vec3> &triangles,
    std::vector<glm::vec3> &vertexNormals,
    std::vector<float> &vertexData, 
    std::vector<unsigned int> &indices)
{
    static const float bary[3][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };

    for (size_t i = 0; i < triangles.size(); i++)
    {
        // Get triangle vertices
        int idx[3] = {
            (int)triangles[i].x, 
            (int)triangles[i].y, 
            (int)triangles[i].z
        };;       

        for (int j = 0; j < 3; j++)
        {
            const glm::vec3 &v = vertices[idx[j]];
            const glm::vec3 &n = vertexNormals[idx[j]];
            vertexData.push_back(v.x);
            vertexData.push_back(v.y);
            vertexData.push_back(v.z);
            vertexData.push_back(n.x);
            vertexData.push_back(n.y);
            vertexData.push_back(n.z);
            vertexData.push_back(bary[j][0]);
            vertexData.push_back(bary[j][1]);
            vertexData.push_back(bary[j][2]);
        }

        // Add indices for the triangle
        unsigned int baseIndex = static_cast<unsigned int>(i) * 3;
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
    }
}

void createRenderingData(
    unsigned int *VAOs, 
    unsigned int *VBOs, 
    unsigned int *EBOs, 
    const std::vector<float> &phongData,
    const std::vector<float> &flatData,
    const std::vector<float> &baryData,
    const std::vector<unsigned int> &phongIdx,
    const std::vector<unsigned int> &flatIdx,
    const std::vector<unsigned int> &baryIdx)
{
    glGenVertexArrays(4, VAOs);
    glGenBuffers(4, VBOs);
    glGenBuffers(4, EBOs);

    auto uploadBasic = [&](int i, const std::vector<float> &data, const std::vector<unsigned int> &idx)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    };

    auto uploadBary = [&](int i, const std::vector<float> &data, const std::vector<unsigned int> &idx)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    };

    uploadBasic(0, phongData, phongIdx); // Phong
    uploadBasic(1, flatData,  flatIdx);  // Flat
    uploadBary (2, baryData,  baryIdx);  // Circle
    uploadBary (3, baryData,  baryIdx);  // Voronoi (same geometry)

    glBindVertexArray(0);
}

void A1solution::run(char *filename)
{
    glm::mat4 modelview, projection;
    int width, height;
    std::vector<glm::vec3> vertices, triangles;

    readInputFile(filename, modelview, projection, width, height, vertices, triangles);

    // Build CPU-side vertex data
    std::vector<glm::vec3> phongNormals;
    std::vector<float> phongData, flatData, baryData;
    std::vector<unsigned int> phongIdx, flatIdx, baryIdx;

    buildPhongData(vertices, triangles, phongNormals, phongData, phongIdx);
    buildFlatData(vertices, triangles, flatData, flatIdx);
    buildBaryData(vertices, triangles, phongNormals, baryData, baryIdx);

    // State
    int currentShader = 0;
    int mode = 0; // 0 for fill, 1 for wireframe
    bool sKeyDown = false;
    bool wKeyDown = false;

    // Initialize GLFW
    if (!glfwInit())
    {
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

    // Compile shaders
    int shaderPrograms[4] = {
        compileAndLinkShaders(getVertexShaderPhong(), getFragmentShaderPhong()),
        compileAndLinkShaders(getVertexShaderFlat(),  getFragmentShaderFlat()),
        compileAndLinkShaders(getVertexShaderBary(),  getFragmentShaderCircle()),
        compileAndLinkShaders(getVertexShaderBary(),  getFragmentShaderVoronoi())
    };

    int indexCount[4] = {
        (int)phongIdx.size(),
        (int)flatIdx.size(),
        (int)baryIdx.size(),
        (int)baryIdx.size()
    };

    // Upload vertex data to GPU
    unsigned int VAOs[4], VBOs[4], EBOs[4];
    createRenderingData(VAOs, VBOs, EBOs, phongData, flatData, baryData, phongIdx, flatIdx, baryIdx);
    
    // Entering Main Loop
    while (!glfwWindowShouldClose(window))
    {
        // Each frame, reset color and depth of each pixel
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderPrograms[currentShader]); // Use the current shader program

        // Set shader uniforms
        unsigned int modelLoc = glGetUniformLocation(shaderPrograms[currentShader], "modelview");
        unsigned int projLoc = glGetUniformLocation(shaderPrograms[currentShader], "projection");
        unsigned int normalLoc = glGetUniformLocation(shaderPrograms[currentShader], "normalMat");
        unsigned int lightLoc = glGetUniformLocation(shaderPrograms[currentShader], "lightPos");
        unsigned int lightColorLoc = glGetUniformLocation(shaderPrograms[currentShader], "lightColor");
        
        // Compute normal matrix (transpose of inverse of upper-left 3x3 of modelview)
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelview)));
        
        // Upload matrices to shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelview));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normalMat));
        
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // Set light color to white at full intensity
        glUniform3f(lightLoc, 0.0f, 0.0f, 1.0f); // Light position in view space

        // Draw the triangles
        glBindVertexArray(VAOs[currentShader]);
        glDrawElements(GL_TRIANGLES, indexCount[currentShader], GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap front and back buffers and poll for events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Input handling
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        bool sPressed = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;        
        if (sPressed && !sKeyDown) {
            currentShader = (currentShader + 1) % 4;
        }
        sKeyDown = sPressed;

        bool wPressed = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS; 
        if (wPressed && !wKeyDown)
        {
            mode = (mode + 1) % 2;
            glPolygonMode(GL_FRONT_AND_BACK, mode == 0 ? GL_FILL : GL_LINE);
        }
        wKeyDown = wPressed;
    }

    // Shutdown GLFW
    glfwTerminate();
}