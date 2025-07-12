#ifndef EBO_H
#define EBO_H

class EBO
{
public:
	unsigned int id;
	EBO(unsigned int indices[], GLsizeiptr size) {
		glGenBuffers(1, &id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
	}
	inline void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id); }
	inline void unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
	void Delete() { glDeleteBuffers(1, &id); }
};

#endif // !EBO_H
