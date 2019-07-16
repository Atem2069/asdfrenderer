#define STB_IMAGE_IMPLEMENTATION
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

//Now for assimp initialized object code.

bool Object::init(std::string filePath, ShaderDescriptor descriptor)
{
	//Set shader descriptor
	m_descriptor = descriptor;

	if (!std::filesystem::exists(filePath))
	{
		std::cout << "File " << filePath << " does not exist." << std::endl;
		return false;
	}

	//Getting directory so textures can be loaded if mtl file wants them
	std::string dir = filePath.substr(0, filePath.find_last_of(R"(\)")) + R"(\)";

	//Importing stuff with assimp
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_GenNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "Given model failed to load: " << importer.GetErrorString() << std::endl;
		return false;
	}

	m_meshes = new Mesh[scene->mNumMeshes];
	m_numMeshes = scene->mNumMeshes;

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		Mesh tempMesh = {};
		aiMesh* mesh = scene->mMeshes[i];

		//Loading vertices and indices.
		std::vector<Vertex> tempVertices = loadMeshVertices(mesh);
		std::vector<unsigned int> m_indices = loadMeshIndices(mesh);

		//Creating buffers and providing them to each temp mesh
		glGenBuffers(1, &tempMesh.m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, tempMesh.m_VBO);
		glBufferData(GL_ARRAY_BUFFER, tempVertices.size() * sizeof(Vertex), &tempVertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &tempMesh.m_IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tempMesh.m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &tempMesh.m_VAO);
		glBindVertexArray(tempMesh.m_VAO);
		glVertexAttribPointer(m_descriptor.positionBinding, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
		glEnableVertexAttribArray(m_descriptor.positionBinding);
		glVertexAttribPointer(m_descriptor.normalBinding, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));	//Offsetof call gets the offset into each vertex when given.
		glEnableVertexAttribArray(m_descriptor.normalBinding);
		glVertexAttribPointer(m_descriptor.texcoordBinding, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glEnableVertexAttribArray(m_descriptor.texcoordBinding);
		glBindVertexArray(0);

		tempMesh.m_numIndices = m_indices.size();
		m_meshes[i] = tempMesh;
	}
	return true;
}

void Object::destroy()
{
	for (int i = 0; i < m_numMeshes; i++)
	{
		glDeleteVertexArrays(1, &m_meshes[i].m_VAO);
		glDeleteBuffers(1, &m_meshes[i].m_VBO);
		glDeleteBuffers(1, &m_meshes[i].m_IBO);
	}
}

void Object::draw()
{
	for (int i = 0; i < m_numMeshes; i++)
	{	
		glBindVertexArray(m_meshes[i].m_VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshes[i].m_IBO);
		glDrawElements(GL_TRIANGLES, m_meshes[i].m_numIndices, GL_UNSIGNED_INT, nullptr);
	}
}

//Textured object.

bool TexturedObject::init(std::string filePath, ShaderDescriptor descriptor)
{
	//Set shader descriptor
	m_descriptor = descriptor;

	if (!std::filesystem::exists(filePath))
	{
		std::cout << "File " << filePath << " does not exist." << std::endl;
		return false;
	}

	//Getting directory so textures can be loaded if mtl file wants them
	std::string dir = filePath.substr(0, filePath.find_last_of(R"(\)")) + R"(\)";

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

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		Mesh tempMesh = {};
		aiMesh* mesh = scene->mMeshes[i];

		//Loading vertices and indices.
		std::vector<Vertex> tempVertices = loadMeshVertices(mesh);
		std::vector<unsigned int> m_indices = loadMeshIndices(mesh);

		//Loading a texture for mesh
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];

			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
				tempMesh.m_texture = createMeshTexture(material, dir, i);

		}
		//Creating buffers and providing them to each temp mesh
		glGenBuffers(1, &tempMesh.m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, tempMesh.m_VBO);
		glBufferData(GL_ARRAY_BUFFER, tempVertices.size() * sizeof(Vertex), &tempVertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &tempMesh.m_IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tempMesh.m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &tempMesh.m_VAO);
		glBindVertexArray(tempMesh.m_VAO);
		glVertexAttribPointer(m_descriptor.positionBinding, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
		glEnableVertexAttribArray(m_descriptor.positionBinding);
		glVertexAttribPointer(m_descriptor.normalBinding, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));	//Offsetof call gets the offset into each vertex when given.
		glEnableVertexAttribArray(m_descriptor.normalBinding);
		glVertexAttribPointer(m_descriptor.texcoordBinding, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glEnableVertexAttribArray(m_descriptor.texcoordBinding);
		glBindVertexArray(0);

		tempMesh.m_numIndices = m_indices.size();
		m_meshes[i] = tempMesh;
	}
	return true;
}

void TexturedObject::destroy()
{

}

void TexturedObject::draw()
{
	for (int i = 0; i < m_numMeshes; i++)
	{
		glActiveTexture(GL_TEXTURE0+m_descriptor.textureBinding);
		glBindTexture(GL_TEXTURE_2D, m_meshes[i].m_texture);
		glBindVertexArray(m_meshes[i].m_VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshes[i].m_IBO);
		glDrawElements(GL_TRIANGLES, m_meshes[i].m_numIndices, GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
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

GLuint TexturedObject::createMeshTexture(aiMaterial* material, std::string workingDirectory, int currentIteration)
{
	GLuint m_texture = 0;
	//This is only run assuming texcount is valid, otherwise dumb stuff happens
	aiString texPath;
	material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
	std::string texFile = texPath.C_Str();
	std::string texturePath = workingDirectory + texFile;

	m_meshes[currentIteration].m_texturePath = texturePath;

	int width, height, channels;
	unsigned char * image = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);
	if (!image)
	{
		std::cout << "Failure loading image." << std::endl;
	}
	GLenum loadType;
	if (channels == 3)
		loadType = GL_RGB;
	if (channels == 4)
		loadType = GL_RGBA;

	//Generating texture
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, loadType, GL_UNSIGNED_BYTE, image);
	stbi_image_free(image);
	//Filtering stuff
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Unbinding if something added forgets to bind
	glBindTexture(GL_TEXTURE_2D, 0);

	return m_texture;
}