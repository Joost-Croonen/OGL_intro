#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <map>
#include <random>

#include "vbo.h"
#include "ebo.h"
#include "vao.h"
#include "fbo.h"
#include "rbo.h"
#include "ubo.h"
#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "mesh.h"
#include "model.h"
#include "ppo.h"
#include "gbo.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
float lerp(float a, float b, float f);

// settings
const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1440;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
const float NEAR_PLANE = 0.1;
const float FAR_PLANE = 100;
bool zoom = false;
bool post_process = false;
bool post_process_key = false;
bool normal_mapping = false;
bool normal_mapping_key = false;
float heightScale = 0.0f;
float exposure = 1.0f;
bool bloom = false;
bool bloom_key = false;
bool ssaoFlag = false;
bool ssao_key = false;
bool toggle = false;
bool toggle_key = false;

// camera
//Camera camera(glm::vec3(1.0f, 1.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -100, -20);
Camera camera(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90, 0);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int base_scene() {
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    // Depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Stencil testing
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // MSAA
    glEnable(GL_MULTISAMPLE);

    // Gamma correction
    bool gamma_correct = false;
    if(gamma_correct) glEnable(GL_FRAMEBUFFER_SRGB);

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices
    float planeVertices[] = {
        // positions            // normals         // texcoords
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,    //3
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,    //0
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,    //1

         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,    //3
         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,    //2
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f     //0
    };


    // Buffer objects
    VAO planeVAO = VAO();
    VBO planeVBO = VBO(planeVertices, sizeof(planeVertices));
    planeVAO.bind();
    planeVAO.linkVBO(planeVBO);
    planeVAO.setAttributes(3, 3, 2);
    planeVAO.unbind();

    // Shaders
    Shader ourShader("../../../src/shaders/simple.vert", "../../../src/shaders/simple.frag");

    ourShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);

    // Model
    Model ourModel("../../../src/models/backpack/backpack.obj", gamma_correct);

    // Load other textures
    Texture floorTexture = Texture("../../../src/textures/wood.png", gamma_correct);

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Floor
        ourShader.use();
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        floorTexture.activate(ourShader, "texture_diffuse1", 0);
        planeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Backpack
        ourShader.use();
        model = glm::translate(model, glm::vec3(0.0, 0.5, 0.0));
        model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        ourModel.Draw(ourShader);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    planeVAO.Delete();
    planeVBO.Delete();
    glfwTerminate();
    return 0;
}

int main_scene()
{
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Vertices
    float planeVertices[] = {
        // positions            normals               texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,     2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,     0.0f, 2.0f,
         5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,     2.0f, 2.0f
    };

    float quadVertices[] = {
        // positions          normals              texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
         0.0f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
         1.0f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
         1.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f
    };
    float screenVertices[] = {
        // positions          texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
        -1.0f / 2.0f,  1.0f / 2.0f, 0.0f,   0.0f, 1.0f,
        -1.0f / 2.0f, -1.0f / 2.0f, 0.0f,   0.0f, 0.0f,
         1.0f / 2.0f, -1.0f / 2.0f, 0.0f,   1.0f, 0.0f,
         1.0f / 2.0f,  1.0f / 2.0f, 0.0f,   1.0f, 1.0f
    };

    float cubeVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,

         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,

        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    std::vector<glm::vec3> vegetation;
    vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

    // plane VAO
    VAO planeVAO = VAO();
    VBO planeVBO = VBO(planeVertices, sizeof(planeVertices));
    EBO planeEBO = EBO(indices, sizeof(indices));
    planeVAO.bind();
    planeVAO.linkVBO(planeVBO);
    planeVAO.linkEBO(planeEBO);
    planeVAO.setAttributes();
    planeVAO.unbind();

    // quad VAO
    VAO quadVAO = VAO();
    VBO quadVBO = VBO(quadVertices, sizeof(quadVertices));
    EBO quadEBO = EBO(indices, sizeof(indices));
    quadVAO.bind();
    quadVAO.linkVBO(quadVBO);
    quadVAO.linkEBO(quadEBO);
    quadVAO.setAttributes();
    quadVAO.unbind();

    // screen VAO
    VAO screenVAO = VAO();
    VBO screenVBO = VBO(screenVertices, sizeof(screenVertices));
    EBO screenEBO = EBO(indices, sizeof(indices));
    screenVAO.bind();
    screenVAO.linkVBO(screenVBO);
    screenVAO.linkEBO(screenEBO);
    screenVAO.setAttributes(3, 0, 2);
    screenVAO.unbind();

    // skybox VAO
    VAO skyVAO = VAO();
    VBO skyVBO = VBO(cubeVertices, sizeof(cubeVertices));
    //EBO skyEBO = EBO(indices, sizeof(indices));
    skyVAO.bind();
    skyVAO.linkVBO(skyVBO);
    //screenVAO.linkEBO(screenEBO);
    skyVAO.setAttributes(3, 0, 2);
    skyVAO.unbind();

    // Shaders
    Shader ourShader("../../../src/shaders/vertex.vert", "../../../src/shaders/fragment.frag");
    Shader outlineShader("../../../src/shaders/outline.vert", "../../../src/shaders/outline.frag");
    Shader simpleShader("../../../src/shaders/simple.vert", "../../../src/shaders/simple.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/screen.frag");
    Shader skyShader("../../../src/shaders/cubemap.vert", "../../../src/shaders/cubemap.frag");
    Shader reflectShader("../../../src/shaders/vertex.vert", "../../../src/shaders/refraction.frag");

    // Model
    Model ourModel("../../../src/models/backpack/backpack.obj");

    // Load other textures
    Texture floorTexture = Texture("../../../src/textures/marble.jpg",
        GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Texture grassTexture = Texture("../../../src/textures/grass.png",
        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Texture windowTexture = Texture("../../../src/textures/window.png",
        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    std::vector < std::string > cubemap_paths = {
        "../../../src/textures/skybox/right.jpg",
        "../../../src/textures/skybox/left.jpg",
        "../../../src/textures/skybox/top.jpg",
        "../../../src/textures/skybox/bottom.jpg",
        "../../../src/textures/skybox/front.jpg",
        "../../../src/textures/skybox/back.jpg"
    };

    Cubemap skybox = Cubemap(cubemap_paths);

    // Render to texture
    FBO fbo = FBO();
    fbo.bind();
    Texture bufferTexture = Texture(SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0, GL_RGB);
    bufferTexture.attach(GL_COLOR_ATTACHMENT0);
    RBO rbo = RBO(SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0, GL_DEPTH24_STENCIL8);
    rbo.bind();
    rbo.attach(GL_DEPTH_STENCIL_ATTACHMENT);
    //rbo.unbind();
    fbo.check_status();
    fbo.unbind();



    // Lights
    ourShader.use();
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };
    // dir light
    ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    ourShader.setVec3("dirLight.ambient", 0.0f, 0.0f, 0.0f);
    ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    ourShader.setVec3("dirLight.specular", 0.4f, 0.4f, 0.4f);
    // point lights
    ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
    ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    ourShader.setFloat("pointLights[0].constant", 1.0f);
    ourShader.setFloat("pointLights[0].linear", 0.09f);
    ourShader.setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    ourShader.setFloat("pointLights[1].constant", 1.0f);
    ourShader.setFloat("pointLights[1].linear", 0.09f);
    ourShader.setFloat("pointLights[1].quadratic", 0.032f);
    // point light 3
    ourShader.setVec3("pointLights[2].position", pointLightPositions[2]);
    ourShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    ourShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
    ourShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    ourShader.setFloat("pointLights[2].constant", 1.0f);
    ourShader.setFloat("pointLights[2].linear", 0.09f);
    ourShader.setFloat("pointLights[2].quadratic", 0.032f);
    // point light 4
    ourShader.setVec3("pointLights[3].position", pointLightPositions[3]);
    ourShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    ourShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
    ourShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    ourShader.setFloat("pointLights[3].constant", 1.0f);
    ourShader.setFloat("pointLights[3].linear", 0.09f);
    ourShader.setFloat("pointLights[3].quadratic", 0.032f);

    // spot light
    ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    ourShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);
    ourShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);
    ourShader.setFloat("spotLight.constant", 1.0f);
    ourShader.setFloat("spotLight.linear", 0.09f);
    ourShader.setFloat("spotLight.quadratic", 0.032f);
    ourShader.setVec3("spotLight.position", camera.Position);
    ourShader.setVec3("spotLight.direction", camera.Front);
    ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

    // material
    ourShader.setFloat("material.shininess", 32.0f);

    // Enable depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable stencil testing
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Inputs
        processInput(window);

        // Rendering
        // First pass to texture
        fbo.bind();
        glViewport(0, 0, SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(15.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, NEAR_PLANE, FAR_PLANE);
        // camera/view matrix
        glm::mat4 view = camera.GetViewMatrix();
        // model matrix
        glm::mat4 model = glm::mat4(1.0f);

        // activate shader and set uniforms
        ourShader.use();
        // viewpos
        ourShader.setVec3("viewPos", camera.Position);

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // 1st pass backpack
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);

        //glStencilFunc(GL_ALWAYS, 1, 0xFF);
        //glStencilMask(0xFF);
        //glStencilMask(0x00);
        //ourModel.Draw(ourShader);

        /*
        reflectShader.use();
        glStencilMask(0x00);
        skybox.activate(reflectShader, "skybox", 0);
        reflectShader.setMat4("projection", projection);
        reflectShader.setMat4("view", view);
        reflectShader.setMat4("model", model);
        reflectShader.setVec3("viewPos", camera.Position);
        ourModel.Draw(reflectShader);
        */
        simpleShader.use();
        glStencilMask(0x00);
        skybox.activate(simpleShader, "skybox", 0);
        simpleShader.setMat4("projection", projection);
        simpleShader.setMat4("view", view);
        simpleShader.setMat4("model", model);
        simpleShader.setVec3("viewPos", camera.Position);
        ourModel.Draw(simpleShader);

        // floor
        ourShader.use();
        model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);
        glStencilMask(0x00);
        planeVAO.bind();
        floorTexture.activate(ourShader, "material.texture_diffuse1", 0);
        floorTexture.activate(ourShader, "material.texture_specular1", 1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        planeVAO.unbind();

        // Grass
        simpleShader.use();
        glStencilMask(0x00);
        quadVAO.bind();
        grassTexture.activate(simpleShader, "texture_diffuse1", 0);
        simpleShader.setMat4("projection", projection);
        simpleShader.setMat4("view", view);

        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < vegetation.size(); i++)
        {
            float distance = glm::length(camera.Position - vegetation[i]);
            sorted[distance] = vegetation[i];
        }
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            simpleShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        quadVAO.unbind();

        /*
        //2nd pass backpack outline
        outlineShader.use();
        outlineShader.setFloat("outlineScale", 0.2);
        outlineShader.setMat4("projection", projection);
        outlineShader.setMat4("view", view);
        outlineShader.setMat4("model", model);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        ourModel.Draw(outlineShader);

        glBindVertexArray(0);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);
        */

        glDepthFunc(GL_LEQUAL);
        skyShader.use();
        glStencilMask(0x00);
        glm::mat4 cubeView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyShader.setMat4("view", cubeView);
        skyShader.setMat4("projection", projection);
        // ... set view and projection matrix
        skyVAO.bind();
        skybox.activate(skyShader, "cubemap", 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // ######################
        // Second pass to draw normal scene
        fbo.unbind();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // pass projection matrix to shader
        projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, NEAR_PLANE, FAR_PLANE);

        // activate shader and set uniforms
        ourShader.use();
        ourShader.setMat4("projection", projection);

        // 1st pass backpack
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);

        //glStencilFunc(GL_ALWAYS, 1, 0xFF);
        //glStencilMask(0xFF);
        //glStencilMask(0x00);
        //ourModel.Draw(ourShader);

        /*
        reflectShader.use();
        skybox.activate(reflectShader, "skybox", 0);
        reflectShader.setMat4("projection", projection);
        reflectShader.setMat4("model", model);
        ourModel.Draw(reflectShader);
        */
        simpleShader.use();
        skybox.activate(simpleShader, "skybox", 0);
        simpleShader.setMat4("projection", projection);
        simpleShader.setMat4("model", model);
        ourModel.Draw(simpleShader);

        // floor
        ourShader.use();
        model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);
        glStencilMask(0x00);
        planeVAO.bind();
        floorTexture.activate(ourShader, "material.texture_diffuse1", 0);
        floorTexture.activate(ourShader, "material.texture_specular1", 1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        planeVAO.unbind();

        // Grass
        simpleShader.use();
        glStencilMask(0x00);
        quadVAO.bind();
        grassTexture.activate(simpleShader, "texture_diffuse1", 0);
        simpleShader.setMat4("projection", projection);

        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            simpleShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        quadVAO.unbind();

        glDepthFunc(GL_LEQUAL);
        skyShader.use();
        glStencilMask(0x00);
        skyShader.setMat4("projection", projection);
        skyVAO.bind();
        skybox.activate(skyShader, "cubemap", 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // Final pass to draw render texture to screen quad
        if (zoom) {
            fbo.unbind();
            glDisable(GL_DEPTH_TEST);
            //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            //glClear(GL_COLOR_BUFFER_BIT);

            screenShader.use();
            screenVAO.bind();
            bufferTexture.activate(screenShader, "screenTexture", 0);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Clear objects
    planeVAO.Delete();
    quadVAO.Delete();
    screenVAO.Delete();
    planeVBO.Delete();
    quadVBO.Delete();
    screenVBO.Delete();
    planeEBO.Delete();
    quadEBO.Delete();
    screenEBO.Delete();
    rbo.Delete();
    fbo.Delete();
    // Terminate glfw
    glfwTerminate();
    return 0;
}

int simple_scene() {

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Setup geometry, textures, buffers and shaders
    // Vertices
    float cubeVertices[] = {
        // positions            // texcoords       
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,

         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,

        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f
    };

    for (int i = 0; i < (sizeof(cubeVertices) / sizeof(cubeVertices[0])); i++) {
        cubeVertices[i] = cubeVertices[i]/2.0;
    }

    std::vector<glm::vec3> positions;
    positions.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));

    // Buffer objects
    VAO cubeVAO = VAO();
    VBO cubeVBO = VBO(cubeVertices, sizeof(cubeVertices));
    cubeVAO.bind();
    cubeVAO.linkVBO(cubeVBO);
    cubeVAO.setAttributes(3, 0, 2);
    cubeVAO.unbind();

    // Shaders
    Shader redShader("../../../src/shaders/ubo_test.vert", "../../../src/shaders/solid.frag");
    Shader greenShader("../../../src/shaders/ubo_test.vert", "../../../src/shaders/solid.frag");
    Shader blueShader("../../../src/shaders/ubo_test.vert", "../../../src/shaders/solid.frag");
    Shader yellowShader("../../../src/shaders/ubo_test.vert", "../../../src/shaders/solid.frag");

    redShader.use();
    redShader.setVec3("color", 1.0f, 0.0f, 0.0f);
    greenShader.use();
    greenShader.setVec3("color", 0.0f, 1.0f, 0.0f);
    blueShader.use();
    blueShader.setVec3("color", 0.0f, 0.0f, 1.0f);
    yellowShader.use();
    yellowShader.setVec3("color", 1.0f, 1.0f, 0.0f);

    redShader.setUniformBuffer("Matrices", 0);
    greenShader.setUniformBuffer("Matrices", 0);
    blueShader.setUniformBuffer("Matrices", 0);
    yellowShader.setUniformBuffer("Matrices", 0);

    UBO ubo = UBO(2 * sizeof(glm::mat4));
    ubo.bindRange(0, 0, 2 * sizeof(glm::mat4));

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ubo.writeRange(0, sizeof(glm::mat4), glm::value_ptr(projection));

    // Model
    //Model ourModel("../../../src/models/backpack/backpack.obj");

    // Load other textures
    //Texture floorTexture = Texture("../../../src/textures/marble.jpg",
    //    GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Enable depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable stencil testing
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        ubo.writeRange(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

        glm::mat4 model = glm::mat4(1.0f);

        cubeVAO.bind();
        
        redShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.75, 0.75, 0.0));
        redShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        greenShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.75, 0.75, 0.0));
        redShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        blueShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.75, -0.75, 0.0));
        redShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        yellowShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.75, -0.75, 0.0));
        redShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();   
    };
    // Terminate
    cubeVAO.Delete();
    cubeVBO.Delete();
    glfwTerminate();
    return 0;
}

