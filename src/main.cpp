#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include "stb_image.h"
#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main(void)
{
    // Initialse GLFW
    glfwInit();

    // Setup GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create and verify window 
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);

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
    glViewport(0, 0, 800, 600);

    // Handle resizing of viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Shaders
    Shader ourShader("../../../src/shaders/vertex.vert", "../../../src/shaders/fragment.frag");

    // Vertex data
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };

    // Vertex indices
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    // Create vertex buffer object
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    // Create element buffer object
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    // Create vertex attribute object
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // Bind VAO
    glBindVertexArray(VAO);
    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Bind EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // Set vertex position attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Set vertex color attribute pointers
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    // Set texture coordinate attribute pointers
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Unbind VBO
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Unbind VAO
    // glBindVertexArray(0);
    
    // Textures
    // Set image orientation
    stbi_set_flip_vertically_on_load(true);
    // Generate and bind
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Define texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load image
    int width, height, nrChannels;
    unsigned char* data = stbi_load("../../../src/textures/container.jpg", &width, &height, &nrChannels, 0);
    // Create texture and generate mipmaps for currently bound texture
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load image texture" << std::endl;
    }
    // Memory cleanup
    stbi_image_free(data);
    // Texture 2
    unsigned int texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // Define texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load image
    unsigned char* data2 = stbi_load("../../../src/textures/awesomeface.png", &width, &height, &nrChannels, 0);
    // Create texture and generate mipmaps for currently bound texture
    if (data2) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load image texture" << std::endl;
    }
    // Memory cleanup
    stbi_image_free(data2);
    

    // Enable wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main render loop
    while (!glfwWindowShouldClose(window)) 
    {
        // Inputs
        processInput(window);

        // Rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Core
        // Activate textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        ourShader.use();
        ourShader.setInt("ourTexture1", 0);
        ourShader.setInt("ourTexture2", 1);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap buffers and poll for IO events
        glfwSwapBuffers(window); 
        glfwPollEvents();
    }
    // Dealocate resources 
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
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
}