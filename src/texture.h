#ifndef TEXTURE_H 
#define TEXTURE_H

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

unsigned int TextureFromFile(const char* path, const std::string &dir) {
    unsigned int id;
    int width, height, nrChannels;
    // Set image orientation
    stbi_set_flip_vertically_on_load(true);
    // Generate and bind
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    // Load image
    std::string filename = std::string(path);
    filename = dir + '/' + filename;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    // Create texture and generate mipmaps for currently bound texture
    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Define texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
    }
    // Memory cleanup
    // glActiveTexture(GL_TEXTURE0);
    stbi_image_free(data);
    return id;
}

class Texture
{
public:
    unsigned int id;

    Texture() {};

    Texture(const char* path, 
        GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT,
        GLint min_filt = GL_LINEAR_MIPMAP_LINEAR, GLint mag_filt = GL_LINEAR) :
        albedoPath(path)
	{
        // Set image orientation
        stbi_set_flip_vertically_on_load(true);
        // Generate and bind
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        // Define texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filt);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filt);
        // Load image
        data = stbi_load(albedoPath, &width, &height, &nrChannels, 0);
        // Create texture and generate mipmaps for currently bound texture
        if (data) {
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load image texture" << std::endl;
        }
        // Memory cleanup
        stbi_image_free(data);
        glActiveTexture(GL_TEXTURE0);
	}

    Texture(unsigned int width, unsigned int height, GLenum format, 
        GLint min_filt=GL_LINEAR, GLint mag_filt=GL_LINEAR) :
        albedoPath(""), width(width), height(height)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, this->width, this->height, 0, format, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filt);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filt);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void activate(Shader shader, const char* name, GLenum texture_unit) const
    {
        glActiveTexture(texture_unit);
        glBindTexture(GL_TEXTURE_2D, id);
        shader.setInt(name, texture_unit);
    }

    void attach(GLenum type) const
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, id, 0);
    }

private:
    int width, height, nrChannels;
    const char* albedoPath;
    unsigned char* data;
};

class Cubemap
{
public:
    unsigned int id;

    Cubemap() {}

    Cubemap(std::vector<std::string> paths): paths(paths) {
        // Set image orientation
        stbi_set_flip_vertically_on_load(false);
        // Generate and bind
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        // Define texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load data
        for (int i = 0; i < 6; i++) {
            // Load image
            data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
            // Create texture and generate mipmaps for currently bound texture
            if (data) {
                GLenum format;
                if (nrChannels == 1)
                    format = GL_RED;
                else if (nrChannels == 3)
                    format = GL_RGB;
                else if (nrChannels == 4)
                    format = GL_RGBA;
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            }
            else {
                std::cout << "Failed to load image texture" << std::endl;
            }
            stbi_image_free(data);
        }
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    void activate(Shader shader, const char* name, GLenum texture_unit) const
    {
        glActiveTexture(texture_unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        shader.setInt(name, texture_unit);
    }

private:
    int width, height, nrChannels;
    std::vector<std::string> paths;
    unsigned char* data;
};

#endif
