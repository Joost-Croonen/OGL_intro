#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <map>

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1440;
const float NEAR_PLANE = 0.1;
const float FAR_PLANE = 100;
bool zoom = false;

// camera
Camera camera(glm::vec3(1.0f, 1.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -100, -20);
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
    glEnable(GL_FRAMEBUFFER_SRGB);

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
    Model ourModel("../../../src/models/backpack/backpack.obj");

    // Load other textures
    Texture floorTexture = Texture("../../../src/textures/wood.png");

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
        unsigned int VAO = rock.meshes[i].VAO;
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
            glBindVertexArray(rock.meshes[i].VAO);
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
        screenBufferTexture.activate(screenShader, "screenTexture", GL_TEXTURE0);
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
    glEnable(GL_MULTISAMPLE);

    // Gamma correction
    glEnable(GL_FRAMEBUFFER_SRGB);

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
    ourShader.setVec3("lightPos", 0.0, 0.0, 0.0);

    // Model
    //Model ourModel("../../../src/models/backpack/backpack.obj");

    // Load other textures
    Texture floorTexture = Texture("../../../src/textures/wood.png");

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

        floorTexture.activate(ourShader, "texture_diffuse1", 0);

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
    bool gamma = false;
    glEnable(GL_FRAMEBUFFER_SRGB);
    gamma = true;

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
    Texture multiSampleBufferTexture = Texture(SCR_WIDTH, SCR_HEIGHT, GL_RGB, 4);
    multiSampleBufferTexture.attach(GL_COLOR_ATTACHMENT0);
    RBO rbo = RBO(SCR_WIDTH, SCR_HEIGHT, GL_DEPTH24_STENCIL8, 4);
    rbo.attach(GL_DEPTH_STENCIL_ATTACHMENT);
    multisampleFrameBuffer.check_status();
    multisampleFrameBuffer.unbind();
    // post process
    FBO postProcessFrameBuffer = FBO();
    postProcessFrameBuffer.bind();
    Texture screenBufferTexture = Texture(SCR_WIDTH, SCR_HEIGHT, GL_RGB);
    screenBufferTexture.attach(GL_COLOR_ATTACHMENT0);
    postProcessFrameBuffer.check_status();
    postProcessFrameBuffer.unbind();

    // Shaders
    Shader ourShader("../../../src/shaders/vertex.vert", "../../../src/shaders/blinn_phong.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/ppfx.frag");

    ourShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);
    ourShader.setFloat("material.shininess", 64.0);

    // Model
    Model ourModel("../../../src/models/backpack/backpack.obj", gamma);

    // Load other textures
    Texture floorTexture = Texture("../../../src/textures/wood.png", gamma);

    // Lights
    ourShader.use();
    ourShader.setVec3 ("light.position", glm::vec3(0.0, 1.5, 1.0));
    ourShader.setFloat("light.constant", 1.0);
    ourShader.setFloat("light.linear", 0.09);
    ourShader.setFloat("light.quadratic", 0.032);
    if (gamma) {

        ourShader.setFloat("light.constant", 0.0);
        ourShader.setFloat("light.linear", 0.0);
        ourShader.setFloat("light.quadratic", 1.0);
    }

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
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Draw
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Floor
        ourShader.use();
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        ourShader.setVec3("viewPos", camera.Position);
        floorTexture.activate(ourShader, "material.texture_diffuse1", GL_TEXTURE0);
        floorTexture.activate(ourShader, "material.texture_specular1", GL_TEXTURE1);
        planeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Backpack
        ourShader.use();
        model = glm::translate(model, glm::vec3(0.0, 0.5, 0.0));
        model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        ourShader.setVec3("viewPos", camera.Position);
        ourModel.Draw(ourShader);

        // 2. Texture to screen
        multisampleFrameBuffer.blit(SCR_WIDTH, SCR_HEIGHT, postProcessFrameBuffer.id);
        multisampleFrameBuffer.unbind();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        screenShader.use();
        screenBufferTexture.activate(screenShader, "screenTexture", GL_TEXTURE0);
        screenVAO.bind();
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

int main(void)
{
    switch (9)
    {
    case 0: return base_scene(); break;
    case 1: return main_scene(); break;
    case 2: return simple_scene(); break;
    case 3: return geom_shader_scene(); break;
    case 4: return norm_vect_scene(); break;
    case 5: return instancing_scene(); break;
    case 6: return asteroid_scene(); break;
    case 7: return msaa_scene(); break;
    case 8: return blinn_phong_scene(); break;
    case 9: return gamma_scene(); break;
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