int geom_shader_scene() {
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSetCursorPosCallback(window, mouse_callback);

    // Setup geometry, textures, buffers and shaders
    // Vertices

    float pointVertices[] = {
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
    };

    // Buffer objects
    VAO pointVAO = VAO();
    VBO pointVBO = VBO(pointVertices, sizeof(pointVertices));
    pointVAO.bind();
    pointVAO.linkVBO(pointVBO);
    pointVAO.setAttributes(2, 0, 0, 3);
    pointVAO.unbind();

    // Shader
    Shader shader("../../../src/shaders/geotest.vert", "../../../src/shaders/geotest.geom", "../../../src/shaders/geotest.frag");

    // Model
    //Model ourModel("../../../src/models/backpack/backpack.obj");

    // Load other textures
    //Texture floorTexture = Texture("../../../src/textures/marble.jpg",
    //    GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Enable depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable stencil testing
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Enable blending
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        //float currentFrame = static_cast<float>(glfwGetTime());
        //deltaTime = currentFrame - lastFrame;
        //lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        //glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw
        shader.use();
        pointVAO.bind();
        glDrawArrays(GL_POINTS, 0, 4);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    pointVAO.Delete();
    pointVBO.Delete();
    glfwTerminate();
    return 0;
}

int norm_vect_scene() {
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Setup geometry, textures, buffers and shaders

    // Shaders
    Shader simpleShader("../../../src/shaders/simple.vert", "../../../src/shaders/simple.frag");
    Shader normalShader("../../../src/shaders/normals.vert", "../../../src/shaders/normals.geom", "../../../src/shaders/solid.frag");

    // Setup const shader uniforms
    simpleShader.use();
    simpleShader.setMat4("projection", glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f));
    normalShader.use();
    normalShader.setMat4("projection", glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f));
    normalShader.setVec3("color", 1.0f, 1.0f, 0.0f);
    
    // Model
    Model ourModel("../../../src/models/backpack/backpack.obj");
    
    // Enable depht test
    glEnable(GL_DEPTH_TEST);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable stencil testing
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set shader uniforms
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        simpleShader.use();
        simpleShader.setMat4("model", model);
        simpleShader.setMat4("view", view);
        normalShader.use();
        normalShader.setMat4("model", model);
        normalShader.setMat4("view", view);

        // Draw
        // main scene
        simpleShader.use();
        ourModel.Draw(simpleShader);

        // normal lines
        normalShader.use();
        ourModel.Draw(normalShader);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Clear memory
    ourModel.Delete();
    // Terminate
    glfwTerminate();
    return 0;
}

int instancing_scene() {
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Setup geometry, textures, buffers and shaders
    // Vertices
    float quadVertices[] = {
        // positions     // colors
        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
         0.05f,  0.05f,  0.0f, 1.0f, 1.0f
    };

    glm::vec2 translations[100];
    int index = 0;
    float offset = 0.1f;
    for (int y = -10; y < 10; y += 2)
    {
        for (int x = -10; x < 10; x += 2)
        {
            glm::vec2 translation;
            translation.x = (float)x / 10.0f + offset;
            translation.y = (float)y / 10.0f + offset;
            translations[index++] = translation;
        }
    }

    // Buffer objects
    VAO quadVAO = VAO();
    VBO quadVBO = VBO(quadVertices, sizeof(quadVertices));
    quadVAO.bind();
    quadVAO.linkVBO(quadVBO);
    quadVAO.setAttributes(2, 0, 0, 3);
    quadVAO.unbind();

    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    quadVAO.bind();
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1);


    // Shaders
    Shader shader("../../../src/shaders/instance.vert", "../../../src/shaders/instance.frag");

    shader.use();
    for (unsigned int i = 0; i < 100; i++)
    {
        shader.setVec2(("offsets[" + std::to_string(i) + "]"), translations[i]);
    }

    // Model
    //Model ourModel("../../../src/models/backpack/backpack.obj");

    // Load other textures
    //Texture floorTexture = Texture("../../../src/textures/marble.jpg",
    //    GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Enable depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CCW);

    // Enable stencil testing
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        //glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw
        shader.use();
        quadVAO.bind();
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    quadVAO.Delete();
    quadVBO.Delete();
    glfwTerminate();
    return 0;
}

int asteroid_scene() {
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Setup geometry, textures, buffers and shaders
    // Vertices

    unsigned int amount = 1000;
    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime());
    float radius = 50.0;
    float offset = 2.5f;
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // translation
        float angle = (float)i / amount * 360.0;
        float displacement = (rand() % (int)(2 * offset * 100.0)) / 100.0 - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100.0)) / 100.0 - offset;
        float z = cos(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100.0)) / 100.0 - offset;
        float y = 0.4 * displacement;
        model = glm::translate(model, glm::vec3(x, y, z));
        // scale
        float scale = (rand() % 20) / 100.0 + 0.05;
        model = glm::scale(model, glm::vec3(scale));
        // rotation
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
        // assign
        modelMatrices[i] = model;
    }


    // Shaders
    Shader planetShader("../../../src/shaders/simple.vert", "../../../src/shaders/simple.frag");
    Shader rockShader("../../../src/shaders/asteroid.vert", "../../../src/shaders/simple.frag");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    planetShader.use();
    planetShader.setMat4("projection", projection);
    rockShader.use();
    rockShader.setMat4("projection", projection);

    // Model
    Model planet("../../../src/models/planet/planet.obj");
    Model rock("../../../src/models/rock/rock.obj");

    // Buffer objects
    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * amount, &modelMatrices[0], GL_STATIC_DRAW);
    std::size_t vec4size = sizeof(glm::vec4);
    for (unsigned int i = 0; i < rock.meshes.size() ; i++)
    {
        unsigned int VAO = rock.meshes[i].VAOid;
        glBindVertexArray(VAO);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (void*)0);
        glVertexAttribDivisor(3, 1);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (void*)(1 * vec4size));
        glVertexAttribDivisor(4, 1);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (void*)(2 * vec4size));
        glVertexAttribDivisor(5, 1);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (void*)(3 * vec4size));
        glVertexAttribDivisor(6, 1);
        glBindVertexArray(0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Load other textures
    //Texture floorTexture = Texture("../../../src/textures/marble.jpg",
    //    GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Enable depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);

    // Enable stencil testing
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Enable blending
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        //glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update uniforms
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();

        planetShader.use();
        planetShader.setMat4("view", view);
        model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        planetShader.setMat4("model", model);

        rockShader.use();
        rockShader.setMat4("view", view);

        // Draw
        planetShader.use();
        planet.Draw(planetShader);

        rockShader.use();
        rockShader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);
        for (unsigned int i = 0; i < rock.meshes.size(); i++)
        {
            glBindVertexArray(rock.meshes[i].VAOid);
            glDrawElementsInstanced(GL_TRIANGLES, rock.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
        }


        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    planet.Delete();
    rock.Delete();
    glfwTerminate();
    return 0;
}

