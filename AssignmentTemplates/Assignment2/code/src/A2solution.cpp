#include "A2solution.h"
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
        // input vertex attributes from VBO
        layout(location = 0) in vec3 aPos; // vertex pos in model space
        layout(location = 1) in vec3 aNormal; // vertex normal in model space

        // output passed to fragment shader
        out vec3 fragPos; // pos in view space
        out vec3 normal; // normal in view space

        uniform mat4 projection;
        uniform mat4 modelview;
        uniform mat3 normalMat;

        void main()
        {
            // transform vertex to view space
            vec4 vertPos4 = modelview * vec4(aPos, 1.0);
            fragPos = vertPos4.xyz;

            // transform normal to view space using normal matrix
            normal = normalMat * aNormal;     
            
            // final clip-space pos for rasterization
            gl_Position = projection * vertPos4;
        }
    )";
}

const char *getFragmentShaderPhong()
{
    return R"(
    #version 330 core

    in vec3 fragPos; // interpolated view-space pos
    in vec3 normal; // interpolated view-space normal

    out vec4 FragColor;

    uniform vec3 lightPos; // light pos in view space
    uniform vec3 lightColor; 

    void main()
    {
        vec3 norm = normalize(normal); // re-normalize after interpolation

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
        flat out vec3 normal; // flat makes every pixel in triangle to use the same normal

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
    layout(location = 2) in vec3 aIncenter;
    layout(location = 3) in float aInradius;

    out vec3 fragPos;

    out vec3 normal;
    out vec3 incenter;
    out float inradius;

    uniform mat4 projection;
    uniform mat4 modelview;
    uniform mat3 normalMat;

    void main()
    {
        vec4 vertPos4 = modelview * vec4(aPos, 1.0);
        fragPos = vertPos4.xyz;
        normal = normalMat * aNormal;
        incenter = (modelview * vec4(aIncenter, 1.0)).xyz;
        inradius = aInradius;
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
    in vec3 incenter;
    in float inradius;

    out vec4 FragColor;

    uniform vec3 lightPos;
    uniform vec3 lightColor;

    void main() 
    {        
        // Distance check 
        float dist = length(fragPos - incenter);
        bool insideCircle = dist < inradius;
        
        vec3 norm = normalize(normal);
        vec3 ambient;
        vec3 diffuseColor;
        vec3 specular = vec3(0.0);

        if (insideCircle) 
        {
            ambient = vec3(0.1, 0.05, 0.05) * lightColor;
            diffuseColor = vec3(1.0, 0.5, 0.5);
        }
        else
        {
            ambient = vec3(0.05, 0.05, 0.1) * lightColor;
            diffuseColor = vec3(0.5, 0.5, 1.0);
        }

        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * diffuseColor;

        if (insideCircle)
        {
            vec3 viewDir = normalize(-fragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5.0);
            specular = spec * lightColor * vec3(0.3);
        }
        else 
        {
            specular = vec3(0.0);
        }

        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
    )";
}

const char *getVertexShaderVoronoi()
{
    return R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec3 aV0;
        layout(location = 3) in vec3 aV1;
        layout(location = 4) in vec3 aV2;

        out vec3 fragPos;
        out vec3 normal;
        out vec3 v0;
        out vec3 v1;
        out vec3 v2;

        uniform mat4 projection;
        uniform mat4 modelview;
        uniform mat3 normalMat;

        void main()
        {
            vec4 vertPos4 = modelview * vec4(aPos, 1.0);
            fragPos = vertPos4.xyz;
            normal = normalMat * aNormal;
            // Transform vertex positions to view space for distance comparison
            v0 = (modelview * vec4(aV0, 1.0)).xyz;
            v1 = (modelview * vec4(aV1, 1.0)).xyz;
            v2 = (modelview * vec4(aV2, 1.0)).xyz;
            gl_Position = projection * vertPos4;
        }
    )";
}

const char *getFragmentShaderVoronoi()
{
    return R"(
    #version 330 core

    in vec3 fragPos;
    in vec3 normal;
    in vec3 v0;
    in vec3 v1;
    in vec3 v2;

    out vec4 FragColor;

    uniform vec3 lightPos;
    uniform vec3 lightColor;

    void main()
    {
        // Compute distance from this fragment to each vertices
        float d0 = length(fragPos - v0);
        float d1 = length(fragPos - v1);
        float d2 = length(fragPos - v2);

        vec3 diffuseColor;
        vec3 ambientColor;

        // closest to v0 = red
        if (d0 <= d1 && d0 <= d2)
        {
            diffuseColor = vec3(1.0, 0.5, 0.5); // red
            ambientColor = vec3(0.1, 0.05, 0.05);
        }
        else if (d1 <= d2) // closest to v1 = green
        {
            diffuseColor = vec3(0.5, 1.0, 0.5); // green
            ambientColor = vec3(0.05, 0.1, 0.05);
        }
        else // closest to v2 = blue
        {
            diffuseColor = vec3(0.5, 0.5, 1.0); // blue
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
        vec3 specular = spec * lightColor * vec3(0.3, 0.3, 0.3);

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

    // Calculate vertex normals 
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
    // Normalize vertex normals
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

void buildCircleData(
    const std::vector<glm::vec3> &vertices,
    const std::vector<glm::vec3> &triangles,
    std::vector<glm::vec3> &vertexNormals,
    std::vector<float> &vertexData,
    std::vector<unsigned int> &indices)
{
    for (size_t i = 0; i < triangles.size(); i++)
    {
        // Get triangle vertices
        int idx[3] = {
            (int)triangles[i].x,
            (int)triangles[i].y,
            (int)triangles[i].z};
        ;

        glm::vec3 v0 = vertices[idx[0]];
        glm::vec3 v1 = vertices[idx[1]];
        glm::vec3 v2 = vertices[idx[2]];

        float a = glm::length(v2 - v1);
        float b = glm::length(v2 - v0);
        float c = glm::length(v1 - v0);

        glm::vec3 incenter = (a * v0 + b * v1 + c * v2) / (a + b + c);
        float s = (a + b + c) / 2.0f;
        float area = glm::length(glm::cross(v1 - v0, v2 - v0)) / 2.0f;
        float inradius = area / s;

        glm::vec3 verts[3] = {v0, v1, v2};

        for (int j = 0; j < 3; j++)
        {
            const glm::vec3 &v = verts[j];
            const glm::vec3 &n = vertexNormals[idx[j]];

            // Position (3)
            vertexData.push_back(v.x);
            vertexData.push_back(v.y);
            vertexData.push_back(v.z);
            // Normal (3)
            vertexData.push_back(n.x);
            vertexData.push_back(n.y);
            vertexData.push_back(n.z);
            // Incenter (3)
            vertexData.push_back(incenter.x);
            vertexData.push_back(incenter.y);
            vertexData.push_back(incenter.z);
            // Inradius (1)
            vertexData.push_back(inradius);
        }

        // Add indices for the triangle
        unsigned int baseIndex = static_cast<unsigned int>(i) * 3;
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
    }
}

void buildVoronoiData(
    const std::vector<glm::vec3> &vertices,
    const std::vector<glm::vec3> &triangles,
    std::vector<glm::vec3> &vertexNormals,
    std::vector<float> &vertexData,
    std::vector<unsigned int> &indices)
{
    for (size_t i = 0; i < triangles.size(); i++)
    {
        int idx[3] = {
            (int)triangles[i].x,
            (int)triangles[i].y,
            (int)triangles[i].z};
          
        glm::vec3 v0 = vertices[idx[0]];
        glm::vec3 v1 = vertices[idx[1]];
        glm::vec3 v2 = vertices[idx[2]];

        for (int j = 0; j < 3; j++)
        {
            const glm::vec3 &v = vertices[idx[j]];
            const glm::vec3 &n = vertexNormals[idx[j]];

            // Position (3)
            vertexData.push_back(v.x);
            vertexData.push_back(v.y);
            vertexData.push_back(v.z);
            // Normal (3)
            vertexData.push_back(n.x);
            vertexData.push_back(n.y);
            vertexData.push_back(n.z);
            // Vertex 0 position (3)
            vertexData.push_back(v0.x);
            vertexData.push_back(v0.y);
            vertexData.push_back(v0.z);
            // Vertex 1 position (3)
            vertexData.push_back(v1.x);
            vertexData.push_back(v1.y);
            vertexData.push_back(v1.z);
            // Vertex 2 position (3)
            vertexData.push_back(v2.x);
            vertexData.push_back(v2.y);
            vertexData.push_back(v2.z);
        }

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
    const std::vector<float> &circleData,
    const std::vector<float> &voronoiData,
    const std::vector<unsigned int> &phongIdx,
    const std::vector<unsigned int> &flatIdx,
    const std::vector<unsigned int> &circleIdx,
    const std::vector<unsigned int> &voronoiIdx)
{
    glGenVertexArrays(4, VAOs);
    glGenBuffers(4, VBOs);
    glGenBuffers(4, EBOs);

    // For Phong and Flat
    auto uploadBasic = [&](int i, const std::vector<float> &data, const std::vector<unsigned int> &idx)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        // Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    };

    auto uploadCircle = [&](int i, const std::vector<float> &data, const std::vector<unsigned int> &idx)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        // Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // Incenter
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        // Inradius
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(9 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    };

    auto uploadVoronoi = [&](int i, const std::vector<float> &data, const std::vector<unsigned int> &idx)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData( GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        // Position attribute at location 0 is 3 floats, stride 15 floats, starts at offset 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // Normal attribute at location 1 is 3 floats, stride 15 floats, starts at offset 3 floats in
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // Vertex 0 position
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        // Vertex 1 position
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void *)(9 * sizeof(float)));
        glEnableVertexAttribArray(3);
        // Vertex 2 position
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (void *)(12 * sizeof(float)));
        glEnableVertexAttribArray(4);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    };

    uploadBasic(0, phongData, phongIdx);       
    uploadBasic(1, flatData, flatIdx);         
    uploadCircle(2, circleData, circleIdx);    
    uploadVoronoi(3, voronoiData, voronoiIdx); 

    glBindVertexArray(0);
}

struct AppState {
    int currentShader;
    bool mouseButtonDown;
    double mouseX, mouseY;

    // For picking
    const std::vector<glm::vec3>* vertices;
    const std::vector<glm::vec3>* triangles;
    glm::mat4 modelview;
    glm::mat4 projection;
    int width, height;
};

// Converts a 3D vertex to 2D screen pixel coordinates.
glm::vec3 projectToScreen(const glm::vec3& vertex, const glm::mat4& modelview, const glm::mat4& projection, int width, int height) 
{
    glm::vec4 viewSpace = modelview * glm::vec4(vertex, 1.0f);
    glm::vec4 clipSpace = projection * viewSpace;
    glm::vec3 ndc = {clipSpace.x/clipSpace.w, clipSpace.y/clipSpace.w, clipSpace.z/clipSpace.w};
    glm::vec3 screenSpace = {
        (ndc.x + 1)/2 * width, 
        (1 - ndc.y)/2 * height,
        viewSpace.z
    }; 
    return screenSpace;
}

// Checks which side of each edge p falls on. Must be same side for all 3.
bool pointInTriangle(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c) 
{
    float crossABAP = (b.x - a.x)*(p.y - a.y) - (b.y - a.y)*(p.x - a.x);
    float crossBCBP = (c.x - b.x)*(p.y - b.y) - (c.y - b.y)*(p.x - b.x);
    float crossCACP = (a.x - c.x)*(p.y - c.y) - (a.y - c.y)*(p.x - c.x);

    // Check all 3 has same sign
    return (crossABAP < 0 && crossBCBP < 0 && crossCACP < 0) || (crossABAP > 0 && crossBCBP > 0 && crossCACP > 0);
}

glm::vec3 barycentricCoords(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
    float areaABC = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
    float areaPBC = (b.x - p.x)*(c.y - p.y) - (b.y - p.y)*(c.x - p.x);
    float areaAPC = (p.x - a.x)*(c.y - a.y) - (p.y - a.y)*(c.x - a.x);
    float areaABP = (b.x - a.x)*(p.y - a.y) - (b.y - a.y)*(p.x - a.x);

    float lambda1 = areaPBC / areaABC;
    float lambda2 = areaAPC / areaABC;
    float lambda3 = 1.0f - lambda1 - lambda2;

    return {lambda1, lambda2, lambda3};
}

void pickTriangle(AppState* state) {
    // Only pick in Phong mode
    if (state->currentShader != 0) return;

    glm::vec2 mouse = {(float)state->mouseX, (float)state->mouseY};

    int bestTriangle = -1;
    float bestZ = -FLT_MAX; //highest (least negative) Z
    glm::vec3 bestBary;
    glm::vec3 best3DPoint;

    for (int i = 0; i < state->triangles->size(); i++) {
        // Get the 3 vertex indices for this triangle
        int i0 = (int)(*state->triangles)[i].x;
        int i1 = (int)(*state->triangles)[i].y;
        int i2 = (int)(*state->triangles)[i].z;

        // Project each vertex to screen space
        glm::vec3 s0 = projectToScreen((*state->vertices)[i0], state->modelview, state->projection, state->width, state->height);
        glm::vec3 s1 = projectToScreen((*state->vertices)[i1], state->modelview, state->projection, state->width, state->height);
        glm::vec3 s2 = projectToScreen((*state->vertices)[i2], state->modelview, state->projection, state->width, state->height);
    
        if (pointInTriangle(mouse, s0, s1, s2)) {
            glm::vec3 bary = barycentricCoords(mouse, s0, s1, s2);

            // Interpolate view space Z for depth comparison
            float z = bary.x * s0.z + bary.y * s1.z + bary.z * s2.z;

            if (z > bestZ) {
                bestZ = z;
                bestTriangle = i;
                bestBary = bary;

                // Reconstruct 3D point using original vertices
                best3DPoint = bary.x * (*state->vertices)[i0] +
                              bary.y * (*state->vertices)[i1] +
                              bary.z * (*state->vertices)[i2];
            }
        }
    }

    if (bestTriangle != -1) {
    std::cout << bestTriangle << " "
                << bestBary.x << " " << bestBary.y << " " << bestBary.z << " "
                << best3DPoint.x << " " << best3DPoint.y << " " << best3DPoint.z
                << std::endl;
    }
}

// Called when mouse button is clicked or released.
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            state->mouseButtonDown = true;
            pickTriangle(state);
        }
        else if (action == GLFW_RELEASE)
        {
            state->mouseButtonDown = false;
        }
    }
}

