#ifndef MESH_H
#define MESH_H


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct TextureData {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh
{
public:
    unsigned int VAOid, VBOid, EBOid;
    std::vector<Vertex>         vertices;
    std::vector<unsigned int>   indices;
    std::vector<TextureData>    textures;

    Mesh(){};

    Mesh(std::vector<Vertex> verts, std::vector<unsigned int> indcs, std::vector<TextureData> texts) :
        vertices(verts),
        indices(indcs),
        textures(texts) 
    {
        setup_mesh();
    }

    void Draw(Shader& shader) {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;

        for (int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number;
            std::string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            if (name == "texture_specular")
                number = std::to_string(specularNr++);
            if (name == "texture_normal")
                number = std::to_string(normalNr++);
            shader.setInt(("material." + name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        glActiveTexture(GL_TEXTURE0);   // unbind

        // Draw mesh
        glBindVertexArray(VAOid);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);           // unbind
    }
    void Delete() const {
        glDeleteVertexArrays(1, &VAOid);
        glDeleteBuffers(1, &VBOid);
        glDeleteBuffers(1, &EBOid);
    }
protected:
    void setup_mesh() {
        // Create buffers
        glGenVertexArrays(1, &VAOid);
        glGenBuffers(1, &VBOid);
        glGenBuffers(1, &EBOid);

        // Bind buffers
        glBindVertexArray(VAOid);
        glBindBuffer(GL_ARRAY_BUFFER, VBOid);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOid);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Set attributes
        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);
        // texcoords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(2);
        // tangent
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        glEnableVertexAttribArray(3);
        // bitangent
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        glEnableVertexAttribArray(4);
        
        // unbind
        glBindVertexArray(0);
    }
};

class Quad : public Mesh
{
public:
    Quad(glm::vec2 size, float texScale, std::vector<TextureData> textureDataList) {
        // Setup vertices
        float sizex = size.x / 2.0;
        float sizey = size.y / 2.0;
        float quadVertices[] = {
        //  verts                   normals           texcoords               tangent         bitangent
            -sizex, -sizey, 0.0,    0.0, 0.0, 1.0,    0.0,        0.0,        1.0, 0.0, 0.0,  0.0, 1.0, 0.0,  //0
             sizex, -sizey, 0.0,    0.0, 0.0, 1.0,    texScale,   0.0,        1.0, 0.0, 0.0,  0.0, 1.0, 0.0,  //1
             sizex,  sizey, 0.0,    0.0, 0.0, 1.0,    texScale,   texScale,   1.0, 0.0, 0.0,  0.0, 1.0, 0.0,  //2
            -sizex,  sizey, 0.0,    0.0, 0.0, 1.0,    0.0,        texScale,   1.0, 0.0, 0.0,  0.0, 1.0, 0.0,  //3
        };

        for (int i = 0; i < 4; i++) {
            Vertex vertex;
            int offset = i * 14;
            vertex.Position = glm::vec3(quadVertices[offset + 0], quadVertices[offset + 1], quadVertices[offset + 2]);
            offset += 3;
            vertex.Normal = glm::vec3(quadVertices[offset + 0], quadVertices[offset + 1], quadVertices[offset + 2]);
            offset += 3;
            vertex.TexCoords = glm::vec2(quadVertices[offset + 0], quadVertices[offset + 1]); 
            offset += 2;
            vertex.Tangent = glm::vec3(quadVertices[offset + 0], quadVertices[offset + 1], quadVertices[offset + 2]);
            offset += 3;
            vertex.Bitangent = glm::vec3(quadVertices[offset + 0], quadVertices[offset + 1], quadVertices[offset + 2]);
            vertices.push_back(vertex);
        }

        /*
        glm::vec3 edge1 = vertices[1].Position - vertices[0].Position;
        glm::vec3 edge2 = vertices[2].Position - vertices[0].Position;
        glm::vec2 deltaUV1 = vertices[1].TexCoords - vertices[0].TexCoords;
        glm::vec2 deltaUV2 = vertices[2].TexCoords - vertices[0].TexCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        float x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        float y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        float z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        std::cout << x << std::endl;
        std::cout << y << std::endl;
        std::cout << z << std::endl;

        x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        std::cout << x << std::endl;
        std::cout << y << std::endl;
        std::cout << z << std::endl;

        edge1 = vertices[2].Position - vertices[0].Position;
        edge2 = vertices[3].Position - vertices[0].Position;
        deltaUV1 = vertices[2].TexCoords - vertices[0].TexCoords;
        deltaUV2 = vertices[3].TexCoords - vertices[0].TexCoords;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        std::cout << x << std::endl;
        std::cout << y << std::endl;
        std::cout << z << std::endl;

        x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        std::cout << x << std::endl;
        std::cout << y << std::endl;
        std::cout << z << std::endl;

        */

        // Setup indices
        unsigned int quadIndices[] = {
            //0, 1, 2,
            //0, 2, 3
            3, 0, 1,
            3, 1, 2
        };
        for (int i = 0; i < 6; i++) {
            indices.push_back(quadIndices[i]);
        }

        // Setup textures
        textures = textureDataList;

        // Setup buffers
        setup_mesh();
    }
};

class Cube : public Mesh {
public:
    Cube(glm::vec3 size, float texScale, Texture texture) {
        // Setup vertices
        float sizex = size.x / 2.0;
        float sizey = size.y / 2.0;
        float sizez = size.z / 2.0;
        float cubeVertices[] = {
            // verts                     normals              texcoords
            // Front
            -sizex, -sizey,  sizez,      0.0,  0.0,  1.0,     0       ,  0       ,   //0   
             sizex, -sizey,  sizez,      0.0,  0.0,  1.0,     texScale,  0       ,   //1
             sizex,  sizey,  sizez,      0.0,  0.0,  1.0,     texScale,  texScale,   //2
            -sizex,  sizey,  sizez,      0.0,  0.0,  1.0,     0       ,  texScale,   //3
             // Back
            -sizex, -sizey, -sizez,      0.0,  0.0, -1.0,     0       ,  0       ,   //4
             sizex, -sizey, -sizez,      0.0,  0.0, -1.0,     texScale,  0       ,   //5
             sizex,  sizey, -sizez,      0.0,  0.0, -1.0,     texScale,  texScale,   //6
            -sizex,  sizey, -sizez,      0.0,  0.0, -1.0,     0       ,  texScale,   //7
             // Top
            -sizex,  sizey,  sizez,      0.0,  1.0,  0.0,     0       ,  0       ,   //8
             sizex,  sizey,  sizez,      0.0,  1.0,  0.0,     texScale,  0       ,   //9
             sizex,  sizey, -sizez,      0.0,  1.0,  0.0,     texScale,  texScale,   //10
            -sizex,  sizey, -sizez,      0.0,  1.0,  0.0,     0       ,  texScale,   //11
             // Bottom                                
            -sizex, -sizey,  sizez,      0.0, -1.0,  0.0,     0       ,  0       ,   //12
             sizex, -sizey,  sizez,      0.0, -1.0,  0.0,     texScale,  0       ,   //13
             sizex, -sizey, -sizez,      0.0, -1.0,  0.0,     texScale,  texScale,   //14
            -sizex, -sizey, -sizez,      0.0, -1.0,  0.0,     0       ,  texScale,   //15
             // Left                                  
            -sizex, -sizey,  sizez,     -1.0,  0.0,  0.0,     0       ,  0       ,   //16
            -sizex,  sizey,  sizez,     -1.0,  0.0,  0.0,     texScale,  0       ,   //17
            -sizex,  sizey, -sizez,     -1.0,  0.0,  0.0,     texScale,  texScale,   //18
            -sizex, -sizey, -sizez,     -1.0,  0.0,  0.0,     0       ,  texScale,   //19
             // Right                                 
             sizex, -sizey,  sizez,      1.0,  0.0,  0.0,     0       ,  0       ,   //20
             sizex,  sizey,  sizez,      1.0,  0.0,  0.0,     texScale,  0       ,   //21
             sizex,  sizey, -sizez,      1.0,  0.0,  0.0,     texScale,  texScale,   //22
             sizex, -sizey, -sizez,      1.0,  0.0,  0.0,     0       ,  texScale    //23

        };
        for (int i = 0; i < 24; i++) {
            Vertex vertex;
            int offset = i * 8;
            vertex.Position = glm::vec3(cubeVertices[offset + 0], cubeVertices[offset + 1], cubeVertices[offset + 2]);
            offset += 3;
            vertex.Normal = glm::vec3(cubeVertices[offset + 0], cubeVertices[offset + 1], cubeVertices[offset + 2]);
            offset += 3;
            vertex.TexCoords = glm::vec2(cubeVertices[offset + 0], cubeVertices[offset + 1]);
            vertices.push_back(vertex);
        }

        // Setup indices
        unsigned int cubeIndices[] = {
            // Front
            0, 1, 2,
            0, 2, 3,
            // Back
            4, 6, 5,
            4, 7, 6,
            // Top
            8, 9, 10,
            8, 10, 11,
            // Bottom
            12, 14, 13,
            12, 15, 14,
            // Left
            16, 17, 18,
            16, 18, 19,
            // Right
            20, 22, 21,
            20, 23, 22
        };
        for (int i = 0; i < 36; i++) {
            indices.push_back(cubeIndices[i]);
        }

        // Setup textures
        TextureData texdata;
        texdata.id = texture.id;
        texdata.type = "texture_diffuse";
        texdata.path = "";
        textures.push_back(texdata);

        // Setup buffers
        setup_mesh();
    }
};

class ScreenQuad {
public:
    unsigned int VAOid, VBOid, EBOid;
    ScreenQuad() {
        // Vertices
        float screenVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        // Indices
        unsigned int screenIndices[] = {
                0, 1, 2,
                0, 2, 3
        };

        // Create buffers
        glGenVertexArrays(1, &VAOid);
        glGenBuffers(1, &VBOid);
        glGenBuffers(1, &EBOid);

        // Bind buffers
        glBindVertexArray(VAOid);
        glBindBuffer(GL_ARRAY_BUFFER, VBOid);
        glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), screenVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOid);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenIndices), screenIndices, GL_STATIC_DRAW);

        // Set attributes
        // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // texcoords
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // unbind
        glBindVertexArray(0);
    }

    void Draw() {
        glBindVertexArray(VAOid);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Delete() const {
        glDeleteVertexArrays(1, &VAOid);
        glDeleteBuffers(1, &VBOid);
        glDeleteBuffers(1, &EBOid);
    }
};

#endif