int msaa_scene() {
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Setup geometry, textures, buffers and shaders
    // Vertices
    float cubeVertices[] = {
        // positions            // texcoords       
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,

         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,

        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,

        -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f
    };

    float screenVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // Vertex buffers
    // cube
    VAO cubeVAO = VAO();
    VBO cubeVBO = VBO(cubeVertices, sizeof(cubeVertices));
    cubeVAO.bind();
    cubeVAO.linkVBO(cubeVBO);
    cubeVAO.setAttributes(3, 0, 2);
    cubeVAO.unbind();
    // screen
    VAO screenVAO = VAO();
    VBO screenVBO = VBO(screenVertices, sizeof(screenVertices));
    screenVAO.bind();
    screenVAO.linkVBO(screenVBO);
    screenVAO.setAttributes(2, 0, 2);
    screenVAO.unbind();

    // Frame buffers
    FBO multisampleFrameBuffer = FBO();
    multisampleFrameBuffer.bind();
    Texture multiSampleBufferTexture = Texture(SCR_WIDTH, SCR_HEIGHT, GL_RGB, 4);
    multiSampleBufferTexture.attach(GL_COLOR_ATTACHMENT0);
    RBO rbo = RBO(SCR_WIDTH, SCR_HEIGHT, GL_DEPTH24_STENCIL8, 4);
    rbo.attach(GL_DEPTH_STENCIL_ATTACHMENT);
    multisampleFrameBuffer.check_status();
    multisampleFrameBuffer.unbind();

    FBO postProcessFrameBuffer = FBO();
    postProcessFrameBuffer.bind();
    Texture screenBufferTexture = Texture(SCR_WIDTH, SCR_HEIGHT, GL_RGB);
    screenBufferTexture.attach(GL_COLOR_ATTACHMENT0);
    postProcessFrameBuffer.check_status();
    postProcessFrameBuffer.unbind();

    // Shaders
    Shader simpleShader("../../../src/shaders/simple.vert", "../../../src/shaders/solid.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");
    
    simpleShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    simpleShader.setMat4("projection", projection);
    simpleShader.setVec3("color", 0.0, 1.0, 0.0);
    screenShader.use();
    screenShader.setFloat("gamma", 1.0f);

    // Model
    //Model ourModel("../../../src/models/backpack/backpack.obj");

    // Load other textures
    //Texture floorTexture = Texture("../../../src/textures/marble.jpg",
    //    GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    

    // Enable depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // MSAA
    // glEnable(GL_MULTISAMPLE);

    // Enable stencil testing
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        //glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. First pass to multisample buffer
        multisampleFrameBuffer.bind();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        simpleShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        simpleShader.setMat4("model", model);
        glm::mat4 view = camera.GetViewMatrix();
        simpleShader.setMat4("view", view);

        cubeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2. Draw framebuffer to screen
        multisampleFrameBuffer.blit(SCR_WIDTH, SCR_HEIGHT, postProcessFrameBuffer.id);
        multisampleFrameBuffer.unbind();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        
        screenShader.use();
        screenVAO.bind();
        screenBufferTexture.activate(screenShader, "screenTexture", 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    cubeVAO.Delete();
    cubeVBO.Delete();

    screenVAO.Delete();
    screenVBO.Delete();

    multisampleFrameBuffer.Delete();
    postProcessFrameBuffer.Delete();
    rbo.Delete();

    glfwTerminate();
    return 0;
}

int blinn_phong_scene() {
    // Settings
    int ms_samples = 1;
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, ms_samples);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Setup viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // OGL state
    // Depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // MSAA
    if (ms_samples != 1)glEnable(GL_MULTISAMPLE);

    // Gamma correction
    //glEnable(GL_FRAMEBUFFER_SRGB);

    // Stencil testing
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Setup geometry, textures, buffers and shaders
    // Vertices
    float planeVertices[] = {
        // positions            // normals         // texcoords
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f
    };

    // Buffer objects
    VAO planeVAO = VAO();
    VBO planeVBO = VBO(planeVertices, sizeof(planeVertices));
    planeVAO.bind();
    planeVAO.linkVBO(planeVBO);
    planeVAO.setAttributes(3, 3, 2);
    planeVAO.unbind();

    // Shaders
    Shader ourShader("../../../src/shaders/vertex.vert", "../../../src/shaders/blinn_phong.frag");

    ourShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);
    ourShader.setFloat("material.shininess", 64.0);

    // Model
    //Model ourModel("../../../src/models/backpack/backpack.obj");

    // Load other textures
    Texture floorTexture = Texture("../../../src/textures/wood.png", false);

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f, 0.0f, 0.5f)
    };
    glm::vec3 lightColors[] = {
        glm::vec3(1.0f)
    };
    int num_lights = sizeof(lightPositions) / sizeof(lightPositions[0]);
    ourShader.use();
    ourShader.setInt("num_lights", num_lights);
    for (int i = 0; i < num_lights; i++) {
        ourShader.setVec3("light[" + std::to_string(i) + "].position", lightPositions[i]);
        ourShader.setVec3("light[" + std::to_string(i) + "].color", lightColors[i]);
        ourShader.setFloat("light[" + std::to_string(i) + "].attenuation_power", 1.0);
    }

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();

        ourShader.use();
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        ourShader.setVec3("viewPos", camera.Position);

        floorTexture.activate(ourShader, "material.texture_diffuse1", 0);
        floorTexture.activate(ourShader, "material.texture_specular1", 1);

        planeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    planeVAO.Delete();
    planeVBO.Delete();
    glfwTerminate();
    return 0;
}

int gamma_scene() {
    // Settings
    int ms_samples = 4;
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, ms_samples);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    // Depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Stencil testing
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // MSAA
    //glEnable(GL_MULTISAMPLE);

    // Gamma correction
    //glEnable(GL_FRAMEBUFFER_SRGB);
    float gamma = 2.2f;
    bool gamma_correct = (gamma != 1.0);

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices
    float planeVertices[] = {
        // positions            // normals         // texcoords
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,

         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f, 
         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f, 
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f 
    };

    float screenVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // Buffer objects
    // floor
    VAO planeVAO = VAO();
    VBO planeVBO = VBO(planeVertices, sizeof(planeVertices));
    planeVAO.bind();
    planeVAO.linkVBO(planeVBO);
    planeVAO.setAttributes(3, 3, 2);
    planeVAO.unbind();

    // screen
    VAO screenVAO = VAO();
    VBO screenVBO = VBO(screenVertices, sizeof(screenVertices));
    screenVAO.bind();
    screenVAO.linkVBO(screenVBO);
    screenVAO.setAttributes(2, 0, 2);
    screenVAO.unbind();

    // Frame buffers
    // main render
    FBO multisampleFrameBuffer = FBO();
    multisampleFrameBuffer.bind();
    Texture multiSampleBufferTexture = Texture(SCR_WIDTH, SCR_HEIGHT, GL_RGB16, ms_samples);
    multiSampleBufferTexture.attach(GL_COLOR_ATTACHMENT0);
    RBO rbo = RBO(SCR_WIDTH, SCR_HEIGHT, GL_DEPTH24_STENCIL8, ms_samples);
    rbo.attach(GL_DEPTH_STENCIL_ATTACHMENT);
    multisampleFrameBuffer.check_status();
    multisampleFrameBuffer.unbind();
    // post process
    FBO postProcessFrameBuffer = FBO();
    postProcessFrameBuffer.bind();
    Texture screenBufferTexture = Texture(SCR_WIDTH, SCR_HEIGHT, GL_RGB16F);
    screenBufferTexture.attach(GL_COLOR_ATTACHMENT0);
    postProcessFrameBuffer.check_status();
    postProcessFrameBuffer.unbind();

    // Shaders
    Shader bpShader("../../../src/shaders/vertex.vert", "../../../src/shaders/blinn_phong.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");

    bpShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    bpShader.setMat4("projection", projection);
    bpShader.setFloat("material.shininess", 64.0);
    screenShader.use();
    screenShader.setFloat("gamma", gamma);

    // Model
    Model backpackModel("../../../src/models/backpack/backpack.obj", gamma_correct);

    // Load other textures
    Texture floorTexture = Texture("../../../src/textures/wood.png", gamma_correct);

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(-3.0f, 0.0f, 0.5f),
        glm::vec3(-1.0f, 0.0f, 0.5f),
        glm::vec3( 1.0f, 0.0f, 0.5f),
        glm::vec3( 3.0f, 0.0f, 0.5f)
    }; 
    glm::vec3 lightColors[] = {
        glm::vec3(0.25f),
        glm::vec3(0.5f),
        glm::vec3(0.75f),
        glm::vec3(1.0f)
    };
    int num_lights = sizeof(lightPositions) / sizeof(lightPositions[0]);
    bpShader.use();
    bpShader.setInt("num_lights", num_lights);
    for (int i = 0; i < num_lights; i++) {
        bpShader.setVec3("light[" + std::to_string(i) + "].position", lightPositions[i]);
        bpShader.setVec3("light[" + std::to_string(i) + "].color", lightColors[i]);
        bpShader.setFloat("light[" + std::to_string(i) + "].attenuation_power", gamma_correct ? 2.0 : 1.0);
    }
    // Background
    glm::vec3 clear_color = glm::vec3(pow(0.1, gamma)); 

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        // 1. render to texture
        multisampleFrameBuffer.bind(); 
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Draw
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Floor
        bpShader.use();
        bpShader.setMat4("model", model);
        bpShader.setMat4("view", view);
        bpShader.setVec3("viewPos", camera.Position);
        floorTexture.activate(bpShader, "material.texture_diffuse1", 0);
        floorTexture.activate(bpShader, "material.texture_specular1", 1);
        planeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Backpack
        bpShader.use();
        model = glm::translate(model, glm::vec3(0.0, 0.5, 0.0));
        model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
        bpShader.setMat4("model", model);
        bpShader.setMat4("view", view);
        bpShader.setVec3("viewPos", camera.Position);
        backpackModel.Draw(bpShader);

        // 2. Texture to screen
        multisampleFrameBuffer.blit(SCR_WIDTH, SCR_HEIGHT, postProcessFrameBuffer.id);
        multisampleFrameBuffer.unbind();
        postProcessFrameBuffer.unbind();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        
        screenShader.use();
        screenShader.setFloat("exposure", exposure);
        screenBufferTexture.activate(screenShader, "screenTexture", 0);
        screenVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    planeVAO.Delete();
    planeVBO.Delete();
    backpackModel.Delete();
    multisampleFrameBuffer.Delete();
    postProcessFrameBuffer.Delete();
    rbo.Delete();
    glfwTerminate();
    return 0;
}

