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
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f
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
    screenVAO.setAttributes(false);
    screenVAO.unbind();

    // Shaders
    Shader ourShader("../../../src/shaders/vertex.vert", "../../../src/shaders/fragment.frag");
    Shader outlineShader("../../../src/shaders/outline.vert", "../../../src/shaders/outline.frag");
    Shader simpleShader("../../../src/shaders/simple.vert", "../../../src/shaders/simple.frag");
    Shader screenShader("../../../src/shaders/screen.vert", "../../../src/shaders/texture.frag");

    // Model
    Model ourModel("../../../src/models/backpack/backpack.obj");

    // Load other textures
    Texture floorTexture = Texture("../../../src/textures/marble.jpg",
        GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Texture grassTexture = Texture("../../../src/textures/grass.png",
        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Texture windowTexture = Texture("../../../src/textures/window.png",
        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // Render to texture
    FBO fbo = FBO();
    fbo.bind();
    Texture bufferTexture = Texture(SCR_WIDTH, SCR_HEIGHT, GL_RGB);
    bufferTexture.attach(GL_COLOR_ATTACHMENT0);
    RBO rbo = RBO(SCR_WIDTH, SCR_HEIGHT, GL_DEPTH24_STENCIL8);
    rbo.bind();
    rbo.attach(GL_DEPTH_STENCIL_ATTACHMENT);
    //rbo.unbind();
    fbo.check_status();
    fbo.unbind();

    // Enable depht test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable stencil testing
    // glEnable(GL_STENCIL_TEST);
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

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

        // First pass to texture
        fbo.bind();
        glEnable(GL_DEPTH_TEST);

        // Rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, NEAR_PLANE, FAR_PLANE);
        //ourShader.setMat4("projection", projection);
        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        //ourShader.setMat4("view", view);
        // model matrix
        glm::mat4 model = glm::mat4(1.0f);

        // activate shader and set uniforms
        ourShader.use();
        // viewpos
        ourShader.setVec3("viewPos", camera.Position);
        // material
        ourShader.setFloat("material.shininess", 32.0f);

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::vec3 pointLightPositions[] = {
            glm::vec3(0.7f,  0.2f,  2.0f),
            glm::vec3(2.3f, -3.3f, -4.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
            glm::vec3(0.0f,  0.0f, -3.0f)
        };
        // Lights
        {
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
        }
        

        // 1st pass backpack
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);

        //glStencilFunc(GL_ALWAYS, 1, 0xFF);
        //glStencilMask(0xFF);
        //glStencilMask(0x00);
        ourModel.Draw(ourShader);

        // floor
        model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);
        //glStencilMask(0x00);
        planeVAO.bind();
        floorTexture.activate(ourShader, "material.texture_diffuse1", 0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        planeVAO.unbind();

        // Grass
        simpleShader.use();
        //glStencilMask(0x00);
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

        // 2nd pass to draw render texture to screen quad
        fbo.unbind();
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        screenVAO.bind();
        bufferTexture.activate(screenShader, "screenTexture", 0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window); 
        glfwPollEvents();
    }

    planeVAO.Delete();
    quadVAO.Delete();
    //screenVAO.Delete();
    planeVBO.Delete();
    quadVBO.Delete();
    //screenVBO.Delete();
    planeEBO.Delete();
    quadEBO.Delete();
    //screenEBO.Delete();
    rbo.Delete();
    fbo.Delete();
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