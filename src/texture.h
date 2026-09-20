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
            internalFormat = gamma_correct ? GL_SRGB_ALPHA : GL_RGBA;
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

    Texture(const char* path,
        bool gamma_correct = false, bool flipY = true, bool inverted = false,
        GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT,
        GLint min_filt = GL_LINEAR_MIPMAP_LINEAR, GLint mag_filt = GL_LINEAR) :
        albedoPath(path), samples(1)
    {
        // Set image orientation
        stbi_set_flip_vertically_on_load(flipY);
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
        if (inverted) {
            for (int i = 0; i < width * height * nrChannels; ++i)
                data[i] = 255 - data[i];
        }
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

    Texture(unsigned int width, unsigned int height, GLenum internalFormat, std::vector<glm::vec3> *data,
        GLint min_filt = GL_LINEAR, GLint mag_filt = GL_LINEAR, 
        GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT, float borderColor[4] = DEFAULT_BORDER_COLOR) :
        albedoPath(""), width(width), height(height), samples(1), nrChannels(1)
    {
        GLenum dataFormat;
        GLenum pixelType;
        if (internalFormat == GL_DEPTH_COMPONENT || internalFormat == GL_DEPTH_COMPONENT32F)
        {
            dataFormat = GL_DEPTH_COMPONENT;
            nrChannels = 1;
            pixelType = GL_FLOAT;
        }
        if (internalFormat == GL_RED)
        {
            dataFormat = internalFormat;
            nrChannels = 1;
            pixelType = GL_FLOAT;
        }
        if (internalFormat == GL_R32F)
        {
            dataFormat = GL_RED;
            nrChannels = 1;
            pixelType = GL_FLOAT;
        }
        if (internalFormat == GL_RGB){
            dataFormat = GL_RGB;
            nrChannels = 3;
            pixelType = GL_UNSIGNED_BYTE;
        }
        if (internalFormat == GL_RGBA) {
            dataFormat = GL_RGBA;
            nrChannels = 4;
            pixelType = GL_UNSIGNED_BYTE;
        }
        if (internalFormat == GL_RGB16F) {
            dataFormat = GL_RGB;
            nrChannels = 3;
            pixelType = GL_FLOAT;
        }
        if (internalFormat == GL_RGBA16F) {
            dataFormat = GL_RGBA;
            nrChannels = 4;
            pixelType = GL_FLOAT;
        }
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width, this->height, 0, dataFormat, pixelType, data->data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filt);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filt);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture(unsigned int width, unsigned int height, GLenum internalFormat, unsigned int samples = 1,
        GLint min_filt = GL_LINEAR, GLint mag_filt = GL_LINEAR,
        GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT, float borderColor[4] = DEFAULT_BORDER_COLOR) :
        albedoPath(""), width(width), height(height), samples(samples), nrChannels(1)
    {
        GLenum dataFormat;
        GLenum pixelType;
        if (internalFormat == GL_DEPTH_COMPONENT || internalFormat == GL_DEPTH_COMPONENT32F)
        {
            dataFormat = GL_DEPTH_COMPONENT;
            nrChannels = 1;
            pixelType = GL_FLOAT;
        }
        if (internalFormat == GL_RED)
        {
            dataFormat = internalFormat;
            nrChannels = 1;
            pixelType = GL_FLOAT;
        }
        if (internalFormat == GL_R32F)
        {
            dataFormat = GL_RED;
            nrChannels = 1;
            pixelType = GL_FLOAT;
        }
        if (internalFormat == GL_RGB) {
            dataFormat = GL_RGB;
            nrChannels = 3;
            pixelType = GL_UNSIGNED_BYTE;
        }
        if (internalFormat == GL_RGBA) {
            dataFormat = GL_RGBA;
            nrChannels = 4;
            pixelType = GL_UNSIGNED_BYTE;
        }
        if (internalFormat == GL_RGB16F) {
            dataFormat = GL_RGB;
            nrChannels = 3;
            pixelType = GL_FLOAT;
        }
        if (internalFormat == GL_RGBA16F) {
            dataFormat = GL_RGBA;
            nrChannels = 4;
            pixelType = GL_FLOAT;
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

    void attach(GLenum attachement, unsigned int mipLevel = 0) const
    {
        if (samples==1)
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachement, GL_TEXTURE_2D, id, mipLevel);
        else
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachement, GL_TEXTURE_2D_MULTISAMPLE, id, mipLevel);
    }

    void generateMipMaps() const {
        glBindTexture(GL_TEXTURE_2D, id);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void restrictMipLevels(unsigned int texture_unit, int minLod, int maxLod) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, minLod);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, maxLod);
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
        GLenum format;
        GLenum dataType;
        if (internalFormat == GL_DEPTH_COMPONENT)
        {
            format = GL_DEPTH_COMPONENT;
            nrChannels = 1;
            dataType = GL_FLOAT;
        }
        else if (internalFormat == GL_RGB16F)
        {
            format = GL_RGB;
            nrChannels = 3;
            dataType = GL_FLOAT;
        }
        else {
            format = GL_RGB;
            nrChannels = 3;
            dataType = GL_UNSIGNED_BYTE;
        }
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        for (unsigned int i=0; i<6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, this->width, this->height, 0, format, dataType, NULL);
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

    void attachAll(GLenum attachement) const
    {
        glFramebufferTexture(GL_FRAMEBUFFER, attachement, id, 0);
    }

    void attachFace(GLenum attachement, unsigned int face, unsigned int mipLevel=0) const
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachement, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, id, mipLevel);
    }

    void generateMipMaps() const
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

