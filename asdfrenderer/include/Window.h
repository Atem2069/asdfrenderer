#pragma once

#include "baseincludes.h"
#include <iostream>

class Window
{
public:
	bool init(int width, int height, int glmajor, int glminor, int samples, std::string title);
	void destroy();

	void swapBackBuffer(bool vsync);

	bool getShouldClose();

	bool isKeyPressed(int key);

	glm::vec2 getDimensions();

	GLFWwindow* getHandle();
private:
	glm::vec2 m_dimensions;
	GLFWwindow * m_window;
};