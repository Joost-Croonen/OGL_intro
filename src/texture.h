#ifndef TEXTURE_H 
#define TEXTURE_H

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

class Texture
{
public:
    unsigned int id;
    GLint wrap_s;
    GLint wrap_t;
    GLint min_filt;
    GLint mag_filt;
    GLenum data_format;

    Texture(const char* path, GLenum dataformat, GLint wraps, GLint wrapt, GLint minfilt, GLint magfilt) : albedoPath(path), data_format(dataformat), wrap_s(wraps), wrap_t(wrapt), min_filt(minfilt), mag_filt(magfilt)
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, data_format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load image texture" << std::endl;
        }
        // Memory cleanup
        stbi_image_free(data);
	}

    void activate(GLenum texture_unit)
    {
        glActiveTexture(texture_unit);
        glBindTexture(GL_TEXTURE_2D, id);
    }

private:
    const char* albedoPath;
    int width, height, nrChannels;
    unsigned char* data;
};

#endif
