#ifndef FBO_H
#define FBO_H

class FBO
{
public:
	unsigned int id;
	FBO() { 
		glGenFramebuffers(1, &id); 
	}
	void bind() const{ 
		glBindFramebuffer(GL_FRAMEBUFFER, id); 
	}
	void unbind() const{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void check_status() const {
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
	}
	void Delete() const{
		glDeleteFramebuffers(1, &id);
	}
};

#endif // !FBO_H