int quad_cube_test_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 4;

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(MS_SAMPLES>1) glEnable(GL_MULTISAMPLE);

    float gamma = 1.0;
    bool gamma_correct = (gamma != 1.0);
    if (gamma_correct) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader ourShader("../../../src/shaders/simple.vert", "../../../src/shaders/simple.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/screen.frag");

    ourShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);

    // Load textures
    Texture wood = Texture("../../../src/textures/wood.png", gamma_correct);

    TextureData woodData = { wood.id, "texture_diffuse", wood.get_path() };

    // Models & meshes
    Quad quad = Quad(glm::vec2(5.0), 1.0, std::vector<TextureData>{woodData});
    Cube cube = Cube(glm::vec3(1.0), 1.0, std::vector<TextureData>{woodData});

    // Offscreen rendering setup
    PPO ppo = PPO(screenShader, SCR_WIDTH, SCR_HEIGHT, MS_SAMPLES);

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        ppo.start_render_to_texture();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Draw
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();

        ourShader.use();
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        cube.Draw(ourShader);

        model = glm::translate(model, glm::vec3(0.0, -0.5, 0.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        quad.Draw(ourShader);

        ppo.draw_texture_to_screen();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    quad.Delete();
    cube.Delete();
    ppo.Delete();
    glfwTerminate();
    return 0;
}

int shadow_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 1.0;
    bool gamma_correct = (gamma != 1.0);

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader ourShader("../../../src/shaders/bp_shadows.vert", "../../../src/shaders/bp_shadows.frag");
    Shader shadowShader("../../../src/shaders/shadow.vert", "../../../src/shaders/shadow.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");
    Shader depthShader("../../../src/shaders/screen.vert", "../../../src/shaders/depth.frag");

    // Load textures
    Texture wood = Texture("../../../src/textures/wood.png", gamma_correct);
    TextureData woodData = { wood.id, "texture_diffuse", wood.get_path() };

    // Models & meshes
    Model quad = Model(Quad(glm::vec2(50.0), 25.0, std::vector<TextureData>{woodData}));
    Model cube = Model(Cube(glm::vec3(2.0), 1.0, std::vector<TextureData>{woodData}));
    ScreenQuad screen = ScreenQuad();

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 1.5f, 0.0),
        glm::vec3(2.0f, 0.0f, 1.0),
        glm::vec3(-1.0f, 0.0f, 2.0)
    };
    glm::vec3 cubeAxis[] = {
        glm::vec3(0.0f, 1.5f, 0.0),
        glm::vec3(2.0f, 0.0f, 1.0),
        glm::vec3(1.0, 0.0, 1.0)
    }; 
    float cubeAngle[] = {0.0, 0.0, 60.0};
    glm::vec3 cubeScale[] = { glm::vec3(0.5), glm::vec3(0.5), glm::vec3(0.25) };

    // Offscreen rendering setup
    // Post processing
    PPO ppo = PPO(screenShader, SCR_WIDTH, SCR_HEIGHT, MS_SAMPLES);
    // Shadows
    FBO shadowFBO = FBO();
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    Texture shadowMap = Texture(SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, 1, 
        GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, borderColor);
    shadowFBO.bind();
    shadowMap.attach(GL_DEPTH_ATTACHMENT);
    shadowFBO.set_draw_buffer(GL_NONE);
    shadowFBO.set_read_buffer(GL_NONE);
    shadowFBO.unbind();

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(-2.0f, 4.0f, -1.0f)
    };
    glm::vec3 lightColors[] = {
        glm::vec3(1.0f)
    };
    int num_lights = sizeof(lightPositions) / sizeof(lightPositions[0]);
    ourShader.use();
    ourShader.setInt("num_lights", num_lights);
    for (int i = 0; i < num_lights; i++) {
        ourShader.setVec3("pointLights[" + std::to_string(i) + "].position", lightPositions[i]);
        ourShader.setVec3("pointLights[" + std::to_string(i) + "].color", lightColors[i]);
        ourShader.setFloat("pointLights[" + std::to_string(i) + "].attenuation_power", gamma_correct ? 2.0 : 1.0);
    }
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);
    ourShader.setVec3("dirLight.direction", glm::vec3(1.0, 1.0, -1.0));
    ourShader.setVec3("dirLight.color", glm::vec3(0.0));
    ourShader.setFloat("material.shininess", 64.0);
    screenShader.use();
    screenShader.setFloat("gamma", gamma);
    depthShader.use();
    depthShader.setBool("perspective", true);
    // Background
    glm::vec3 clear_color = glm::vec3(pow(0.1, gamma));

    auto renderScene = [&](Shader shader) {
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        // floor
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, -0.5, 0.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        //model = glm::scale(model, glm::vec3(25.0));
        shader.setMat4("model", model);
        quad.Draw(shader);
        // cubes
        for (int i = 0; i < cubePositions->length(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(cubeAngle[i]), glm::normalize(cubeAxis[i]));
            model = glm::scale(model, cubeScale[i]);
            shader.setMat4("model", model);
            cube.Draw(shader);
        }
    };

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        // Shadowmap
        float near_plane = 0.5f, far_plane = 27.5f;
        //glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightProjection = glm::perspective(glm::radians(60.0f),  ((float) SHADOW_WIDTH/ (float)SHADOW_HEIGHT), near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(lightPositions[0], glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        shadowFBO.bind();
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT);
        shadowShader.use();
        //glCullFace(GL_FRONT);
        renderScene(shadowShader);
        //glCullFace(GL_BACK);
        shadowFBO.unbind();


        // Render scene
        if (post_process) ppo.start_render_to_texture();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        //Draw
        glm::mat4 view = camera.GetViewMatrix();
        
        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setMat4("view", view);
        ourShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        wood.activate(ourShader, "texture_diffuse1", 0);
        shadowMap.activate(ourShader, "shadowMap", 1);
        
        renderScene(ourShader);

        if (post_process) ppo.draw_texture_to_screen();
        
        depthShader.use();
        shadowMap.activate(depthShader, "depthMap", 0);
        //screen.Draw();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    quad.Delete();
    cube.Delete();
    ppo.Delete();
    shadowFBO.Delete();
    glfwTerminate();
    return 0;
}

int point_shadow_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;
    bool manual_gamma = false;
    bool gamma_correct = (gamma != 1.0);

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader ourShader("../../../src/shaders/bp_omni_shadow.vert", "../../../src/shaders/bp_omni_shadow.frag");
    Shader shadowShader("../../../src/shaders/point_shadow.vert", "../../../src/shaders/point_shadow.geom", "../../../src/shaders/point_shadow.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");

    // Load textures
    Texture wood = Texture("../../../src/textures/wood.png", gamma_correct);
    TextureData woodData = { wood.id, "texture_diffuse", wood.get_path() };
    
    // Models & meshes
    Model quad = Model(Quad(glm::vec2(50.0), 25.0, std::vector<TextureData>{woodData}));
    Model cube = Model(Cube(glm::vec3(2.0), 1.0, std::vector<TextureData>{woodData}));
    ScreenQuad screen = ScreenQuad();

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0),
        glm::vec3(4.0f, -3.5f, 0.0),
        glm::vec3(2.0f, 3.0f, 1.0),
        glm::vec3(-3.0f, -1.0f, 0.0),
        glm::vec3(-1.5f, 1.0f, 1.5),
        glm::vec3(-1.5f, 2.0f, -3.0)
    };
    glm::vec3 cubeAxis[] = {
        glm::vec3(1.0f),
        glm::vec3(1.0f),
        glm::vec3(1.0f),
        glm::vec3(1.0f),
        glm::vec3(1.0f),
        glm::vec3(1.0, 0.0, 1.0)
    };
    float cubeAngle[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 60.0 };
    glm::vec3 cubeScale[] = { glm::vec3(5.0), glm::vec3(0.5), glm::vec3(0.75), glm::vec3(0.5), glm::vec3(0.5), glm::vec3(0.75) };

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f)
    };
    glm::vec3 lightColors[] = {
        glm::vec3(1.0f)
    };

    // Offscreen rendering setup
    // Post processing
    PPO ppo = PPO(screenShader, SCR_WIDTH, SCR_HEIGHT, MS_SAMPLES);
    // Shadows
    // TODO: Automate shadow map generation
    FBO shadowFBO = FBO();
    Cubemap shadowCubeMap = Cubemap(SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT);
    shadowFBO.bind();
    shadowCubeMap.attachAll(GL_DEPTH_ATTACHMENT);
    shadowFBO.set_draw_buffer(GL_NONE);
    shadowFBO.set_read_buffer(GL_NONE);
    shadowFBO.unbind();

    // shader setup
    ourShader.use();
    int num_lights = sizeof(lightPositions) / sizeof(lightPositions[0]);
    ourShader.setInt("num_lights", num_lights);
    for (int i = 0; i < num_lights; i++) {
        ourShader.setVec3("pointLights[" + std::to_string(i) + "].position", lightPositions[i]);
        ourShader.setVec3("pointLights[" + std::to_string(i) + "].color", lightColors[i]);
        ourShader.setFloat("pointLights[" + std::to_string(i) + "].attenuation_power", gamma_correct ? 2.0 : 1.0);
    }
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);
    ourShader.setFloat("material.shininess", 64.0);

    screenShader.use();
    screenShader.setFloat("gamma", gamma);

    shadowShader.use();
    float near_plane = 1.0f, far_plane = 25.0f;
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), ((float)SHADOW_WIDTH / (float)SHADOW_HEIGHT), near_plane, far_plane);
    std::vector<glm::mat4> lightSpaceTransforms;
    lightSpaceTransforms.push_back(lightProjection *
        glm::lookAt(lightPositions[0], lightPositions[0] + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    lightSpaceTransforms.push_back(lightProjection *
        glm::lookAt(lightPositions[0], lightPositions[0] + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    lightSpaceTransforms.push_back(lightProjection *
        glm::lookAt(lightPositions[0], lightPositions[0] + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    lightSpaceTransforms.push_back(lightProjection *
        glm::lookAt(lightPositions[0], lightPositions[0] + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    lightSpaceTransforms.push_back(lightProjection *
        glm::lookAt(lightPositions[0], lightPositions[0] + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    lightSpaceTransforms.push_back(lightProjection *
        glm::lookAt(lightPositions[0], lightPositions[0] + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowShader.setFloat("far_plane", far_plane);
    shadowShader.setVec3("lightPos", lightPositions[0]);
    for (int i = 0; i < 6; i++) {
        shadowShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", lightSpaceTransforms[i]);
    }

    // Background
    glm::vec3 clear_color = glm::vec3(pow(0.1, gamma));

    auto renderScene = [&](Shader shader) {
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        // cubes
        for (int i = 0; i < sizeof(cubePositions) / sizeof(cubePositions[0]); i++) {
            if (i == 0)
            {
                shader.setBool("reverse_normals", true);
                glFrontFace(GL_CW);
            }
            else 
            {
                shader.setBool("reverse_normals", false);
                glFrontFace(GL_CCW);
            }
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(cubeAngle[i]), glm::normalize(cubeAxis[i]));
            model = glm::scale(model, cubeScale[i]);
            shader.setMat4("model", model);
            cube.Draw(shader);
        }
    };

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        // Shadow pass
        shadowFBO.bind();
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT);
        shadowShader.use();
        renderScene(shadowShader);
        shadowFBO.unbind();


        // Main render pass
        if (post_process) ppo.start_render_to_texture();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = camera.GetViewMatrix();

        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setMat4("view", view);
        ourShader.setFloat("far_plane", far_plane);
        shadowCubeMap.activate(ourShader, "shadowCubeMap", 1);

        renderScene(ourShader);

        if (post_process) ppo.draw_texture_to_screen();

        //depthShader.use();
        //shadowCubeMap.activate(ourShader, "shadowCubeMap", 1);
        //screen.Draw();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    quad.Delete();
    cube.Delete();
    ppo.Delete();
    shadowFBO.Delete();
    glfwTerminate();
    return 0;
}

int normal_map_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;
    bool manual_gamma = false;
    bool gamma_correct = (gamma != 1.0);

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader ourShader("../../../src/shaders/normal_mapping.vert", "../../../src/shaders/normal_mapping.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");
    Shader tbnShader("../../../src/shaders/tbn_visualiser.vert", "../../../src/shaders/tbn_visualiser.geom", "../../../src/shaders/tbn_visualiser.frag");

    // Load textures
    Texture brick = Texture("../../../src/textures/brickwall.jpg", gamma_correct, false);
    Texture brick_normal = Texture("../../../src/textures/brickwall_normal.jpg", false, false);

    TextureData brickData = { brick.id, "texture_diffuse", brick.get_path()};
    TextureData brickNormalData = { brick_normal.id, "texture_normal", brick_normal.get_path() };

    // Models & meshes
    Model quad = Model(Quad(glm::vec2(5.0), 2.0, std::vector<TextureData>{brickData, brickNormalData}));

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f)
    };
    glm::vec3 lightColors[] = {
        glm::vec3(1.0f)
    };

    // Offscreen rendering setup
    // Post processing
    PPO ppo = PPO(screenShader, SCR_WIDTH, SCR_HEIGHT, MS_SAMPLES);

    // shader setup
    ourShader.use();
    int num_lights = sizeof(lightPositions) / sizeof(lightPositions[0]);
    ourShader.setInt("num_lights", num_lights);
    for (int i = 0; i < num_lights; i++) {
        ourShader.setVec3("light[" + std::to_string(i) + "].position", lightPositions[i]);
        ourShader.setVec3("light[" + std::to_string(i) + "].color", lightColors[i]);
        ourShader.setFloat("light[" + std::to_string(i) + "].attenuation_power", gamma_correct ? 2.0 : 1.0);
    }
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);
    ourShader.setFloat("material.shininess", 64.0);
    ourShader.setVec3("lightPos", lightPositions[0]);

    screenShader.use();
    screenShader.setFloat("gamma", gamma);

    // Background
    glm::vec3 clear_color = glm::vec3(pow(0.1, gamma));

    auto renderScene = [&](Shader shader) {
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, -0.5, -0.5));
        model = glm::rotate(model, glm::radians(-60.0f), glm::vec3(1.0, 0.0, 0.0));
        shader.setMat4("model", model);
        quad.Draw(shader);
        };

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering

        // Main render pass
        if (post_process) ppo.start_render_to_texture();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = camera.GetViewMatrix();

        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setMat4("view", view);
        ourShader.setBool("normal_mapping", normal_mapping);
        renderScene(ourShader);

        tbnShader.use();
        tbnShader.setMat4("view", view);
        tbnShader.setMat4("projection", projection);
        renderScene(tbnShader);

        if (post_process) ppo.draw_texture_to_screen();

        //depthShader.use();
        //shadowCubeMap.activate(ourShader, "shadowCubeMap", 1);
        //screen.Draw();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    quad.Delete();
    ppo.Delete();
    glfwTerminate();
    return 0;
}

int parallax_map_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;
    bool manual_gamma = false;
    bool gamma_correct = (gamma != 1.0);

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader ourShader("../../../src/shaders/normal_mapping.vert", "../../../src/shaders/parallax.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");
    Shader tbnShader("../../../src/shaders/tbn_visualiser.vert", "../../../src/shaders/tbn_visualiser.geom", "../../../src/shaders/tbn_visualiser.frag");

    // Load textures
    
    Texture brick_albedo = Texture("../../../src/textures/brick_diffuse.png", gamma_correct, true);
    Texture brick_normal = Texture("../../../src/textures/brick_normal.png", false, true);
    Texture brick_height = Texture("../../../src/textures/brick_displacement.png", false, true);
    /*
    Texture brick_albedo = Texture("../../../src/textures/bricks2.jpg", gamma_correct, false);
    Texture brick_normal = Texture("../../../src/textures/bricks2_normal.jpg", false, false);
    Texture brick_height = Texture("../../../src/textures/bricks2_disp.jpg", false, false, true);
    
    Texture brick_albedo = Texture("../../../src/textures/toy_box_diffuse.png", gamma_correct, false);
    Texture brick_normal = Texture("../../../src/textures/toy_box_normal.png", false, false);
    Texture brick_height = Texture("../../../src/textures/toy_box_disp.png", false, false, true);
    */
    TextureData brickAlbedoData = { brick_albedo.id, "texture_diffuse", brick_albedo.get_path() };
    TextureData brickNormalData = { brick_normal.id, "texture_normal", brick_normal.get_path() };
    TextureData brickHeightData = { brick_height.id, "texture_height", brick_height.get_path() };

    // Models & meshes
    Model quad = Model(Quad(glm::vec2(4.0), 1.0, std::vector<TextureData>{brickAlbedoData, brickNormalData, brickHeightData}));
    Model cube = Model(Cube(glm::vec3(1.0), 0.25, std::vector<TextureData>{brickAlbedoData, brickNormalData, brickHeightData}));

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(2.0f)
    };

    // Offscreen rendering setup
    // Post processing
    PPO ppo = PPO(screenShader, SCR_WIDTH, SCR_HEIGHT, MS_SAMPLES);

    // shader setup
    ourShader.use();
    int num_lights = sizeof(lightPositions) / sizeof(lightPositions[0]);
    ourShader.setInt("num_lights", num_lights);
    for (int i = 0; i < num_lights; i++) {
        ourShader.setVec3("light[" + std::to_string(i) + "].position", lightPositions[i]);
        ourShader.setVec3("light[" + std::to_string(i) + "].color", lightColors[i]);
        ourShader.setFloat("light[" + std::to_string(i) + "].attenuation_power", gamma_correct ? 2.0 : 1.0);
    }
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);
    ourShader.setFloat("material.shininess", 64.0);
    ourShader.setVec3("lightPos", lightPositions[0]);

    screenShader.use();
    screenShader.setFloat("gamma", gamma);

    // Background
    glm::vec3 clear_color = glm::vec3(pow(0.1, gamma));

    auto renderScene = [&](Shader shader) {
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 0.0, -1.75));
        //model = glm::rotate(model, glm::radians(-60.0f), glm::vec3(1.0, 0.0, 0.0));
        shader.setMat4("model", model);
        quad.Draw(shader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0, -1.0, -1.0));
        shader.setMat4("model", model);
        cube.Draw(shader);
        };

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering

        // Main render pass
        if (post_process) ppo.start_render_to_texture();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = camera.GetViewMatrix();

        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setMat4("view", view);
        ourShader.setBool("normal_mapping", normal_mapping);
        ourShader.setFloat("height_scale", heightScale);
        renderScene(ourShader);

        tbnShader.use();
        tbnShader.setMat4("view", view);
        tbnShader.setMat4("projection", projection);
        renderScene(tbnShader);

        if (post_process) ppo.draw_texture_to_screen();

        //depthShader.use();
        //shadowCubeMap.activate(ourShader, "shadowCubeMap", 1);
        //screen.Draw();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    quad.Delete();
    ppo.Delete();
    glfwTerminate();
    return 0;
}

