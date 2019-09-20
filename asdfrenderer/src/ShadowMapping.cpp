#include "ShadowMapping.h"

bool DirectionalShadowMap::init(float shadowWidth, float shadowHeight, DirectionalLight &lightInfo)
{
	width = shadowWidth;
	height = shadowHeight;

	glGenTextures(1, &m_depthTexture);
	glBindTexture(GL_TEXTURE_2D, m_depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/*Basic vertex shader to transform to shadow coords*/
	std::string shadowVertexShader = R"(
	#version 460 core
	layout(location=0) in vec3 position;
	layout(location=0) uniform mat4 lightSpaceMatrix;
	layout(location=1) uniform mat4 model;
	
	void main()
	{
		gl_Position = lightSpaceMatrix * model * vec4(position,1.0f);
	}
	)";

	/*No colour-buffer write done*/
	std::string shadowFragmentShader = R"(
	#version 460 core
	
	void main()
	{
	}
	)";

	const char * shadowVertShader = shadowVertexShader.c_str();
	const char * shadowFragShader = shadowFragmentShader.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shadowVertShader, 0);
	glCompileShader(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shadowFragShader, 0);
	glCompileShader(fs);

	m_program = glCreateProgram();
	glAttachShader(m_program, vs);
	glAttachShader(m_program, fs);
	glLinkProgram(m_program);


	m_camera.init(1024, 1024, 0.0f, 10000.0f);	//Seems to be the best trade-off for shadow quality.

	return true;
}

void DirectionalShadowMap::destroy()
{
	//todo
}

void DirectionalShadowMap::beginFrame(DirectionalLight& lightInfo)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_program);
	m_camera.update(0, glm::vec3(-lightInfo.lightDirection), glm::vec3(lightInfo.lightDirection));

	glViewport(0, 0, width, height);
}

void DirectionalShadowMap::endFrame()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

GLuint DirectionalShadowMap::getDepthTexture()
{
	return m_depthTexture;
}

void DirectionalShadowMap::bindDepthTexture(int binding)
{
	glActiveTexture(GL_TEXTURE0 + binding);
	glBindTexture(GL_TEXTURE_2D, m_depthTexture);
}

void DirectionalShadowMap::bindShadowCamera(int binding)
{
	glUniformMatrix4fv(binding, 1, GL_FALSE, glm::value_ptr(m_camera.getProjectionView()));
}