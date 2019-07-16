#pragma once
#include "baseincludes.h"
#include <iostream>
struct BasicLight
{
	glm::vec4 cameraPosition;
	glm::vec4 position;
	glm::vec4 color;
};

struct DirectionalLight
{
	glm::vec4 cameraPosition;
	glm::vec4 lightDirection;
	glm::vec4 color;
};