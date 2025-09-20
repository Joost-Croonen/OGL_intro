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
    FBO msfbo;
    FBO fbo;
    RBO rbo;
    Texture msTexture;
    Texture screenTexture;
    Shader screenShader;
    PPO(Shader shader, int width, int height, int num_samples=1):
        screenShader(shader),
        width(width),
        height(height),
        multisampling(num_samples>1),
        vao(VAO()),
        vbo(VBO(screenVertices, sizeof(screenVertices))),
        ebo(EBO(screenIndices, sizeof(screenIndices))),
        fbo(FBO()),
        msfbo(FBO()),
        rbo(RBO(width, height, GL_DEPTH24_STENCIL8, num_samples)),
        msTexture(      Texture(width, height, GL_RGB16, num_samples)),
        screenTexture(  Texture(width, height, GL_RGB16, 1))
    {
        vao.bind();
        vao.linkVBO(vbo);
        vao.linkEBO(ebo);
        vao.setAttributes(2, 0, 2);
        vao.unbind();

        if (multisampling) {
            msfbo.bind();
            msTexture.attach(GL_COLOR_ATTACHMENT0);
            rbo.attach(GL_DEPTH_STENCIL_ATTACHMENT);
            msfbo.check_status();
            msfbo.unbind();
        }
        fbo.bind();
        screenTexture.attach(GL_COLOR_ATTACHMENT0);
        if (!multisampling) rbo.attach(GL_DEPTH_STENCIL_ATTACHMENT);
        fbo.check_status();
        fbo.unbind();
    }

    void start_render_to_texture() {
        if (multisampling) msfbo.bind();
        else fbo.bind();
        glEnable(GL_DEPTH_TEST);
    }

    void draw_texture_to_screen() {
        if (multisampling) {
            msfbo.blit(width, height, fbo.id);
            msfbo.unbind();
        }
        fbo.unbind();
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glDisable(GL_DEPTH_TEST);     // Why was this here in the first place? 

        screenShader.use();
        screenTexture.activate(screenShader, "screenTexture", 0);
        vao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        vao.unbind();
    }

    void Delete() {
        vao.Delete();
        vbo.Delete();
        ebo.Delete();
        msfbo.Delete();
        fbo.Delete();
        rbo.Delete();
        msTexture.Delete();
        screenTexture.Delete();
    }
};

#endif
