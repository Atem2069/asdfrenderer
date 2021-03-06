#define STB_IMAGE_IMPLEMENTATION
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#include "Object.h"

bool BasicObject::init(Vertex * vertices, int numVertices)
{
	m_numVertices = numVertices;

	//Creating Vertex Buffer
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vertex), vertices, GL_STATIC_DRAW);
	//Create Vertex Array Buffer
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	//Due to the vertex struct type having three components, regardless of whatever the shader inputs are, it will always be setting attrib pointers to 0 (pos), 1 (normal), and 2 (uv). The shader should conform to this.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));	//Offsetof call gets the offset into each vertex when given.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	return true;
}

void BasicObject::destroy()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void BasicObject::draw()
{
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, m_numVertices);
}

void BasicObject::update(Vertex * newVertices, int numVertices, int offset)	
{
	//First, calling subdata to input new data
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, offset, numVertices * sizeof(Vertex), newVertices);
	GLint64 bufSize;
	//Then, getting the new buffer size
	glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufSize);
	m_numVertices = (int)bufSize / (int)sizeof(Vertex);	//Divide buffer size by the sizeof of a Vertex type to get the vertex count. Unless the data is corrupt or a data call is forced by some idiot this should work
	std::cout << bufSize << " " << m_numVertices << std::endl;
}

//Textured object.

bool TexturedObject::init(std::string filePath, ShaderDescriptor descriptor)
{
	m_modelMatrix = glm::mat4(1);
	//Set shader descriptor
	m_descriptor = descriptor;

	if (!std::filesystem::exists(filePath))
	{
		std::cout << "File " << filePath << " does not exist." << std::endl;
		return false;
	}

	//Getting directory so textures can be loaded if mtl file wants them
	std::string dir = filePath.substr(0, filePath.find_last_of(R"(\)")) + R"(\)";
	m_workingPath = dir;
	//Importing stuff with assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_GenNormals | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Given model failed to load: " << importer.GetErrorString() << std::endl;
		return false;
	}

	m_meshes = new Mesh[scene->mNumMeshes];
	m_numMeshes = scene->mNumMeshes;

	processNode(scene, scene->mRootNode);
	return true;
}

void TexturedObject::destroy()
{

}

void TexturedObject::draw()
{
	for (int i = 0; i < m_numMeshes; i++)
	{
		glEnable(GL_BLEND);
		glUniformMatrix4fv(m_descriptor.modelBinding, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));

		if (m_meshes[i].m_hasTexture)
		{
			glActiveTexture(GL_TEXTURE0 + m_descriptor.textureBinding);
			glBindTexture(GL_TEXTURE_2D, m_meshes[i].m_texture);
		}
		
		glUniform1i(5, (int)m_meshes[i].m_bindAlphaMask);
		if (m_meshes[i].m_bindAlphaMask)
		{
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, m_meshes[i].m_alphaMask);
		}
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshes[i].m_IBO);
		glDrawElementsBaseVertex(GL_TRIANGLES, m_meshes[i].m_numIndices, GL_UNSIGNED_INT, nullptr,m_meshes[i].m_baseVertex);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void TexturedObject::transformModelMatrix(glm::mat4 newMatrix)
{
	m_modelMatrix = newMatrix;
}





//Helper funcs

std::vector<Vertex> loadMeshVertices(aiMesh* mesh)
{
	std::vector<Vertex> vertices;
	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex tempVertex = {};
		tempVertex.position.x = mesh->mVertices[i].x;
		tempVertex.position.y = mesh->mVertices[i].y;
		tempVertex.position.z = mesh->mVertices[i].z;
		tempVertex.normal.x = mesh->mNormals[i].x;
		tempVertex.normal.y = mesh->mNormals[i].y;
		tempVertex.normal.z = mesh->mNormals[i].z;
		if (mesh->HasTextureCoords(0))
		{
			tempVertex.uv.x = mesh->mTextureCoords[0][i].x;
			tempVertex.uv.y = mesh->mTextureCoords[0][i].y;
		}


		vertices.push_back(tempVertex);
	}

	return vertices;
}

std::vector<unsigned int> loadMeshIndices(aiMesh* mesh)
{
	std::vector<unsigned int> indices;
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		if (face.mNumIndices != 3)
			continue;

		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	return indices;
}