int hdr_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;
    bool manual_gamma = true;
    bool gamma_correct = (gamma != 1.0);

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader ourShader("../../../src/shaders/blinn_phong.vert", "../../../src/shaders/blinn_phong.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");

    // Load textures
    Texture wood = Texture("../../../src/textures/wood.png", gamma_correct);
    TextureData woodData = { wood.id, "texture_diffuse", wood.get_path() };

    // Models & meshes
    Model cube = Model(Cube(glm::vec3(2.0), 1.0, std::vector<TextureData>{woodData}));

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f,  0.0f, 49.5f),
        glm::vec3(-1.4f, -1.9f, 9.0f),
        glm::vec3(0.0f, -1.8f, 4.0f),
        glm::vec3(0.8f, -1.7f, 6.0f)
    };
    glm::vec3 lightColors[] = {
        glm::vec3(200.0f, 200.0f, 200.0f), 
        glm::vec3(0.1f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.2f), 
        glm::vec3(0.0f, 0.1f, 0.0f)
    };

    // Offscreen rendering setup
    // Post processing
    PPO ppo = PPO(screenShader, SCR_WIDTH, SCR_HEIGHT, MS_SAMPLES);

    // shader setup
    ourShader.use();
    int num_lights = sizeof(lightPositions) / sizeof(lightPositions[0]);
    ourShader.setInt("num_lights", num_lights);
    for (int i = 0; i < num_lights; i++) {
        ourShader.setVec3("light[" + std::to_string(i) + "].position", lightPositions[i]);
        ourShader.setVec3("light[" + std::to_string(i) + "].color", lightColors[i]);
        ourShader.setFloat("light[" + std::to_string(i) + "].attenuation_power", gamma_correct ? 2.0 : 1.0);
    }
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setFloat("material.shininess", 64.0);

    

    // Background
    glm::vec3 clear_color = glm::vec3(pow(0.1, gamma));

    auto renderScene = [&](Shader shader) {
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 25.0));
        model = glm::scale(model, glm::vec3(2.5f, 2.5f, 27.5f));
        shader.setMat4("model", model);
        shader.setBool("reverse_normals", true);
        glFrontFace(GL_CW);
        cube.Draw(shader);
        glFrontFace(GL_CCW);
        };

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering

        // Main render pass
        if (post_process) ppo.start_render_to_texture();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = camera.GetViewMatrix();

        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        renderScene(ourShader);

        screenShader.use();
        screenShader.setFloat("gamma", gamma);
        screenShader.setFloat("exposure", exposure);
        if (post_process) ppo.draw_texture_to_screen();

        //depthShader.use();
        //shadowCubeMap.activate(ourShader, "shadowCubeMap", 1);
        //screen.Draw();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    cube.Delete();
    ppo.Delete();
    glfwTerminate();
    return 0;
}

int bloom_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;
    bool manual_gamma = true;
    bool gamma_correct = abs(gamma - 2.2) < 0.01;

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader ourShader("../../../src/shaders/blinn_phong.vert", "../../../src/shaders/multirender_bp.frag");
    Shader lightShader("../../../src/shaders/blinn_phong.vert", "../../../src/shaders/bloom_light.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");

    // Load textures
    Texture wood = Texture("../../../src/textures/wood.png", gamma_correct);
    TextureData woodData = { wood.id, "texture_diffuse", wood.get_path() };
    Texture container = Texture("../../../src/textures/container2.png", gamma_correct);
    TextureData containerData = { container.id, "texture_diffuse", container.get_path() };

    // Models & meshes
    Model cube = Model(Cube(glm::vec3(2.0), 1.0, std::vector<TextureData>{woodData}));
    Model box = Model(Cube(glm::vec3(2.0), 1.0, std::vector<TextureData>{containerData}));

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f, 0.5f, 1.5f),
        glm::vec3(-4.0f, 0.5f, -3.0f),
        glm::vec3(3.0f, 0.5f, 1.0f),
        glm::vec3(-.8f, 2.4f, -1.0f)
    };
    glm::vec3 lightColors[] = {
        glm::vec3(5.0f,   5.0f,  5.0f),
        glm::vec3(10.0f,  0.0f,  0.0f),
        glm::vec3(0.0f,   0.0f,  15.0f),
        glm::vec3(0.0f,   5.0f,  0.0f)
    };

    // Offscreen rendering setup
    // Post processing
    PPO ppo = PPO(screenShader, SCR_WIDTH, SCR_HEIGHT, MS_SAMPLES);

    // shader setup
    ourShader.use();
    int num_lights = sizeof(lightPositions) / sizeof(lightPositions[0]);
    ourShader.setInt("num_lights", num_lights);
    for (int i = 0; i < num_lights; i++) {
        ourShader.setVec3("light[" + std::to_string(i) + "].position", lightPositions[i]);
        ourShader.setVec3("light[" + std::to_string(i) + "].color", lightColors[i]);
        ourShader.setFloat("light[" + std::to_string(i) + "].attenuation_power", gamma_correct ? 2.0 : 1.0);
    }
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setFloat("material.shininess", 64.0);


    // Background
    //glm::vec3 clear_color = glm::vec3(pow(0.1, gamma));
    float clear_color[] = { pow(0.1, gamma), pow(0.1, gamma), pow(0.1, gamma), 1.0 };

    auto renderScene = [&]() {
        ourShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        // create one large cube that acts as the floor
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
        model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
        ourShader.setMat4("model", model);
        cube.Draw(ourShader);
        // then create multiple cubes as the scenery
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        ourShader.setMat4("model", model);
        box.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
        model = glm::scale(model, glm::vec3(0.5f));
        ourShader.setMat4("model", model);
        box.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
        model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        ourShader.setMat4("model", model);
        box.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
        model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(1.25));
        ourShader.setMat4("model", model);
        box.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
        model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        ourShader.setMat4("model", model);
        box.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        ourShader.setMat4("model", model);
        box.Draw(ourShader);

        lightShader.use();
        for (unsigned int i = 0; i < num_lights; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(lightPositions[i]));
            model = glm::scale(model, glm::vec3(0.25f));
            lightShader.setMat4("model", model);
            lightShader.setVec3("lightColor", lightColors[i]);
            cube.Draw(lightShader);
        }
        };

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering

        // Main render pass

        if (post_process) ppo.start_render_to_texture();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearBufferfv(GL_COLOR, 0, clear_color);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = camera.GetViewMatrix();

        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        lightShader.use();
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);
        renderScene();

        screenShader.use();
        screenShader.setFloat("gamma", gamma);
        screenShader.setFloat("exposure", exposure);
        screenShader.setFloat("bloom", bloom);

        if (post_process) ppo.bloom();

        if (post_process) ppo.draw_texture_to_screen();

        //depthShader.use();
        //shadowCubeMap.activate(ourShader, "shadowCubeMap", 1);
        //screen.Draw();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    cube.Delete();
    ppo.Delete();
    glfwTerminate();
    return 0;
}

