#ifndef VAO_H
#define VAO_H

unsigned int vao(float vertices[], bool normals = true, unsigned int indices = 0) {
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    /*
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    if (indices != 0) {
        unsigned int EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);
    }
    unsigned int stride = 5;
    unsigned int vertexCoordStart = 3;
    unsigned int vertexCoordAttr = 1;
    if (normals) {
        stride = 8;
        vertexCoordStart = 6;
        vertexCoordAttr = 2;
    }
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    if (normals) {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    glVertexAttribPointer(vertexCoordAttr, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(vertexCoordStart * sizeof(float)));
    glEnableVertexAttribArray(vertexCoordAttr);
    glBindVertexArray(0);
    */
    return VAO;
}

class VAO {
public:
    unsigned int id;
    VAO() {
        glGenVertexArrays(1, &id);
    }
    void bind() { glBindVertexArray(id); }
    void unbind() { glBindVertexArray(0); }
    void linkVBO(VBO vbo) { vbo.bind();}
    void linkEBO(EBO ebo) { ebo.bind(); }
    void setAttributes(bool normals = true) {
        unsigned int stride = 5;
        unsigned int vertexCoordStart = 3;
        unsigned int vertexCoordAttr = 1;
        if (normals) {
            stride = 8;
            vertexCoordStart = 6;
            vertexCoordAttr = 2;
        }
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        if (normals) {
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
        }
        glVertexAttribPointer(vertexCoordAttr, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(vertexCoordStart * sizeof(float)));
        glEnableVertexAttribArray(vertexCoordAttr);
    }
    void Delete() { glDeleteVertexArrays(1, &id); }
};

#endif // !VAO_H

