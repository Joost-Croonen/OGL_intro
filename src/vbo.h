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
	inline void bind() const { glBindBuffer(GL_ARRAY_BUFFER, id); }
	inline void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
	void Delete() { glDeleteBuffers(1, &id); }
};

#endif // !VBO_H

