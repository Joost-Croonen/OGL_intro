#ifndef GBO_H
#define GBO_H

class GBO {
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
    FBO fbo;
    RBO rbo;
    Texture albedoSpecGBuffer;
    Texture normalGbuffer;
    Texture positionGbuffer;
    //Texture depthTex;
    Shader gbufferShader;
    Shader deferredShader;
    GBO(int width, int height, Shader gBufferShader, Shader deferredShader) :
        gbufferShader(gBufferShader),
        deferredShader(deferredShader),
        width(width),
        height(height),
        vao(VAO()),
        vbo(VBO(screenVertices, sizeof(screenVertices))),
        ebo(EBO(screenIndices, sizeof(screenIndices))),
        rbo(RBO(width, height, GL_DEPTH24_STENCIL8, 1)),
        albedoSpecGBuffer(Texture(width, height, GL_RGBA, 1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)),
        normalGbuffer(Texture(width, height, GL_RGBA16F, 1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)),
        positionGbuffer(Texture(width, height, GL_RGBA16F, 1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE))
        //depthTex(Texture(width, height, GL_DEPTH_COMPONENT, 1, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE))
    {
        vao.bind();
        vao.linkVBO(vbo);
        vao.linkEBO(ebo);
        vao.setAttributes(2, 0, 2);
        vao.unbind();

        fbo.bind();
        //depthTex.attach(GL_DEPTH_ATTACHMENT);
        positionGbuffer.attach(GL_COLOR_ATTACHMENT0);
        normalGbuffer.attach(GL_COLOR_ATTACHMENT1);
        albedoSpecGBuffer.attach(GL_COLOR_ATTACHMENT2); 
        GLenum attachments[3] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        fbo.multiDrawBuffers(3, attachments);
        rbo.attach(GL_DEPTH_STENCIL_ATTACHMENT);
        fbo.check_status();
        fbo.unbind();
    }

    void geometry_pass() {
        fbo.bind();
    }

    void lighting_pass(unsigned int id) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        //glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glDisable(GL_DEPTH_TEST);     // Why was this here in the first place? 
        //glDepthMask(GL_FALSE);
        deferredShader.use();
        positionGbuffer.activate(deferredShader, "gPosition", 0);
        normalGbuffer.activate(deferredShader, "gNormal", 1);
        albedoSpecGBuffer.activate(deferredShader, "gAlbedoSpec", 2);
        vao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        vao.unbind();
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Delete() {
        vao.Delete();
        vbo.Delete();
        ebo.Delete();
        fbo.Delete();
        rbo.Delete();
        albedoSpecGBuffer.Delete();
        normalGbuffer.Delete();
        positionGbuffer.Delete();
    }
};

#endif