int deferred_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;
    bool manual_gamma = true;
    bool gamma_correct = abs(gamma - 2.2) < 0.01;

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);
    
    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    std::cout << "SCR: " << SCR_WIDTH << "x" << SCR_HEIGHT << std::endl;
    std::cout << "Framebuffer: " << fbWidth << "x" << fbHeight << std::endl;

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader geometryPassShader("../../../src/shaders/vertex.vert", "../../../src/shaders/deferred_geometry_pass.frag");
    Shader lightingPassShader("../../../src/shaders/screen.vert", "../../../src/shaders/deferred_lighting_pass.frag");
    Shader lightBoxShader("../../../src/shaders/simple.vert", "../../../src/shaders/solid.frag");
    Shader depthShader("../../../src/shaders/screen.vert", "../../../src/shaders/depth_debug.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/screen.frag");
    Shader ppfxShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");

    // Load textures

    // Models & meshes
    Model cube = Model(Cube(glm::vec3(2.0), 1.0, std::vector<TextureData>{}));
    ScreenQuad screen = ScreenQuad();
    Model backpack("../../../src/models/backpack/backpack.obj", gamma_correct);
    std::vector<glm::vec3> objectPositions;
    objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 3.0));

    // Lights
    const unsigned int NR_LIGHTS = 32;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    srand(13);
    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        // calculate slightly random offsets
        float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
        float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
        // also calculate random color
        float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        lightColors.push_back(glm::vec3(rColor, gColor, bColor));
    }

    // Render object setup
    PPO ppo = PPO(ppfxShader, SCR_WIDTH, SCR_HEIGHT, MS_SAMPLES);
    GBO GBuffer = GBO(SCR_WIDTH, SCR_HEIGHT, geometryPassShader, lightingPassShader);

    // shader setup


    // Background
    float clear_color[] = { pow(0.1, gamma), pow(0.1, gamma), pow(0.1, gamma), 1.0 };


    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering

        // Geometry Pass
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GBuffer.geometry_pass();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearBufferfv(GL_COLOR, 0, clear_color);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        
        geometryPassShader.use();
        
        for (unsigned int i = 0; i < objectPositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, objectPositions[i]);
            model = glm::scale(model, glm::vec3(0.5f));
            geometryPassShader.setMat4("model", model);
            geometryPassShader.setMat4("view", view);
            geometryPassShader.setMat4("projection", projection);
            backpack.Draw(geometryPassShader);
        }
        
        // Deferred Lighting Pass
        // Update light uniforms
        lightingPassShader.use();
        lightingPassShader.setVec3("viewPos", camera.Position);
        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].color", lightColors[i]);
            lightingPassShader.setFloat("lights[" + std::to_string(i) + "].attenuation", gamma_correct ? 2.0 : 1.0);
        }
        GBuffer.lighting_pass(ppo.renderBuffer.id);

        

        // Forward overlay pass
        GBuffer.fbo.blit(SCR_WIDTH, SCR_HEIGHT, ppo.renderBuffer.id, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //depthShader.use();
        //screen.Draw();
        ppo.renderBuffer.bind();

        lightBoxShader.use();
        lightBoxShader.setMat4("projection", projection);
        lightBoxShader.setMat4("view", view);
        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.25f));
            lightBoxShader.setMat4("model", model);
            lightBoxShader.setVec3("color", lightColors[i]);
            cube.Draw(lightBoxShader);
        }
        ppo.renderBuffer.unbind();

        //screenShader.use();
        //ppo.renderTexture.activate(screenShader, "screenTexture", 0);
        //screen.Draw();
        ppfxShader.use();
        ppfxShader.setFloat("gamma", gamma);
        ppfxShader.setFloat("exposure", exposure);
        ppfxShader.setFloat("bloom", bloom); 
        ppo.draw_texture_to_screen();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    ppo.Delete();
    GBuffer.Delete();
    glfwTerminate();
    return 0;
}

int ssao_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;      // best to use 2.2
    bool manual_gamma = true;
    bool gamma_correct = abs(gamma - 2.2) < 0.01;

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    std::cout << "SCR: " << SCR_WIDTH << "x" << SCR_HEIGHT << std::endl;
    std::cout << "Framebuffer: " << fbWidth << "x" << fbHeight << std::endl;

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader geometryPassShader("../../../src/shaders/vertex_invertable.vert", "../../../src/shaders/deferred_geometry_pass.frag");
    Shader lightingPassShader("../../../src/shaders/screen.vert", "../../../src/shaders/deferred_lighting_pass.frag");
    Shader ssaoShader("../../../src/shaders/screen.vert", "../../../src/shaders/ssao.frag");
    Shader ssaoBlurShader("../../../src/shaders/screen.vert", "../../../src/shaders/ssaoBlur.frag");
    Shader ppfxShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ssao_visualiser.frag");

    // Load textures
    Texture wood = Texture("../../../src/textures/wood.png", gamma_correct);
    TextureData woodData = { wood.id, "texture_diffuse", wood.get_path() };

    // Models & meshes
    Model cube = Model(Cube(glm::vec3(2.0), 1.0, std::vector<TextureData>{woodData}));
    ScreenQuad screen = ScreenQuad();
    Model backpack("../../../src/models/backpack/backpack.obj", gamma_correct);

    // Lights
    const unsigned int NR_LIGHTS = 32;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;

    lightPositions.push_back(glm::vec3(2.0, 4.0, -2.0));
    lightColors.push_back(glm::vec3(0.3, 0.3, 0.9));

    // Render object setup
    PPO ppo = PPO(ppfxShader, SCR_WIDTH, SCR_HEIGHT, MS_SAMPLES);
    GBO GBuffer = GBO(SCR_WIDTH, SCR_HEIGHT, geometryPassShader, lightingPassShader);
    FBO ssaoFBO = FBO();
    Texture ssaoColorBuffer = Texture(SCR_WIDTH, SCR_HEIGHT, GL_RED, 1, GL_NEAREST, GL_NEAREST);
    ssaoFBO.bind();
    ssaoColorBuffer.attach(GL_COLOR_ATTACHMENT0);
    ssaoFBO.check_status();
    ssaoFBO.unbind();
    FBO ssaoBlurFBO = FBO();
    Texture ssaoBlurBuffer = Texture(SCR_WIDTH, SCR_HEIGHT, GL_RED, 1, GL_NEAREST, GL_NEAREST);
    ssaoBlurFBO.bind();
    ssaoBlurBuffer.attach(GL_COLOR_ATTACHMENT0);
    ssaoBlurFBO.check_status();
    ssaoBlurFBO.unbind();

    // shader setup

    // SSAO setup
    // These samples are not properly distributed. 
    // By sampling a cube and reshaping to a sphere there is oversampling near the corners of the original cube. 
    // Instead, use rejeciton sampling, or sample phi/theta/rho in correct proportions. 
    int ssaoKernelSize = 64;
    std::default_random_engine generator;
    std::normal_distribution<float> normalFloats(0.0, 1.0);
    std::uniform_real_distribution<float> uniformFloats(0.0, 1.0);

    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < ssaoKernelSize; ++i)
    {
        float r = std::sqrt(uniformFloats(generator));    // sqrt for uniform disk sampling
        float phi = 2.0f * glm::pi<float>() * uniformFloats(generator);
        glm::vec3 sample(
            r* std::cos(phi),
            r* std::sin(phi),
            std::sqrt(1.0f - r * r));   // z = sqrt(1 - x^2 - y^2), always positive
        sample *= uniformFloats(generator);
        float scale = (float)i / ssaoKernelSize;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    std::vector<glm::vec3> ssaoOldKernel;
    for (unsigned int i = 0; i < ssaoKernelSize; ++i)
    {
        glm::vec3 sample(
            normalFloats(generator),
            normalFloats(generator),
            std::abs(normalFloats(generator)));
        sample = glm::normalize(sample);
        sample *= uniformFloats(generator);
        float scale = (float)i / ssaoKernelSize;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoOldKernel.push_back(sample);
    }

    std::vector<glm::vec3> ssaoBadKernel;
    for (unsigned int i = 0; i < ssaoKernelSize; ++i)
    {
        glm::vec3 sample(
            uniformFloats(generator) * 2.0 - 1.0,
            uniformFloats(generator) * 2.0 - 1.0,
            uniformFloats(generator));
        sample = glm::normalize(sample);
        sample *= uniformFloats(generator); 
        float scale = (float)i / ssaoKernelSize;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoBadKernel.push_back(sample);
    }

    int ssaoNoiseSize = 4;
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < ssaoNoiseSize * ssaoNoiseSize; i++) {
        glm::vec3 noise(
            normalFloats(generator),
            normalFloats(generator),
            0.0);
        ssaoNoise.push_back(glm::normalize(noise));
    }

    std::vector<glm::vec3> ssaoBadNoise;
    for (unsigned int i = 0; i < ssaoNoiseSize * ssaoNoiseSize; i++) {
        glm::vec3 noise(
            uniformFloats(generator) * 2.0 - 1.0,
            uniformFloats(generator) * 2.0 - 1.0,
            0.0);
        ssaoBadNoise.push_back(noise);
    }

    Texture ssaoNoiseTexture = Texture(ssaoNoiseSize, ssaoNoiseSize, GL_RGB16F, &ssaoNoise, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
    Texture ssaoBadNoiseTexture = Texture(ssaoNoiseSize, ssaoNoiseSize, GL_RGB16F, &ssaoBadNoise, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);

    auto vec3ToString = [](const glm::vec3& v) -> std::string {
        return "(" + std::to_string(v.x) + ", "
            + std::to_string(v.y) + ", "
            + std::to_string(v.z) + ")";
        };

    // Background
    float clear_color[] = { pow(0.1, gamma), pow(0.1, gamma), pow(0.1, gamma), 1.0 };


    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering

        // Geometry Pass
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GBuffer.geometry_pass();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearBufferfv(GL_COLOR, 0, clear_color);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);

        geometryPassShader.use();

        // Update geometry uniforms
        // view and projection
        geometryPassShader.setMat4("view", view);
        geometryPassShader.setMat4("projection", projection);
        // model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 7.0f, 0.0f));
        model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));
        geometryPassShader.setMat4("model", model);
        geometryPassShader.setInt("invertedNormals", 1); // invert normals as we're inside the cube
        glFrontFace(GL_CW);
        cube.Draw(geometryPassShader);
        geometryPassShader.setInt("invertedNormals", 0); // reset to correct normals 
        glFrontFace(GL_CCW);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        model = glm::scale(model, glm::vec3(1.0f));
        geometryPassShader.setMat4("model", model);
        backpack.Draw(geometryPassShader);


        // SSAO pass
        ssaoFBO.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoShader.use();
        GBuffer.positionGbuffer.activate(ssaoShader, "gPosition", 0);
        GBuffer.normalGbuffer.activate(ssaoShader, "gNormal", 1);
        GBuffer.albedoSpecGBuffer.activate(ssaoShader, "gAlbedoSpec", 2);
        toggle ? ssaoNoiseTexture.activate(ssaoShader, "ssaoNoise", 3): ssaoBadNoiseTexture.activate(ssaoShader, "ssaoNoise", 3);
        ssaoShader.setMat4("projection", projection);
        ssaoShader.setMat4("view", view);
        for (unsigned int i = 0; i < ssaoKernelSize; i++) {
            ssaoShader.setVec3("samples[" + std::to_string(i) + "]", toggle ? ssaoKernel[i]: ssaoBadKernel[i]);
        }
        std::string message = toggle ? "good kernel" : "bad kernel";
        std::cout << message << std::endl;
        ssaoShader.setInt("kernelSize", ssaoKernelSize);
        ssaoShader.setFloat("radius", 0.5);
        ssaoShader.setFloat("bias", 0.05);
        ssaoShader.setFloat("occlusionStrength", 1.0);
        screen.Draw();
        ssaoFBO.unbind();

        ssaoBlurFBO.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoBlurShader.use();
        ssaoColorBuffer.activate(ssaoBlurShader, "ssaoInput", 0);
        ssaoBlurShader.setInt("ssaoNoiseSize", ssaoNoiseSize);
        screen.Draw();
        ssaoBlurFBO.unbind();


        // Deferred Lighting Pass
        // Update light uniforms
        lightingPassShader.use();
        lightingPassShader.setVec3("viewPos", camera.Position);
        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
            lightingPassShader.setVec3("lights[" + std::to_string(i) + "].color", lightColors[i]);
            lightingPassShader.setFloat("lights[" + std::to_string(i) + "].attenuation", gamma_correct ? 2.0 : 1.0);
        }
        ssaoBlurBuffer.activate(lightingPassShader, "ssaoColorBuffer", 3);
        lightingPassShader.setBool("ssaoFlag", ssaoFlag);
        //calculate lighting
        GBuffer.lighting_pass(ppo.renderBuffer.id);


        // Forward overlay pass
        GBuffer.fbo.blit(SCR_WIDTH, SCR_HEIGHT, ppo.renderBuffer.id, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        ppo.renderBuffer.bind();
        // add overlay elements here
        ppo.renderBuffer.unbind();

        
        // draw to screen
        ppfxShader.use();
        ppfxShader.setFloat("gamma", gamma);
        ppfxShader.setFloat("exposure", exposure);
        ppfxShader.setFloat("bloom", bloom);
        ppo.draw_texture_to_screen();
        
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //screenShader.use();
        //ssaoBlurBuffer.activate(screenShader, "screenTexture", 0);
        //screen.Draw();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    ppo.Delete();
    GBuffer.Delete();
    cube.Delete();
    screen.Delete();
    ssaoFBO.Delete();
    glfwTerminate();
    return 0;
}

