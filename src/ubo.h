#ifndef UBO_H
#define UBO_H

class UBO
{
public:
	unsigned int id;
	unsigned int size;

	UBO(unsigned int size) : size(size) {
		glGenBuffers(1, &id);
		glBindBuffer(GL_UNIFORM_BUFFER, id);
		glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	void bind(unsigned int binding) const { 
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, id); 
	}
	void bindRange(unsigned int binding, unsigned int offset, unsigned int size) const {
		glBindBufferRange(GL_UNIFORM_BUFFER, binding, id, offset, size);
	}
	void write(const void* data) {
		glBindBuffer(GL_UNIFORM_BUFFER, id);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	void writeRange(unsigned int offset, unsigned int size, const void* data) {
		glBindBuffer(GL_UNIFORM_BUFFER, id);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	inline void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
	void Delete() { glDeleteBuffers(1, &id); }
};

#endif // !UBO_H
