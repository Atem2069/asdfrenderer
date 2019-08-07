#include "Camera.h"

bool PerspectiveCamera::init(int screenWidth, int screenHeight, float fovRadians, float depth)
{
	m_projection = glm::perspective(fovRadians, (float)screenWidth / (float)screenHeight, 1.0f, depth);
	m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	return true;
}

void PerspectiveCamera::destroy()
{
	//todo
}

void PerspectiveCamera::update(int projectionViewBinding, glm::vec3 camPosition, glm::vec3 camLookAt)
{
	m_view = glm::lookAt(camPosition, camLookAt, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 m_projectionView = m_projection * m_view;
	glUniformMatrix4fv(projectionViewBinding, 1, GL_FALSE, glm::value_ptr(m_projectionView));
}