int kernel_test_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;      // best to use 2.2
    bool manual_gamma = true;
    bool gamma_correct = abs(gamma - 2.2) < 0.01;

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    std::cout << "SCR: " << SCR_WIDTH << "x" << SCR_HEIGHT << std::endl;
    std::cout << "Framebuffer: " << fbWidth << "x" << fbHeight << std::endl;

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader ourShader("../../../src/shaders/kernel_visualiser.vert", "../../../src/shaders/solid.frag");
    Shader floorShader("../../../src/shaders/vertex.vert", "../../../src/shaders/solid.frag");

    // Load textures

    // Models & meshes
    std::vector<TextureData> empty;
    Model floor = Model(Quad(glm::vec2(2.0), 1.0, empty));

    // Lights

    // Render object setup

    // shader setup

    // SSAO setup
    // These samples are not properly distributed. 
    // By sampling a cube and reshaping to a sphere there is oversampling near the corners of the original cube. 
    // Instead, use rejeciton sampling, or sample phi/theta/rho in correct proportions. 
    const int ssaoKernelSize = 512;
    std::vector<glm::vec3> ssaoKernel;
    std::default_random_engine generator;
    std::normal_distribution<float> normalFloats(0.0, 1.0);
    std::uniform_real_distribution<float> uniformFloats(0.0, 1.0);
    for (unsigned int i = 0; i < ssaoKernelSize; ++i)
    {
        glm::vec3 sample(
            normalFloats(generator),
            normalFloats(generator),
            std::abs(normalFloats(generator)));
        sample = glm::normalize(sample);
        sample *= uniformFloats(generator);
        float scale = (float)i / ssaoKernelSize;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(glm::vec3(0.0));
        ssaoKernel.push_back(sample);
    }

    std::vector<glm::vec3> ssaoBadKernel;
    for (unsigned int i = 0; i < ssaoKernelSize; ++i)
    {
        glm::vec3 sample(
            uniformFloats(generator) * 2.0 - 1.0,
            uniformFloats(generator) * 2.0 - 1.0,
            uniformFloats(generator));
        sample = glm::normalize(sample);
        sample *= uniformFloats(generator);
        float scale = (float)i / ssaoKernelSize;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoBadKernel.push_back(glm::vec3(0.0));
        ssaoBadKernel.push_back(sample);
    }

    int ssaoNoiseSize = 4;
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(
            uniformFloats(generator) * 2.0 - 1.0,
            uniformFloats(generator) * 2.0 - 1.0,
            0.0);
        ssaoNoise.push_back(noise);
    }

    std::vector<glm::vec3> ssaoBadNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(
            uniformFloats(generator) * 2.0 - 1.0,
            uniformFloats(generator) * 2.0 - 1.0,
            0.0);
        ssaoBadNoise.push_back(noise);
    }

    Texture ssaoNoiseTexture = Texture(ssaoNoiseSize, ssaoNoiseSize, GL_RGB16F, &ssaoNoise, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);

    auto vec3ToString = [](const glm::vec3& v) -> std::string {
        return "(" + std::to_string(v.x) + ", "
            + std::to_string(v.y) + ", "
            + std::to_string(v.z) + ")";
        };

    VAO vaoGood = VAO();
    VBO vboGood = VBO(ssaoKernel);
    vaoGood.bind();
    vaoGood.linkVBO(vboGood);
    vaoGood.setAttributes(3, 0, 0, 0);
    vaoGood.unbind();

    VAO vaoBad = VAO();
    VBO vboBad = VBO(ssaoBadKernel);
    vaoBad.bind();
    vaoBad.linkVBO(vboBad);
    vaoBad.setAttributes(3, 0, 0, 0);
    vaoBad.unbind();



    // Background
    //float clear_color[] = { pow(0.1, gamma), pow(0.1, gamma), pow(0.1, gamma), 1.0 };
    float clear_color[] = { 0.0, 0.5, 0.5, 1.0 };


    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearBufferfv(GL_COLOR, 0, clear_color);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

        ourShader.use();
        ourShader.setVec3("color", glm::vec3(1.0, 0.0, 0.0));
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("model", model);
        toggle ? vaoGood.bind(): vaoBad.bind();
        std::string message = toggle ? "good kernel" : "bad kernel";
        std::cout << message << std::endl;
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(ssaoKernel.size()));

        floorShader.use();
        floorShader.setVec3("color", glm::vec3(0.3, 0.3, 0.3));
        floorShader.setMat4("view", view);
        floorShader.setMat4("projection", projection);
        floorShader.setMat4("model", model);
        floor.Draw(floorShader);


        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    glfwTerminate();
    return 0;
}

int pbr_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;      // best to use 2.2
    bool manual_gamma = true;
    bool gamma_correct = abs(gamma - 2.2) < 0.01;

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    std::cout << "SCR: " << SCR_WIDTH << "x" << SCR_HEIGHT << std::endl;
    std::cout << "Framebuffer: " << fbWidth << "x" << fbHeight << std::endl;

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader pbrShader("../../../src/shaders/vertex_advanced.vert", "../../../src/shaders/pbr_param.frag");

    // Load textures

    // Models & meshes
    Model sphere = Model("../../../src/models/sphere/sphere.obj", gamma_correct);

    // Model transformations
    int nrRows = 4;
    int nrColumns = 4;
    float spacing = 2.5;

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3(10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    // Render object setup

    // shader setup
    
    // Background
    float clear_color[] = { pow(0.1, gamma), pow(0.1, gamma), pow(0.1, gamma), 1.0 };

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearBufferfv(GL_COLOR, 0, clear_color);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

        pbrShader.use();
        pbrShader.setMat4("view", view);
        pbrShader.setMat4("projection", projection);
        pbrShader.setInt("numLights", sizeof(lightPositions) / sizeof(lightPositions[0]));
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            pbrShader.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
            pbrShader.setVec3("lights[" + std::to_string(i) + "].color", lightColors[i]);
        }
        pbrShader.setVec3("camPos", camera.Position);
        pbrShader.setVec3("albedo", 0.5f, 0.0f, 0.0f);
        pbrShader.setFloat("occlusion", 1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            pbrShader.setFloat("metalness", (float)row / (float)(nrRows - 1));
            //pbrShader.setFloat("metalness", 1.0);
            for (int col = 0; col < nrColumns; ++col)
            {
                // we clamp the roughness to 0.05 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting.
                pbrShader.setFloat("roughness", glm::clamp((float)col / (float)(nrColumns - 1), 0.05f, 1.0f));

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (col - (nrColumns / 2)) * spacing,
                    (row - (nrRows / 2)) * spacing,
                    0.0f
                ));
                pbrShader.setMat4("model", model);
                pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                sphere.Draw(pbrShader);
            }
        }


        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    glfwTerminate();
    return 0;
}

int pbr_texture_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;      // best to use 2.2
    bool manual_gamma = true;
    bool gamma_correct = abs(gamma - 2.2) < 0.01;

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader pbrShader("../../../src/shaders/vertex_advanced.vert", "../../../src/shaders/pbr_text.frag");

    // Load textures
    Texture rustedIronAlbedo("../../../src/textures/rusted_iron/albedo.png", gamma_correct);
    Texture rustedIronNormal("../../../src/textures/rusted_iron/normal.png", gamma_correct);
    Texture rustedIronORM("../../../src/textures/rusted_iron/ORM.png", gamma_correct);

    // Models & meshes
    Model sphere = Model("../../../src/models/sphere/sphere.obj", gamma_correct);

    // Model transformations
    int nrRows = 4;
    int nrColumns = 4;
    float spacing = 2.5;

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3(10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    // Render object setup

    // shader setup

    // Background
    float clear_color[] = { pow(0.1, gamma), pow(0.1, gamma), pow(0.1, gamma), 1.0 };

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearBufferfv(GL_COLOR, 0, clear_color);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

        pbrShader.use();
        pbrShader.setMat4("view", view);
        pbrShader.setMat4("projection", projection);
        pbrShader.setInt("numLights", sizeof(lightPositions) / sizeof(lightPositions[0]));
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            pbrShader.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
            pbrShader.setVec3("lights[" + std::to_string(i) + "].color", lightColors[i]);
        }
        pbrShader.setVec3("camPos", camera.Position);
        rustedIronAlbedo.activate(pbrShader, "AlbedoMap", 0);
        rustedIronNormal.activate(pbrShader, "NormalMap", 1);
        rustedIronORM.activate(pbrShader, "ORMMap", 2);
        for (int row = 0; row < nrRows; ++row)
        {
            //pbrShader.setFloat("metalness", 1.0);
            for (int col = 0; col < nrColumns; ++col)
            {
                // we clamp the roughness to 0.05 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting.
                pbrShader.setFloat("roughness", glm::clamp((float)col / (float)(nrColumns - 1), 0.05f, 1.0f));

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (col - (nrColumns / 2)) * spacing,
                    (row - (nrRows / 2)) * spacing,
                    0.0f
                ));
                pbrShader.setMat4("model", model);
                pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                sphere.Draw(pbrShader);
            }
        }

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    glfwTerminate();
    return 0;
}