private:
    int width, height, nrChannels;
    std::vector<std::string> paths;
};



class HDR
{
public:
    unsigned int id;

    HDR() {}

    HDR(const char* path) :path(path)
    {
        // Set image orientation
        stbi_set_flip_vertically_on_load(true);
        // Generate and bind
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        // Define texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Load image
        float* data = stbi_loadf(path, &width, &height, &nrChannels, 0);
        // Create texture and generate mipmaps for currently bound texture
        if (data) {
            GLenum format;
            GLenum internalFormat;
            GLenum dataType = GL_FLOAT;
            if (nrChannels == 1) {
                format = GL_RED;
                internalFormat = GL_R16F;
            }
            else if (nrChannels == 3) {
                format = GL_RGB;
                internalFormat = GL_RGB16F;
            }
            else if (nrChannels == 4) {
                format = GL_RGBA;
                internalFormat = GL_RGBA16F;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load image texture" << std::endl;
        }
        // Memory cleanup
        stbi_image_free(data);
        glActiveTexture(GL_TEXTURE0);
    }

    void activate(Shader shader, const char* name, unsigned int texture_unit) const
    {
        shader.use();
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        shader.setInt(name, texture_unit);
        glBindTexture(GL_TEXTURE_2D, id);
    }

private:
    int width, height, nrChannels;
    std::string path;
};


class Material
{
private:
    std::string albedoPath;
    std::string normalPath;
    std::string ormPath;
public:
    Texture albedo;
    Texture normal;
    Texture orm;
    Material(const char* path, bool gamma_correct) :
        albedoPath(std::string(path) + "/albedo.png"),
        normalPath(std::string(path) + "/normal.png"),
        ormPath(std::string(path) + "/ORM.png"),
        albedo(Texture(albedoPath.c_str(), gamma_correct)),
        normal(Texture(normalPath.c_str(), false)),
        orm(Texture(ormPath.c_str(), false))
    {
        //std::cout << path << std::endl;
        //std::cout << albedoPath << std::endl;
        //std::cout << normalPath << std::endl;
        //std::cout << ormPath << std::endl;
    }
};




class ValueNoiseTexture : public Texture
{
public:
    ValueNoiseTexture(unsigned int width, unsigned int height, int scale) :
        Texture(width, height, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, DEFAULT_BORDER_COLOR)
    {
        int num_cells = scale;
        std::vector<glm::vec3> noiseData(width * height);
        std::vector<float> rand_grid = value_noise_grid(num_cells);
        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                float nx = (float)x / (float)width;
                float ny = (float)y / (float)height;
                float noise = value_noise(nx, ny, num_cells, rand_grid);
                noiseData[y * width + x] = glm::vec3(noise);
            }
        }
        glBindTexture(GL_TEXTURE_2D, id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, noiseData.data());
    }

    ValueNoiseTexture(unsigned int width, unsigned int height, std::vector<int> octaves, std::vector<float> powers) :
        Texture(width, height, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, DEFAULT_BORDER_COLOR)
    {
        std::vector<glm::vec3> noiseData(width * height, glm::vec3(0.0));
        for (int i = 0; i < octaves.size(); ++i) {
            int num_cells = octaves[i];
			float power = powers[i];
            std::vector<float> rand_grid = value_noise_grid(num_cells);
            for (unsigned int y = 0; y < height; ++y) {
                for (unsigned int x = 0; x < width; ++x) {
                    float nx = (float)x / (float)width;
                    float ny = (float)y / (float)height;
					float noise = value_noise(nx, ny, num_cells, rand_grid);
                    noiseData[y * width + x] += glm::vec3(power * noise);
                }
            }
        }
        glBindTexture(GL_TEXTURE_2D, id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, noiseData.data());
    }

    float value_noise(float x, float y, int num_cells, const std::vector<float>& rand_vectors)
    {
        int ix0 = static_cast<int>(x * num_cells) % num_cells;
        int iy0 = static_cast<int>(y * num_cells) % num_cells;
        int ix1 = (ix0 + 1) % num_cells;
        int iy1 = (iy0 + 1) % num_cells;
        float dx = x * num_cells - static_cast<float>(ix0);
        float dy = y * num_cells - static_cast<float>(iy0);
        float s00 = rand_vectors[iy0 * num_cells + ix0];  
		float s10 = rand_vectors[iy0 * num_cells + ix1];
		float s01 = rand_vectors[iy1 * num_cells + ix0];
		float s11 = rand_vectors[iy1 * num_cells + ix1];
        return smootherlerp(smootherlerp(s00, s10, dx), smootherlerp(s01, s11, dx), dy);
    }

    std::vector<float> value_noise_grid(int num_points)
    {
        std::vector<float> rand_vectors(num_points * num_points);
        for (int i = 0; i < num_points * num_points; ++i) {
            rand_vectors[i] = random();
        }
		return rand_vectors;
    }
};



