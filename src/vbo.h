#ifndef VBO_H
#define VBO_H

class VBO
{
public:
	unsigned int id;
	VBO(float vertices[], GLsizeiptr size) {
		glGenBuffers(1, &id);
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}
	VBO(std::vector<glm::vec3> vertices) {		//position only buffer
		glGenBuffers(1, &id);
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	}
	VBO(std::vector<float> vertices) {		//position only buffer
		glGenBuffers(1, &id);
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	}
	inline void bind() const { glBindBuffer(GL_ARRAY_BUFFER, id); }
	inline void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
	void Delete() { glDeleteBuffers(1, &id); }
};

#endif // !VBO_H