GLuint TexturedObject::createMeshTexture(aiMaterial* material, std::string workingDirectory, int currentIteration, int materialIndex)
{
	GLuint m_texture = 0;
	//This is only run assuming texcount is valid, otherwise dumb stuff happens
	aiString texPath;
	material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
	std::string texFile = texPath.C_Str();
	std::string texturePath = workingDirectory + texFile;
	m_meshes[currentIteration].m_texturePath = texturePath;	//hacky but need a way to store texture paths.
	//Searching if textures exist already
	for (int i = 0; i < currentIteration; i++)
	{
		if (m_meshes[i].m_materialIndex == materialIndex)
			return m_meshes[i].m_texture;
	}

	int width, height, channels;
	unsigned char * image = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);
	if (!image)
	{
		std::cout << "Failure loading image." << std::endl;
	}
	GLenum loadType;
	loadType = GL_RGB;
	if (channels == 4)
		loadType = GL_RGBA;

	//Generating texture
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, width, height, 0, loadType, GL_UNSIGNED_BYTE, image);
	stbi_image_free(image);
	//Filtering stuff
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 4.0f);
	glGenerateMipmap(GL_TEXTURE_2D);	//Mipmaps 

	//Unbinding if something added forgets to bind
	glBindTexture(GL_TEXTURE_2D, 0);

	return m_texture;
}

void TexturedObject::processNode(const aiScene * scene, aiNode * node)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(scene, mesh, m_meshes[node->mMeshes[i]], node->mMeshes[i]);
	}

	for (int i = 0; i < node->mNumChildren; i++)
		processNode(scene, node->mChildren[i]);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

void TexturedObject::processMesh(const aiScene* scene, aiMesh * aimesh, Mesh & mesh, int currentIteration)
{
	//Loading vertices and indices.
	//std::vector<Vertex> tempVertices = loadMeshVertices(aimesh);
	for (int i = 0; i < aimesh->mNumVertices; i++)
	{
		Vertex tempVertex = {};
		tempVertex.position.x = aimesh->mVertices[i].x;
		tempVertex.position.y = aimesh->mVertices[i].y;
		tempVertex.position.z = aimesh->mVertices[i].z;
		tempVertex.normal.x = aimesh->mNormals[i].x;
		tempVertex.normal.y = aimesh->mNormals[i].y;
		tempVertex.normal.z = aimesh->mNormals[i].z;
		if (aimesh->HasTextureCoords(0))
		{
			tempVertex.uv.x = aimesh->mTextureCoords[0][i].x;
			tempVertex.uv.y = aimesh->mTextureCoords[0][i].y;
		}


		m_vertices.push_back(tempVertex);
	}

	std::vector<unsigned int> m_indices = loadMeshIndices(aimesh);
	mesh.m_baseVertex = m_lastIndex;
	m_lastIndex += aimesh->mNumVertices;

	//Loading a texture for mesh
	if (aimesh->mMaterialIndex >= 0)
	{
		aiMaterial * material = scene->mMaterials[aimesh->mMaterialIndex];
		mesh.m_materialIndex = aimesh->mMaterialIndex;
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			mesh.m_hasTexture = true;
			mesh.m_texture = createMeshTexture(material, m_workingPath, currentIteration,aimesh->mMaterialIndex);
		}


		if (material->GetTextureCount(aiTextureType_OPACITY) > 0)
		{
			bool doMaskLoad = true;
			mesh.m_bindAlphaMask = true;

			aiString texPath;
			material->GetTexture(aiTextureType_OPACITY, 0, &texPath);
			std::string texFile = texPath.C_Str();
			std::string texturePath = m_workingPath + texFile;

			for (int i = 0; i < currentIteration; i++)
			{
				if (m_meshes[i].m_materialIndex == aimesh->mMaterialIndex)
				{
					mesh.m_alphaMask = m_meshes[i].m_alphaMask;
					doMaskLoad = false;
				}
			}

			if (doMaskLoad)
			{
				int width, height, channels;
				unsigned char * image = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);
				if (!image)
					std::cout << "Failure loading image." << std::endl;

				GLint storeMethod = GL_RED;
				if (channels > 1)
					storeMethod = GL_RG;
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				//Generating texture
				glGenTextures(1, &mesh.m_alphaMask);
				glBindTexture(GL_TEXTURE_2D, mesh.m_alphaMask);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, storeMethod, GL_UNSIGNED_BYTE, image);
				stbi_image_free(image);
				//Filtering stuff
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

				//Unbinding if something added forgets to bind
				glBindTexture(GL_TEXTURE_2D, 0);
			}

		}

	}

	//Creating buffers and providing them to each temp mesh
	glGenBuffers(1, &mesh.m_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

	mesh.m_numIndices = m_indices.size();
}
