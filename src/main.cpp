#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "shader.h"
#include "camera.h"
#include "texture.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1440;

// camera
Camera camera(glm::vec3(1.0f, 1.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -100, -20);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main(void)
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

    // Shaders
    Shader cubeShader("../../../src/shaders/vertex.vert", "../../../src/shaders/fragment.frag");
    Shader lightShader("../../../src/shaders/vertex.vert", "../../../src/shaders/light.frag");

    // Vertex data
    /*
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };
    */
    /*
    float vertices[] = {
        // positions          // colors           // texture coords
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f
    };
    */
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    //// Vertex indices
    //unsigned int indices[] = {
    //    0, 1, 3, // first triangle
    //    1, 2, 3  // second triangle
    //};

    // Create vertex buffer object
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    // Create element buffer object
    //unsigned int EBO;
    //glGenBuffers(1, &EBO);
    // Create vertex attribute object
    unsigned int cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    // Bind VAO
    glBindVertexArray(cubeVAO);
    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Set vertex color attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Set vertex normal attribute pointers
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    // Set texture coordinate attribute pointers
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Bind EBO
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // Set vertex position attribute pointers
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // // Set vertex color attribute pointers
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    // glEnableVertexAttribArray(1);
    // // Set texture coordinate attribute pointers
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // glEnableVertexAttribArray(2);

    // Same for lights
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    
    // Textures
    Texture texture0("../../../src/textures/container2.png", GL_RGBA, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Texture texture1("../../../src/textures/container2_specular.png", GL_RGBA, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Enable depht test
    glEnable(GL_DEPTH_TEST);

    // Enable wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

        // Core
        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        
        // Activate textures
        texture0.activate(GL_TEXTURE0);
        texture1.activate(GL_TEXTURE1);

        // Lights
        glm::vec3 pointLightPositions[] = {
                glm::vec3(0.7f,  0.2f,  2.0f),
                glm::vec3(2.3f, -3.3f, -4.0f),
                glm::vec3(-4.0f,  2.0f, -12.0f),
                glm::vec3(0.0f,  0.0f, -3.0f)
        };

        // activate shader and set uniforms
        cubeShader.use();
        // viewpos
        cubeShader.setVec3("viewPos", camera.Position);
        // material
        cubeShader.setInt("material.diffuse", 0);
        cubeShader.setInt("material.specular", 1);
        cubeShader.setFloat("material.shininess", 32.0f);
        // dir light
        cubeShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        cubeShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        cubeShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point lights
        cubeShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        cubeShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("pointLights[0].constant", 1.0f);
        cubeShader.setFloat("pointLights[0].linear", 0.09f);
        cubeShader.setFloat("pointLights[0].quadratic", 0.032f);
        // point light 2
        cubeShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        cubeShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("pointLights[1].constant", 1.0f);
        cubeShader.setFloat("pointLights[1].linear", 0.09f);
        cubeShader.setFloat("pointLights[1].quadratic", 0.032f);
        // point light 3
        cubeShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        cubeShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("pointLights[2].constant", 1.0f);
        cubeShader.setFloat("pointLights[2].linear", 0.09f);
        cubeShader.setFloat("pointLights[2].quadratic", 0.032f);
        // point light 4
        cubeShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        cubeShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("pointLights[3].constant", 1.0f);
        cubeShader.setFloat("pointLights[3].linear", 0.09f);
        cubeShader.setFloat("pointLights[3].quadratic", 0.032f);

        // spot light
        cubeShader.setVec3("spotLight.ambient", 0.1f, 0.1f, 0.1f);
        cubeShader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("spotLight.constant", 1.0f);
        cubeShader.setFloat("spotLight.linear", 0.09f);
        cubeShader.setFloat("spotLight.quadratic", 0.032f);
        cubeShader.setVec3("spotLight.position", camera.Position);
        cubeShader.setVec3("spotLight.direction", camera.Front);
        cubeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        cubeShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

        

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        cubeShader.setMat4("projection", projection);
        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        cubeShader.setMat4("view", view);
        // model matrix
        glm::mat4 model = glm::mat4(1.0f);
        cubeShader.setMat4("model", model);

        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            cubeShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        
        
        // render boxes
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        
        for (unsigned int i = 0; i < 4; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lightShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window); 
        glfwPollEvents();
    }
    // Dealocate resources 
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    // Terminate GLFW
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
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