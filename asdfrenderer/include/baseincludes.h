//Base includes for renderer components. just GL related stuff. messy because include format has to be same as for when it's included in source.
#pragma once
#include <../libs/glad/include/glad/glad.h>
#include <../libs/glfw-3.2.1/include/GLFW/glfw3.h>

#include <../libs/glm-0.9.9-a2/glm/glm.hpp>
#include <../libs/glm-0.9.9-a2/glm/gtc/matrix_transform.hpp>
#include <../libs/glm-0.9.9-a2/glm/gtc/type_ptr.hpp>

#include <../libs/assimp/include/assimp/Importer.hpp>
#include <../libs/assimp/include/assimp/scene.h>
#include <../libs/assimp/include/assimp/postprocess.h>

struct ShaderDescriptor
{
	bool canTexture;
	int textureBinding;

	bool canNormalMap;
	int normalMapBinding;

	int positionBinding;
	int normalBinding;
	int texcoordBinding;

	int pvBinding;
	int modelBinding;
};