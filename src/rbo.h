#ifndef RBO_H
#define RBO_H

class RBO
{
public:
	unsigned int id;
	RBO(unsigned int width, unsigned int height, GLenum type) {
		glGenRenderbuffers(1, &id);
		glBindRenderbuffer(GL_RENDERBUFFER, id);
		glRenderbufferStorage(GL_RENDERBUFFER, type, width, height);
	}
	void bind() const {
		glBindRenderbuffer(GL_RENDERBUFFER, id);
	}
	void unbind() const {
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	void attach(GLenum type) const {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, type, GL_RENDERBUFFER, id);
	}
	void Delete() const {
		glDeleteRenderbuffers(1, &id);
	}
};

#endif // !RBO_H
