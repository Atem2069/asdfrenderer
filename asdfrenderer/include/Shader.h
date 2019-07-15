#pragma once

#include "baseincludes.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

class Shader
{
public:
	bool init(std::string vertexPath, std::string fragmentPath, ShaderDescriptor descriptor);
	void destroy();

	void bind();

	GLuint getHandle();

	ShaderDescriptor getDescriptor();
private:
	ShaderDescriptor m_descriptor;
	GLuint m_program;
	std::string loadFile(std::string path);
};