#ifndef VAO_H
#define VAO_H

class VAO {
public:
    unsigned int id;
    VAO() {
        glGenVertexArrays(1, &id);
    }
    void bind() const { glBindVertexArray(id); }
    void unbind() const { glBindVertexArray(0); }
    void linkVBO(VBO vbo) const { vbo.bind();}
    void linkEBO(EBO ebo) const { ebo.bind(); }
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

