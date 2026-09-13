#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	unsigned int ID;		// Program ID

	//Shader(const char* vertexPath, const char* fragmentPath)
	//{
	//	// 1. Retrieve shader source code
	//	// initialise
	//	std::string vertexCode;
	//	std::string fragmentCode;
	//	std::ifstream vShaderFile;
	//	std::ifstream fShaderFile;
	//	// exception enabling
	//	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	//	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	//	try
	//	{
	//		// open files
	//		vShaderFile.open(vertexPath);
	//		fShaderFile.open(fragmentPath);
	//		// read files into stream
	//		std::stringstream vShaderStream, fShaderStream;
	//		vShaderStream << vShaderFile.rdbuf();
	//		fShaderStream << fShaderFile.rdbuf();
	//		// close files
	//		vShaderFile.close();
	//		fShaderFile.close();
	//		// convert stream into into string
	//		vertexCode = vShaderStream.str();
	//		fragmentCode = fShaderStream.str();
	//	}
	//	catch(std::ifstream::failure e)
	//	{
	//		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	//		std::cout << "  vertex: " << vertexPath << std::endl;
	//		std::cout << "  fragment: " << fragmentPath << std::endl;
	//	}
	//	// convert to const char
	//	const char* vShaderCode = vertexCode.c_str();
	//	const char* fShaderCode = fragmentCode.c_str();
	//
	//	// 2. Compile shaders
	//	// init
	//	unsigned int vertexShader, fragmentShader;
	//	// compile and verify vertex shader
	//	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//	glShaderSource(vertexShader, 1, &vShaderCode, NULL);
	//	glCompileShader(vertexShader);
	//	checkCompileErrors(vertexShader, "VERTEX");
	//	// compile and verify fragment shader
	//	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//	glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
	//	glCompileShader(fragmentShader);
	//	checkCompileErrors(fragmentShader, "FRAGMENT");
	//	// Link and verify shader program
	//	ID = glCreateProgram();
	//	glAttachShader(ID, vertexShader);
	//	glAttachShader(ID, fragmentShader);
	//	glLinkProgram(ID);
	//	checkCompileErrors(ID, "PROGRAM");
	//	// clean up
	//	glDeleteShader(vertexShader);
	//	glDeleteShader(fragmentShader);
	//
	//};

	//Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath)
	//{
	//	// 1. Retrieve shader source code
	//	// initialise
	//	std::string vertexCode;
	//	std::string geometryCode;
	//	std::string fragmentCode;
	//	std::ifstream vShaderFile;
	//	std::ifstream gShaderFile;
	//	std::ifstream fShaderFile;
	//	// exception enabling
	//	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	//	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	//	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	//	try
	//	{
	//		// open files
	//		vShaderFile.open(vertexPath);
	//		gShaderFile.open(geometryPath);
	//		fShaderFile.open(fragmentPath);
	//		// read files into stream
	//		std::stringstream vShaderStream, gShaderStream, fShaderStream;
	//		vShaderStream << vShaderFile.rdbuf();
	//		gShaderStream << gShaderFile.rdbuf();
	//		fShaderStream << fShaderFile.rdbuf();
	//		// close files
	//		vShaderFile.close();
	//		gShaderFile.close();
	//		fShaderFile.close();
	//		// convert stream into into string
	//		vertexCode = vShaderStream.str();
	//		geometryCode = gShaderStream.str();
	//		fragmentCode = fShaderStream.str();
	//	}
	//	catch (std::ifstream::failure e)
	//	{
	//		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	//		std::cout << "  vertex: " << vertexPath << std::endl;
	//		std::cout << "  geometry: " << geometryPath << std::endl;
	//		std::cout << "  fragment: " << fragmentPath << std::endl;
	//	}
	//	// convert to const char
	//	const char* vShaderCode = vertexCode.c_str();
	//	const char* gShaderCode = geometryCode.c_str();
	//	const char* fShaderCode = fragmentCode.c_str();
	//
	//	// 2. Compile shaders
	//	// init
	//	unsigned int vertexShader, geometryShader, fragmentShader;
	//	// compile and verify vertex shader
	//	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//	glShaderSource(vertexShader, 1, &vShaderCode, NULL);
	//	glCompileShader(vertexShader);
	//	checkCompileErrors(vertexShader, "VERTEX");
	//	// compile and verify geometry shader
	//	geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
	//	glShaderSource(geometryShader, 1, &gShaderCode, NULL);
	//	glCompileShader(geometryShader);
	//	checkCompileErrors(geometryShader, "GEOMETRY");
	//	// compile and verify fragment shader
	//	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//	glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
	//	glCompileShader(fragmentShader);
	//	checkCompileErrors(fragmentShader, "FRAGMENT");
	//	// Link and verify shader program
	//	ID = glCreateProgram();
	//	glAttachShader(ID, vertexShader);
	//	glAttachShader(ID, geometryShader);
	//	glAttachShader(ID, fragmentShader);
	//	glLinkProgram(ID);
	//	checkCompileErrors(ID, "PROGRAM");
	//	// clean up
	//	glDeleteShader(vertexShader); 
	//	glDeleteShader(geometryShader);
	//	glDeleteShader(fragmentShader);
	//
	//};

	Shader(const char* vertexPath, 
		const char* fragmentPath, 
		const char* geometryPath = nullptr,
		const char* tessellationCtrlPath = nullptr, 
		const char* tessellationEvalPath = nullptr)
	{
		// 1. Retrieve shader source code
		// initialise
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		std::string tessCtrlCode;
		std::string tessEvalCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;
		std::ifstream tcShaderFile;
		std::ifstream teShaderFile;
		// exception enabling
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		tcShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		teShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			// read files into stream
			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close files
			vShaderFile.close();
			fShaderFile.close();
			// convert stream into into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			// read geometry shader if provided
			if (geometryPath != nullptr)
			{
				gShaderFile.open(geometryPath);
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryCode = gShaderStream.str();
			}
			// read tessellation control shader if provided
			if (tessellationCtrlPath != nullptr)
			{
				tcShaderFile.open(tessellationCtrlPath);
				std::stringstream tcShaderStream;
				tcShaderStream << tcShaderFile.rdbuf();
				tcShaderFile.close();
				tessCtrlCode = tcShaderStream.str();
			}
			// read tessellation evaluation shader if provided
			if (tessellationEvalPath != nullptr)
			{
				teShaderFile.open(tessellationEvalPath);
				std::stringstream teShaderStream;
				teShaderStream << teShaderFile.rdbuf();
				teShaderFile.close();
				tessEvalCode = teShaderStream.str();
			}
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			std::cout << "  vertex: " << vertexPath << std::endl;
			std::cout << "  fragment: " << fragmentPath << std::endl;
			if (geometryPath!=nullptr)
				std::cout << "  geometry: " << geometryPath << std::endl;
			if (tessellationCtrlPath != nullptr)
				std::cout << "  tess_ctrl: " << tessellationCtrlPath << std::endl;
			if (tessellationEvalPath != nullptr)
				std::cout << "  tess_eval: " << tessellationEvalPath << std::endl;
		}
		// convert to const char
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		// 2. Compile shaders
		// init
		unsigned int vertexShader, fragmentShader, geometryShader, tessCtrlShader, tessEvalShader;
		// compile and verify vertex shader
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vShaderCode, NULL);
		glCompileShader(vertexShader);
		checkCompileErrors(vertexShader, "VERTEX");
		// compile and verify fragment shader
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
		glCompileShader(fragmentShader);
		checkCompileErrors(fragmentShader, "FRAGMENT");
		// compile and verify geometry shader if provided
		if (geometryPath != nullptr)
		{
			const char* gShaderCode = geometryCode.c_str();
			geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometryShader, 1, &gShaderCode, NULL);
			glCompileShader(geometryShader);
			checkCompileErrors(geometryShader, "GEOMETRY");
		}
		// compile and verify tessellation control shader if provided
		if (tessellationCtrlPath != nullptr)
		{
			const char* tcShaderCode = tessCtrlCode.c_str();
			tessCtrlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
			glShaderSource(tessCtrlShader, 1, &tcShaderCode, NULL);
			glCompileShader(tessCtrlShader);
			checkCompileErrors(tessCtrlShader, "TESS_CTRL");
		}
		// compile and verify tessellation evaluation shader if provided
		if (tessellationEvalPath != nullptr)
		{
			const char* teShaderCode = tessEvalCode.c_str();
			tessEvalShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
			glShaderSource(tessEvalShader, 1, &teShaderCode, NULL);
			glCompileShader(tessEvalShader);
			checkCompileErrors(tessEvalShader, "TESS_EVAL");
		}
		// Link and verify shader program
		ID = glCreateProgram();
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		if (geometryPath != nullptr) {
			glAttachShader(ID, geometryShader);
		}
		if (tessellationCtrlPath != nullptr)
		{
			glAttachShader(ID, tessCtrlShader);
		}
		if (tessellationEvalPath != nullptr) 
		{
			glAttachShader(ID, tessEvalShader);
		}
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");
		// clean up
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		if (geometryPath != nullptr)
			glDeleteShader(geometryShader);
		if (tessellationCtrlPath != nullptr)
			glDeleteShader(tessCtrlShader);
		if (tessellationEvalPath != nullptr)
			glDeleteShader(tessEvalShader);

	};

	void use() const
	{
		glUseProgram(ID);
	};

	void setUniformBuffer(const std::string& name, unsigned int binding) const
	{
		glUniformBlockBinding(ID, glGetUniformBlockIndex(ID, name.c_str()), 0);
	}

	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string& name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};

#endif