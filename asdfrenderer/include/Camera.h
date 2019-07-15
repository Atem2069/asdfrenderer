#pragma once

#include "baseincludes.h"

class Camera
{
public:
	virtual bool init() { return true; }
	virtual void destroy() {}

	virtual void update() {}

	glm::mat4 getProjection() { return m_projection; }
	glm::mat4 getView() { return m_view; }
private:
	glm::mat4 m_projection;
	glm::mat4 m_view;
};

class PerspectiveCamera : public Camera
{
public:
	bool init(int screenWidth, int screenHeight, float fovRadians, float depth);
	void destroy();

	void update(int projectionViewBinding, glm::vec3 camPosition, glm::vec3 camLookAt);
private:
	glm::mat4 m_projection;
	glm::mat4 m_view;
};

