#pragma once

#include "Baseincludes.h"
#include "Light.h"
#include "Camera.h"

class DirectionalShadowMap
{
public:
	bool init(float shadowWidth, float shadowHeight, DirectionalLight &lightInfo);
	void destroy();

	void beginFrame(DirectionalLight &lightInfo);
	void endFrame();

	GLuint getDepthTexture();
	void bindShadowCamera(int binding);

private:
	OrthoCamera m_camera;
	float width, height;
	GLuint m_program;
	GLuint m_FBO;
	GLuint m_depthTexture;
};