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
    void setAttributes(unsigned int positions = 3, unsigned int normals = 3, unsigned int texcoords = 2, unsigned int vertcolor = 0) {
        unsigned int stride = positions + normals + texcoords + vertcolor;
        unsigned int attributeCount = 0;
        glVertexAttribPointer(attributeCount, positions, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
        glEnableVertexAttribArray(attributeCount++);
        if (normals > 0) {
            glVertexAttribPointer(attributeCount, normals, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(positions * sizeof(float)));
            glEnableVertexAttribArray(attributeCount++);
        }
        if (texcoords > 0) {
            glVertexAttribPointer(attributeCount, texcoords, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)((positions + normals) * sizeof(float)));
            glEnableVertexAttribArray(attributeCount++);
        } 
        if (vertcolor > 0) {
            glVertexAttribPointer(attributeCount, vertcolor, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)((positions + normals + texcoords) * sizeof(float)));
            glEnableVertexAttribArray(attributeCount);
        }
    }
    void Delete() { glDeleteVertexArrays(1, &id); }
};

#endif // !VAO_H

