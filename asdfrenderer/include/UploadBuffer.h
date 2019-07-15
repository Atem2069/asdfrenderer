//Class that implements uniform buffer objects. Any struct can be passed as void* with any size, and be given alongside some sort of binding point in order to upload data to whatever shader is currently bound.
#pragma once

#include "baseincludes.h"
#include <iostream>

class UploadBuffer
{
public:
	bool init(int bindingPoint, void* data, size_t dataSize);
	void destroy();

	void update(void* data, size_t dataSize, int offset);

	void upload();
private:
	GLuint m_UBO;
	int m_bindingPoint;
};
