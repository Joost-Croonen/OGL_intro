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
	void blit(unsigned int width, unsigned int height, unsigned int target = 0)	const {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
	void set_draw_read_buffer(bool flag) const {
		if (!flag) {
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
		else {

			glDrawBuffer(GL_FRONT);
			glReadBuffer(GL_FRONT);
		}

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
