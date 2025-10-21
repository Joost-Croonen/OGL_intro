#ifndef PPO_H
#define PPO_H

class PPO {
public:
    float screenVertices[16] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    unsigned int screenIndices[6] = {
                0, 1, 2,
                0, 2, 3
    };
    int width;
    int height;
    bool multisampling;
    VAO vao;
    VBO vbo;
    EBO ebo;
    FBO renderBuffer;
    FBO screenBuffer;
    FBO pingBuffer;
    FBO pongBuffer;
    RBO rbo;
    Texture renderTexture;
    Texture screenTexture;
    Texture brightTexture;
    Texture pingTexture;
    Texture pongTexture;
    Shader screenShader;
    Shader bloomShader;
    PPO(Shader shader, int width, int height, int num_samples=1):
        screenShader(shader),
        bloomShader(Shader("../../../src/shaders/screen.vert", "../../../src/shaders/bloom.frag")),
        width(width),
        height(height),
        multisampling(num_samples>1),
        vao(VAO()),
        vbo(VBO(screenVertices, sizeof(screenVertices))),
        ebo(EBO(screenIndices, sizeof(screenIndices))),
        screenBuffer(FBO()),
        renderBuffer(FBO()),
        pingBuffer(FBO()),
        pongBuffer(FBO()),
        rbo(RBO(width, height, GL_DEPTH24_STENCIL8, num_samples)),
        renderTexture(Texture(width, height, GL_RGB16F, num_samples, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)),
        screenTexture(Texture(width, height, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)),
        brightTexture(Texture(width, height, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)),
        pingTexture(Texture(width, height, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)),
        pongTexture(Texture(width, height, GL_RGB16F, 1, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE))
    {
        vao.bind();
        vao.linkVBO(vbo);
        vao.linkEBO(ebo);
        vao.setAttributes(2, 0, 2);
        vao.unbind();

        renderBuffer.bind();
        renderTexture.attach(GL_COLOR_ATTACHMENT0);
        brightTexture.attach(GL_COLOR_ATTACHMENT1);
        GLenum attachments[2] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 };
        renderBuffer.multiDrawBuffers(2, attachments);
        rbo.attach(GL_DEPTH_STENCIL_ATTACHMENT);
        renderBuffer.check_status();
        renderBuffer.unbind();
        
        screenBuffer.bind();
        screenTexture.attach(GL_COLOR_ATTACHMENT0);
        screenBuffer.check_status();
        screenBuffer.unbind();

        pingBuffer.bind();
        pingTexture.attach(GL_COLOR_ATTACHMENT0);
        pingBuffer.check_status();
        pingBuffer.unbind();

        pongBuffer.bind();
        pongTexture.attach(GL_COLOR_ATTACHMENT0);
        pongBuffer.check_status();
        pongBuffer.unbind();
    }

    void start_render_to_texture() {
        renderBuffer.bind();
        static const float black[] = { 0.0, 0.0, 0.0, 1.0 };
        glClearBufferfv(GL_COLOR, 1, black); // ensure bloom buffer has black clearcolor
        // glEnable(GL_DEPTH_TEST);     // Why was this here in the first place? 
    }

    void draw_texture_to_screen() {
        renderBuffer.bind();
        renderBuffer.blit(width, height, screenBuffer.id);
        renderBuffer.unbind();
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glDisable(GL_DEPTH_TEST);     // Why was this here in the first place? 

        screenShader.use();
        screenTexture.activate(screenShader, "screenTexture", 0);
        pingTexture.activate(screenShader, "bloomTexture", 1);
        vao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        vao.unbind();
    }

    void bloom() {
        renderBuffer.bind();
        renderBuffer.set_read_buffer(GL_COLOR_ATTACHMENT1);
        renderBuffer.blit(width, height, pingBuffer.id);
        renderBuffer.set_read_buffer(GL_COLOR_ATTACHMENT0);
        unsigned int amount = 5;
        bloomShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            pongBuffer.bind();
            bloomShader.setInt("horizontal", true);
            pingTexture.activate(bloomShader, "image", 0);
            vao.bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            pingBuffer.bind();
            bloomShader.setInt("horizontal", false);
            pongTexture.activate(bloomShader, "image", 0);
            vao.bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Delete() {
        vao.Delete();
        vbo.Delete();
        ebo.Delete();
        renderBuffer.Delete();
        screenBuffer.Delete();
        rbo.Delete();
        renderTexture.Delete();
        screenTexture.Delete();
    }
};

#endif
