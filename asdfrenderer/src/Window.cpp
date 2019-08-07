#include "Window.h"

bool Window::init(int width, int height, int glmajor, int glminor, int samples, std::string title)
{
	if (!glfwInit())
	{
		std::cout << "Failed to init GLFW" << std::endl;
		return false;
	}
	//Setting configuration for the current window.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glmajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glminor);
	glfwWindowHint(GLFW_SAMPLES, samples);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!m_window)
	{
		std::cout << "Failed to create window" << std::endl;
		return false;
	}
	glfwMakeContextCurrent(m_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))	//Initialize GLAD so opengl functions that aren't ancient can be used
	{
		std::cout << "Failed to load GLAD" << std::endl;
		return false;
	}

	//Enable some OpenGL States
	glEnable(GL_DEPTH_TEST);

	//Backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Alpha blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_dimensions = glm::vec2(width, height);
	return true;
}


void Window::destroy()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Window::swapBackBuffer(bool vsync)
{
	glfwSwapInterval((int)vsync);	//Swap interval of 1 == vertical sync for 1/RF, swap interval of 0 == 0/RF, AKA no vsync.
	glfwSwapBuffers(m_window);	//Swap back and front buffers.
}

bool Window::getShouldClose()
{
	glfwPollEvents();	
	return glfwWindowShouldClose(m_window);
}

bool Window::isKeyPressed(int key)
{
	return (glfwGetKey(m_window, key) == GLFW_PRESS);
}

GLFWwindow* Window::getHandle()
{
	return m_window;
}

glm::vec2 Window::getDimensions()
{
	return m_dimensions;
}