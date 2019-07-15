#include "Shader.h"

bool Shader::init(std::string vertexPath, std::string fragmentPath, ShaderDescriptor descriptor)
{
	m_descriptor = descriptor;
	//Loading to std::string, then to const char
	std::string vertexSrc = loadFile(vertexPath);
	if (vertexSrc.empty())
		return false;
	std::string fragmentSrc = loadFile(fragmentPath);
	if (fragmentSrc.empty())
		return false;
	const char * vertShader = vertexSrc.c_str();
	const char * fragShader = fragmentSrc.c_str();

	//Creating shaders
	GLchar infoLog[512];	//Char array to hold information log if compile/link fails.
	GLint compileSuccess;
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertShader, 0);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileSuccess);
	if (compileSuccess == GL_FALSE)	//Compile failed
	{
		std::cout << "Vertex shader compilation failed." << std::endl;
		glGetShaderInfoLog(vertexShader, 512, 0, infoLog);
		std::cout << infoLog << std::endl;
		return false;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragShader, 0);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileSuccess);
	if (compileSuccess == GL_FALSE)
	{
		std::cout << "Fragment shader compilation failed." << std::endl;
		glGetShaderInfoLog(fragmentShader, 512, 0, infoLog);
		std::cout << infoLog << std::endl;
		return false;
	}

	//Create and link program with shaders
	m_program = glCreateProgram();
	glAttachShader(m_program, vertexShader);
	glAttachShader(m_program, fragmentShader);
	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &compileSuccess);
	if (compileSuccess == GL_FALSE)
	{
		std::cout << "Shader program linking failed." << std::endl;
		glGetProgramInfoLog(m_program, 512, 0, infoLog);
		std::cout << infoLog << std::endl;
		return false;
	}
	return true;
}

void Shader::destroy()
{
	glDeleteProgram(m_program);
}

void Shader::bind()
{
	glUseProgram(m_program);
}

GLuint Shader::getHandle()
{
	return m_program;
}

std::string Shader::loadFile(std::string path)
{
	if (!std::filesystem::exists(path))	//use c++ filesystem API to check if the given filename exists.
	{
		std::cout << "File " << path << " does not exist. " << std::endl;
		return std::string();	//Return empty str.
	}
	std::ifstream inFile(path);
	std::stringstream sbuf;
	sbuf << inFile.rdbuf();
	return sbuf.str();
}

ShaderDescriptor Shader::getDescriptor()
{
	return m_descriptor;
}