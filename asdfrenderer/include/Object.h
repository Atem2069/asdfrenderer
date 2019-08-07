//Basic render object class. This contains a base method for creating an object from a set of vertices, and will have other classes inheriting from it for use with textures, etc. 
#pragma once

#include "baseincludes.h"
#include <iostream>
#include <filesystem>
#include <vector>

#include <../libs/stb_image.h>
//Basic vertex type
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

//Basic mesh type. An object is made up of a number of meshes.
struct Mesh
{
	GLuint m_VBO;
	GLuint m_VAO;
	GLuint m_IBO;
	GLuint m_texture;
	GLuint m_normalMap;
	std::string m_texturePath;
	int m_numIndices;
};



//Basic Object is probably not going to be worked on anymore. It simply takes some vertices provided by the end person, then renders it witha single VBO on a set binding point. Nothing else.
class BasicObject
{
public:
	virtual bool init(Vertex * vertices, int numVertices);
	virtual void destroy();

	virtual void draw();

	virtual void update(Vertex * newVertices, int numVertices, int offset);

private:
	int m_numVertices;
	GLuint m_VBO, m_VAO;
};

//Object type that inherits from Basic Object. This class can render complex geometry with vertices and normals, however it cannot do textures.
class Object : BasicObject
{
public:
	bool init(std::string modelPath, ShaderDescriptor descriptor);
	void destroy();
	void draw();
	void transformModelMatrix(glm::mat4 newMatrix);
private:
	ShaderDescriptor m_descriptor;
	Mesh * m_meshes;
	int m_numMeshes;
	glm::mat4 m_modelMatrix;
};

//Object type similar to the complex geometry object type, however this one supports textures.
class TexturedObject : BasicObject
{
public:
	bool init(std::string modelPath, ShaderDescriptor descriptor);
	void destroy();
	void draw();
	void transformModelMatrix(glm::mat4 newMatrix);
private:
	ShaderDescriptor m_descriptor;
	Mesh * m_meshes;
	int m_numMeshes;
	glm::mat4 m_modelMatrix;
	GLuint createMeshTexture(aiMaterial* material, std::string workingDirectory, int currentIteration);
};


std::vector<Vertex> loadMeshVertices(aiMesh* mesh);
std::vector<unsigned int> loadMeshIndices(aiMesh* mesh);

