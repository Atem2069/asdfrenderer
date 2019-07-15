#include "UploadBuffer.h"

bool UploadBuffer::init(int bindingPoint, void* data, size_t dataSize)
{
	//Saving bind point for glBindBufferBase call down the line
	m_bindingPoint = bindingPoint;
	//Initialize UBO
	glGenBuffers(1, &m_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferData(GL_UNIFORM_BUFFER, dataSize, data, GL_DYNAMIC_DRAW);	//dynamic - going to be updated likely per-frame
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	int err = glGetError();
	if (err != GL_NO_ERROR)
	{
		std::cout << "Failed to create UBO: " << err << std::endl;
		return false;
	}

	return true;
}

void UploadBuffer::destroy()
{
	glDeleteBuffers(1, &m_UBO);
}

void UploadBuffer::update(void* data, size_t dataSize, int offset)
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UploadBuffer::upload()
{
	//We're going to assume that the binding point for the shader is going to be the same as the binding point for the UBO. it makes sense and should keep everything in-line.
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_UBO);
}