int ibl_scene() {
    // Variable setup
    const unsigned int MS_SAMPLES = 1;
    float gamma = 2.2;      // best to use 2.2
    bool manual_gamma = true;
    bool gamma_correct = abs(gamma - 2.2) < 0.01;

    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, MS_SAMPLES);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Set context to current window
    glfwMakeContextCurrent(window);

    // Intitialise and verify GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Enable mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    // OGL state setup --------------------------------------------------
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LEQUAL);

    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (MS_SAMPLES > 1) glEnable(GL_MULTISAMPLE);

    if (gamma_correct && !manual_gamma) glEnable(GL_FRAMEBUFFER_SRGB);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Setup geometry, textures, buffers and shaders --------------------
    // Vertices

    // Shaders
    Shader pbrManualShader("../../../src/shaders/vertex_advanced.vert", "../../../src/shaders/pbr_ibl_param.frag");
    Shader pbrTextureShader("../../../src/shaders/vertex_advanced.vert", "../../../src/shaders/pbr_ibl_text.frag");
    Shader lightShader("../../../src/shaders/vertex_advanced.vert", "../../../src/shaders/solid.frag");
    Shader equirectToCubemapShader("../../../src/shaders/cube_project.vert", "../../../src/shaders/equirect.frag");
    Shader diffEnvMapPrefilterShader("../../../src/shaders/cube_project.vert", "../../../src/shaders/envMapPrefilterDiffuse.frag");
    Shader specEnvMapPrefilterShader("../../../src/shaders/cube_project.vert", "../../../src/shaders/envMapPrefilterSpecular.frag");
    Shader brdfPrecomputeShader("../../../src/shaders/screen.vert", "../../../src/shaders/brdf_precompute.frag");
    Shader skyboxShader("../../../src/shaders/cubemap.vert", "../../../src/shaders/cubemap.frag");
    Shader ppfxShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/screen.frag");


    // Load textures
    Texture rustedIronAlbedo("../../../src/textures/rusted_iron/albedo.png", gamma_correct);
    Texture rustedIronNormal("../../../src/textures/rusted_iron/normal.png", gamma_correct);
    Texture rustedIronORM("../../../src/textures/rusted_iron/ORM.png", gamma_correct);

    std::vector < std::string > cubemap_paths = {
        "../../../src/textures/skybox/right.jpg",
        "../../../src/textures/skybox/left.jpg",
        "../../../src/textures/skybox/top.jpg",
        "../../../src/textures/skybox/bottom.jpg",
        "../../../src/textures/skybox/front.jpg",
        "../../../src/textures/skybox/back.jpg"
    };
    Cubemap skybox(cubemap_paths);

    HDR hdr = HDR("../../../src/textures/HDR/newport_loft.hdr");

    Material rusted_metal = Material("../../../src/textures/rusted_iron", gamma_correct);
    Material gold = Material("../../../src/textures/gold", gamma_correct);
    Material grass = Material("../../../src/textures/grass", gamma_correct);
    Material plastic = Material("../../../src/textures/plastic", gamma_correct);
    Material wall = Material("../../../src/textures/wall", gamma_correct);

    // Models & meshes
    Model sphere = Model("../../../src/models/sphere/sphere.obj", gamma_correct);
    Model cube = Model(Cube(glm::vec3(1.0), 1.0, std::vector<TextureData>{}));
    ScreenQuad screen = ScreenQuad();

    // Model transformations
    int nrRows = 7;
    int nrColumns = 7;
    float spacing = 2.5;

    glm::vec3 spherePositions[] = {
        glm::vec3(-5.0f,  0.0f, 0.0f),
        glm::vec3(-2.5f,  0.0f, 0.0f),
        glm::vec3(0.0f,  0.0f, 0.0f),
        glm::vec3(2.5f,  0.0f, 0.0f),
        glm::vec3(5.0f,  0.0f, 0.0f)
    };
    Material sphereMaterials[] = {
        rusted_metal,
        gold,
        grass,
        plastic,
        wall
    };

    // Lights
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0),
        glm::vec3(10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    // Resolutions
    unsigned int envMapRes = 512;
    unsigned int diffEnvMapRes = 32;
    unsigned int specEnvMapRes = 128;
    unsigned int brdfLutRes = 512;

    // Render object setup
    PPO ppo = PPO(ppfxShader, SCR_WIDTH, SCR_HEIGHT);
    FBO captureFBO = FBO();
    captureFBO.bind();
    RBO captureRBO = RBO(envMapRes, envMapRes, GL_DEPTH_COMPONENT24);
    captureRBO.bind();
    captureRBO.attach(GL_DEPTH_ATTACHMENT);
    captureFBO.check_status();
    
    // Cubemap capture camera setup
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // HDR cubemap setup
    Cubemap hdrEnvCubemap = Cubemap(envMapRes, envMapRes, GL_RGB16F, 1, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    captureFBO.bind();
    equirectToCubemapShader.use();
    hdr.activate(equirectToCubemapShader, "equirectangularMap", 0);
    equirectToCubemapShader.setMat4("projection", captureProjection);
    glViewport(0, 0, envMapRes, envMapRes);
    glFrontFace(GL_CW);
    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectToCubemapShader.setMat4("view", captureViews[i]);
        hdrEnvCubemap.attachFace(GL_COLOR_ATTACHMENT0, i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube.Draw(equirectToCubemapShader);
    }
    glFrontFace(GL_CCW);
    captureFBO.unbind();

    hdrEnvCubemap.generateMipMaps();

    // Diffuse envrionment map prefiltering
    Cubemap diffEnvCubemap = Cubemap(diffEnvMapRes, diffEnvMapRes, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR);
    captureFBO.bind();
    captureRBO.edit(diffEnvMapRes, diffEnvMapRes, GL_DEPTH_COMPONENT24);
    diffEnvMapPrefilterShader.use();
    hdrEnvCubemap.activate(diffEnvMapPrefilterShader, "environmentMap", 0);
    diffEnvMapPrefilterShader.setMat4("projection", captureProjection);
    glViewport(0, 0, diffEnvMapRes, diffEnvMapRes);
    glFrontFace(GL_CW);
    for (unsigned int i = 0; i < 6; ++i)
    {
        diffEnvMapPrefilterShader.setMat4("view", captureViews[i]);
        diffEnvCubemap.attachFace(GL_COLOR_ATTACHMENT0, i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube.Draw(diffEnvMapPrefilterShader);
    }
    glFrontFace(GL_CCW);
    captureFBO.unbind();

    // Specular environment map prefiltering
    Cubemap specEnvCubemap = Cubemap(specEnvMapRes, specEnvMapRes, GL_RGB16F, 1, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    specEnvCubemap.generateMipMaps();
    captureFBO.bind();
    specEnvMapPrefilterShader.use();
    hdrEnvCubemap.activate(specEnvMapPrefilterShader, "environmentMap", 0);
    specEnvMapPrefilterShader.setMat4("projection", captureProjection);
    glFrontFace(GL_CW);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        specEnvMapPrefilterShader.setFloat("roughness", (float)mip / (float)(maxMipLevels - 1));
        unsigned int mipWidth = specEnvMapRes * std::pow(0.5, mip);
        unsigned int mipHeight = specEnvMapRes * std::pow(0.5, mip);
        captureRBO.edit(mipWidth, mipHeight, GL_DEPTH_COMPONENT24);
        glViewport(0, 0, mipWidth, mipHeight);
        for (unsigned int i = 0; i < 6; ++i)
        {
            specEnvMapPrefilterShader.setMat4("view", captureViews[i]);
            specEnvCubemap.attachFace(GL_COLOR_ATTACHMENT0, i, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cube.Draw(specEnvMapPrefilterShader);
        }
    }
    glFrontFace(GL_CCW);
    captureFBO.unbind();

    // BRDF integration lookup table 
    Texture brdfLUT = Texture(brdfLutRes, brdfLutRes, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    captureFBO.bind();
    captureRBO.edit(brdfLutRes, brdfLutRes, GL_DEPTH_COMPONENT24);
    brdfLUT.attach(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, brdfLutRes, brdfLutRes);
    brdfPrecomputeShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    screen.Draw();
    captureFBO.unbind();


    // shader setup

    // Background
    float clear_color[] = { pow(0.1, gamma), pow(0.1, gamma), pow(0.1, gamma), 1.0 };


    bool toggle_old = toggle;
    int caseNr = 0;

    // Main render loop ---------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window);

        // Rendering
        ppo.start_render_to_texture();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearBufferfv(GL_COLOR, 0, clear_color);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

        pbrManualShader.use();
        pbrManualShader.setMat4("view", view);
        pbrManualShader.setMat4("projection", projection);
        pbrManualShader.setInt("numLights", sizeof(lightPositions) / sizeof(lightPositions[0]));
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            pbrManualShader.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
            pbrManualShader.setVec3("lights[" + std::to_string(i) + "].color", lightColors[i]);
        }
        pbrManualShader.setVec3("camPos", camera.Position);
        pbrManualShader.setVec3("albedo", 0.3f, 0.0f, 0.0f);
        pbrManualShader.setFloat("occlusion", 1.0f);
        diffEnvCubemap.activate(pbrManualShader, "diffuseEnvMap", 0);
        specEnvCubemap.activate(pbrManualShader, "specularEnvMap", 1);
        brdfLUT.activate(pbrManualShader, "brdfLUT", 2);
        for (int row = 0; row < nrRows; ++row)
        {
            pbrManualShader.setFloat("metalness", (float)row / (float)(nrRows-1));
            //pbrShader.setFloat("metalness", 1.0);
            for (int col = 0; col < nrColumns; ++col)
            {
                // we clamp the roughness to 0.05 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting.
                pbrManualShader.setFloat("roughness", glm::clamp((float)col / (float)(nrColumns-1), 0.1f, 1.0f));

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (col - (nrColumns / 2)) * spacing,
                    (row - (nrRows / 2)) * spacing,
                    -3.0f
                ));
                pbrManualShader.setMat4("model", model);
                pbrManualShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                sphere.Draw(pbrManualShader);
            }
        }

        // 
        pbrTextureShader.use();
        pbrTextureShader.setMat4("view", view);
        pbrTextureShader.setMat4("projection", projection);
        pbrTextureShader.setInt("numLights", sizeof(lightPositions) / sizeof(lightPositions[0]));
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            pbrTextureShader.setVec3("lights[" + std::to_string(i) + "].position", lightPositions[i]);
            pbrTextureShader.setVec3("lights[" + std::to_string(i) + "].color", lightColors[i]);
        }
        pbrTextureShader.setVec3("camPos", camera.Position);
        diffEnvCubemap.activate(pbrTextureShader, "diffuseEnvMap", 0);
        specEnvCubemap.activate(pbrTextureShader, "specularEnvMap", 1);
        brdfLUT.activate(pbrTextureShader, "brdfLUT", 2);
        for (unsigned int i = 0; i < sizeof(spherePositions) / sizeof(spherePositions[0]); ++i) {
            Texture albedo = sphereMaterials[i].albedo;
            Texture normal = sphereMaterials[i].normal;
            Texture orm = sphereMaterials[i].orm;
            albedo.activate(pbrTextureShader, "AlbedoMap", 3);
            normal.activate(pbrTextureShader, "NormalMap", 4);
            orm.activate(pbrTextureShader, "ORMMap", 5);
            model = glm::mat4(1.0f);
            model = glm::translate(model, spherePositions[i]);
            pbrTextureShader.setMat4("model", model);
            pbrTextureShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            sphere.Draw(pbrTextureShader);
        }


        // Lights
        lightShader.use();
        for (int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i) 
        {
            lightShader.setVec3("color", lightColors[i]);
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.5f));
            lightShader.setMat4("model", model);
            lightShader.setMat4("view", view);
            lightShader.setMat4("projection", projection);
            lightShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            sphere.Draw(lightShader);
        }


        // Cubemap background
        glFrontFace(GL_CW);
        skyboxShader.use();
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setMat4("view", view);
        if (toggle_old != toggle)
        {
            caseNr = (caseNr + 1) % 4;
            toggle_old = toggle;
            std::string message = "case = " + std::to_string((float)caseNr);
            std::cout << message << std::endl;
        }
        skyboxShader.setFloat("mipLevel", (float)caseNr);
        specEnvCubemap.activate(skyboxShader, "cubemap", 0);
        cube.Draw(skyboxShader);
        glFrontFace(GL_CCW);

        // post processing (gamma and exposure correction)
        ppfxShader.use();
        ppfxShader.setFloat("gamma", gamma);
        ppfxShader.setFloat("exposure", exposure);
        ppfxShader.setFloat("bloom", bloom);
        ppo.draw_texture_to_screen();

        // debug texture
        //screenShader.use();
        //brdfLUT.activate(screenShader, "screenTexture", 0);
        //screen.Draw();

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    };
    // Terminate
    glfwTerminate();
    return 0;
}

int main(void)
{
    switch (22)
    {
    case 0:  return base_scene(); break;
    case 1:  return main_scene(); break;
    case 2:  return simple_scene(); break;
    case 3:  return geom_shader_scene(); break;
    case 4:  return norm_vect_scene(); break;
    case 5:  return instancing_scene(); break;
    case 6:  return asteroid_scene(); break;
    case 7:  return msaa_scene(); break;
    case 8:  return blinn_phong_scene(); break;
    case 9:  return gamma_scene(); break;
    case 10: return quad_cube_test_scene(); break;
    case 11: return shadow_scene(); break;
    case 12: return point_shadow_scene(); break;
    case 13: return normal_map_scene(); break;
    case 14: return parallax_map_scene(); break;
    case 15: return hdr_scene(); break;
    case 16: return bloom_scene(); break;
    case 17: return deferred_scene(); break;
    case 18: return ssao_scene(); break;
    case 19: return kernel_test_scene(); break;
    case 20: return pbr_scene(); break;
    case 21: return pbr_texture_scene(); break;
    case 22: return ibl_scene(); break;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
        zoom = true;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
        zoom = false;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !post_process_key) {
        post_process = !post_process;
        post_process_key = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        post_process_key = false;
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !normal_mapping_key) {
        normal_mapping = !normal_mapping;
        normal_mapping_key = true;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
        normal_mapping_key = false;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bloom_key) {
        bloom = !bloom;
        bloom_key = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
        bloom_key = false;
    }

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !ssao_key) {
        ssaoFlag = !ssaoFlag;
        ssao_key = true;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
        ssao_key = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !toggle_key) {
        toggle = !toggle;
        toggle_key = true;
        std::cout << toggle << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        toggle_key = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (heightScale > 0.0f) {
            heightScale -= 0.0005f;
            std::cout << heightScale << std::endl;
        }
      
        else
            heightScale = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (heightScale < 1.0f) {
            heightScale += 0.0005f;
            std::cout << heightScale << std::endl;
        }
        else
            heightScale = 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        if (exposure > 0.01f) {
            exposure -= 0.005f;
            std::cout << exposure << std::endl;
        }
        else
            exposure = 0.01f;
    }
    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        if (exposure < 100.0f) {
            exposure += 0.005f;
            std::cout << exposure << std::endl;
        }
        else
            exposure = 100.0f;
    }
}

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

float lerp(float a, float b, float f) {
    return a + (b - a) * f;
}