// Called whenever the mouse moves (drag).
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    state->mouseX = xpos;
    state->mouseY = ypos;
    if (state->mouseButtonDown)
    {
        pickTriangle(state);
    }
}

void A2solution::run(char *filename)
{
    glm::mat4 modelview, projection;
    int width, height;
    std::vector<glm::vec3> vertices, triangles;

    readInputFile(filename, modelview, projection, width, height, vertices, triangles);

    std::vector<glm::vec3> phongNormals;
    std::vector<float> phongData, flatData, circleData, voronoiData;
    std::vector<unsigned int> phongIdx, flatIdx, circleIdx, voronoiIdx;

    buildPhongData(vertices, triangles, phongNormals, phongData, phongIdx);
    buildFlatData(vertices, triangles, flatData, flatIdx);
    buildCircleData(vertices, triangles, phongNormals, circleData, circleIdx);
    buildVoronoiData(vertices, triangles, phongNormals, voronoiData, voronoiIdx);

    AppState state;
    state.currentShader = 0;
    state.mouseButtonDown = false;
    state.mouseX = 0.0;
    state.mouseY = 0.0;
    state.vertices = &vertices;
    state.triangles = &triangles;
    state.modelview = modelview;
    state.projection = projection;
    state.width = width;
    state.height = height;

    int mode = 0; 
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

    // Create Window and rendering context using GLFW
    GLFWwindow *window = glfwCreateWindow(width, height, "Comp371 - Assignment 01", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Register mouse callbacks for picking
    glfwSetWindowUserPointer(window, &state);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

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
        compileAndLinkShaders(getVertexShaderFlat(), getFragmentShaderFlat()),
        compileAndLinkShaders(getVertexShaderCircle(), getFragmentShaderCircle()),
        compileAndLinkShaders(getVertexShaderVoronoi(), getFragmentShaderVoronoi())};

    // Store index counts for each shader to use in glDrawElements
    int indexCount[4] = {
        (int)phongIdx.size(),
        (int)flatIdx.size(),
        (int)circleIdx.size(),
        (int)voronoiIdx.size()};

    // Upload vertex data to GPU
    unsigned int VAOs[4], VBOs[4], EBOs[4];
    createRenderingData(VAOs, VBOs, EBOs, phongData, flatData, circleData, voronoiData, phongIdx, flatIdx, circleIdx, voronoiIdx);

    // Entering Main Loop
    while (!glfwWindowShouldClose(window))
    {
        // Each frame, reset color and depth of each pixel
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderPrograms[state.currentShader]); // Use the current shader program

        // Set shader uniforms
        unsigned int modelLoc = glGetUniformLocation(shaderPrograms[state.currentShader], "modelview");
        unsigned int projLoc = glGetUniformLocation(shaderPrograms[state.currentShader], "projection");
        unsigned int normalLoc = glGetUniformLocation(shaderPrograms[state.currentShader], "normalMat");
        unsigned int lightLoc = glGetUniformLocation(shaderPrograms[state.currentShader], "lightPos");
        unsigned int lightColorLoc = glGetUniformLocation(shaderPrograms[state.currentShader], "lightColor");

        // Compute normal matrix (transpose of inverse of upper-left 3x3 of modelview)
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelview)));

        // Upload matrices to shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelview));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // Set light color to white at full intensity
        glUniform3f(lightLoc, 0.0f, 0.0f, 1.0f);      // Light position in view space

        // Draw the triangles
        glBindVertexArray(VAOs[state.currentShader]);
        glDrawElements(GL_TRIANGLES, indexCount[state.currentShader], GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap front and back buffers and poll for events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Input handling
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        bool sPressed = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        if (sPressed && !sKeyDown)
        {
            state.currentShader = (state.currentShader + 1) % 4;
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