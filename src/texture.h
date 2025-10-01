#ifndef TEXTURE_H 
#define TEXTURE_H

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

float DEFAULT_BORDER_COLOR[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

unsigned int TextureFromFile(const char* path, const std::string &dir, bool gamma_correct = false) {
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
        GLenum internalFormat;
        if (nrChannels == 1) {
            format = GL_RED;
            internalFormat = GL_RED;
        }
        else if (nrChannels == 3) {
            format = GL_RGB;
            internalFormat = gamma_correct ? GL_SRGB : GL_RGB;
        }
        else if (nrChannels == 4) {
            format = GL_RGBA;
            internalFormat = gamma_correct ? GL_SRGB_ALPHA: GL_RGBA;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
    // Cleanup
    // glActiveTexture(GL_TEXTURE0);
    stbi_image_free(data);
    return id;
}

class Texture
{
public:
    unsigned int id;

    //Texture() {};

    Texture(const char* path, bool gamma_correct = false, 
        GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT,
        GLint min_filt = GL_LINEAR_MIPMAP_LINEAR, GLint mag_filt = GL_LINEAR) :
        albedoPath(path), samples(1)
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
        unsigned char* data = stbi_load(albedoPath, &width, &height, &nrChannels, 0);
        // Create texture and generate mipmaps for currently bound texture
        if (data) {
            GLenum format;
            GLenum internalFormat;
            if (nrChannels == 1) {
                format = GL_RED;
                internalFormat = GL_RED;
            }
            else if (nrChannels == 3) {
                format = GL_RGB;
                internalFormat = GL_RGB;
                internalFormat = gamma_correct ? GL_SRGB : GL_RGB;
            }
            else if (nrChannels == 4) {
                format = GL_RGBA;
                internalFormat = gamma_correct ? GL_SRGB_ALPHA : GL_RGBA;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load image texture" << std::endl;
        }
        // Memory cleanup
        stbi_image_free(data);
        glActiveTexture(GL_TEXTURE0);
	}

    Texture(unsigned int width, unsigned int height, GLenum internalFormat, unsigned int samples = 1,
        GLint min_filt=GL_LINEAR, GLint mag_filt=GL_LINEAR, 
        GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT, float borderColor[4] = DEFAULT_BORDER_COLOR) :
        albedoPath(""), width(width), height(height), samples(samples), nrChannels(1)
    {
        GLenum dataFormat;
        GLenum pixelType;
        if (internalFormat == GL_DEPTH_COMPONENT)
        {
            dataFormat = internalFormat;
            nrChannels = 1;
            pixelType = GL_FLOAT;
        }
        else {
            dataFormat = GL_RGB;
            nrChannels = 3;
            pixelType = GL_UNSIGNED_BYTE;
        }
        glGenTextures(1, &id);
        if (samples == 1)
        {
            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width, this->height, 0, dataFormat, pixelType, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filt);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filt);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else 
        {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, id);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, this->width, this->height, GL_TRUE);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        }
        
    }

    void activate(Shader shader, const char* name, unsigned int texture_unit) const
    {
        shader.use();
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        shader.setInt(name, texture_unit); 
        glBindTexture(GL_TEXTURE_2D, id);
    }

    void attach(GLenum attachement) const
    {
        if (samples==1)
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachement, GL_TEXTURE_2D, id, 0);
        else
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachement, GL_TEXTURE_2D_MULTISAMPLE, id, 0);
    }

    std::string get_path() { return std::string(albedoPath); }

    void Delete() { glDeleteTextures(1, &id); }

private:
    int width, height, nrChannels, samples;
    const char* albedoPath;
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
            unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
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

    Cubemap(unsigned int width, unsigned int height, GLenum internalFormat, unsigned int samples = 1,
        GLint min_filt = GL_NEAREST, GLint mag_filt = GL_NEAREST,
        GLint wrap_r = GL_CLAMP_TO_EDGE, GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE,
        float borderColor[4] = DEFAULT_BORDER_COLOR) :
        paths(std::vector<std::string>{}), width(width), height(height), nrChannels(1)
    {
        GLenum dataFormat;
        GLenum pixelType;
        if (internalFormat == GL_DEPTH_COMPONENT)
        {
            dataFormat = GL_DEPTH_COMPONENT;
            nrChannels = 1;
            pixelType = GL_FLOAT;
        }
        else {
            dataFormat = GL_RGB;
            nrChannels = 3;
            pixelType = GL_UNSIGNED_BYTE;
        }
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        for (unsigned int i=0; i<6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, this->width, this->height, 0, dataFormat, pixelType, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filt);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag_filt);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    void activate(Shader shader, const char* name, unsigned int texture_unit) const
    {
        shader.use();
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        shader.setInt(name, texture_unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    }

    void attach(GLenum attachement) const
    {
        glFramebufferTexture(GL_FRAMEBUFFER, attachement, id, 0);
    }

private:
    int width, height, nrChannels;
    std::vector<std::string> paths;
};

#endif