class PerlinNoiseTexture : public Texture
{
public:
    PerlinNoiseTexture(unsigned int width, unsigned int height, int scale) :
        Texture(width, height, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, DEFAULT_BORDER_COLOR)
    {
        int num_cells = scale;
        std::vector<glm::vec3> noiseData(width * height);
        std::vector<glm::vec2> rand_grid = perlin_noise_grid(num_cells);
        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                float nx = (float)x / (float)width;
                float ny = (float)y / (float)height;
                float noise = perlin_noise(nx, ny, num_cells, rand_grid);
                noiseData[y * width + x] = glm::vec3(noise);
            }
        }
        for (auto& v : noiseData) {
            v *= 0.5f;
            v += 0.5f;
        }
        glBindTexture(GL_TEXTURE_2D, id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, noiseData.data());
    }

    PerlinNoiseTexture(unsigned int width, unsigned int height, std::vector<int> octaves, std::vector<float> powers) :
        Texture(width, height, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, DEFAULT_BORDER_COLOR)
    {
        std::vector<glm::vec3> noiseData(width * height, glm::vec3(0.0));
        for (int i = 0; i < octaves.size(); ++i) {
            int num_cells = octaves[i];
            float power = powers[i];
            std::vector<glm::vec2> rand_grid = perlin_noise_grid(num_cells);
            for (unsigned int y = 0; y < height; ++y) {
                for (unsigned int x = 0; x < width; ++x) {
                    float nx = (float)x / (float)width;
                    float ny = (float)y / (float)height;
                    float noise = perlin_noise(nx, ny, num_cells, rand_grid);
                    noiseData[y * width + x] += glm::vec3(power * noise);
                }
            }
        }
        for (auto& v : noiseData) {
			v *= 0.5f;
			v += 0.5f;
        }
        glBindTexture(GL_TEXTURE_2D, id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, noiseData.data());
    }

    float perlin_noise(float x, float y, int num_cells, const std::vector<glm::vec2>& rand_vectors)
    {
        int ix0 = static_cast<int>(x * num_cells) % num_cells;
        int iy0 = static_cast<int>(y * num_cells) % num_cells;
		int ix1 = (ix0 + 1) % num_cells;
		int iy1 = (iy0 + 1) % num_cells;
        float dx = x * num_cells - static_cast<float>(ix0);
        float dy = y * num_cells - static_cast<float>(iy0);
        glm::vec2 d00 = -glm::vec2(dx, dy);
        glm::vec2 d10 = -glm::vec2(dx - 1.0f, dy);
        glm::vec2 d01 = -glm::vec2(dx, dy - 1.0f);
        glm::vec2 d11 = -glm::vec2(dx - 1.0f, dy - 1.0f);
        float s00 = glm::dot(rand_vectors[iy0 * num_cells + ix0], d00);
        float s10 = glm::dot(rand_vectors[iy0 * num_cells + ix1], d10);
        float s01 = glm::dot(rand_vectors[iy1 * num_cells + ix0], d01);
        float s11 = glm::dot(rand_vectors[iy1 * num_cells + ix1], d11);
        return smootherlerp(smootherlerp(s00, s10, dx), smootherlerp(s01, s11, dx), dy);
    }

    std::vector<glm::vec2> perlin_noise_grid(int num_points)
    {
        std::vector<glm::vec2> rand_vectors(num_points * num_points);
        for (int i = 0; i < num_points * num_points; ++i) {
            rand_vectors[i] = rand_direction();
        }
        return rand_vectors;
    }

    glm::vec2 rand_direction() {
        float angle = random() * 2.0f * 3.14159265358979323846f;
        return glm::vec2(cos(angle), sin(angle));
    }
};





#endif
