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
	void blit(unsigned int width, unsigned int height, unsigned int target = 0, GLenum type = GL_COLOR_BUFFER_BIT)	const {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, type, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void set_draw_buffer(GLenum draw) const {
		glDrawBuffer(draw);
	}
	void set_read_buffer(GLenum read) const {
		glReadBuffer(read);
	}
	void multiDrawBuffers(int num, GLenum* attachments) {
		glDrawBuffers(num, attachments);
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
