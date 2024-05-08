#include "pch.h"
#include "shader.h"
#include "mesh_class.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
public:
	std::vector<Mesh> meshes;
	std::string dir;
	std::vector<Texture> texturesLoaded;

	Model(std::string const &path) {
		loadModel(path);
	}
	void draw(Shader& shader) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].draw(shader);
		}
	}
	
private:
	

	void loadModel(std::string const &path) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || !scene->mRootNode || scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE) {
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		}

		dir = path.substr(0, path.find_last_of('/'));
		recurseNode(scene->mRootNode, scene);

	}

	void recurseNode(aiNode *rNode, const aiScene *scene) {
		for (unsigned int i = 0; i < rNode->mNumMeshes; i++) {
			aiMesh *mesh = scene->mMeshes[rNode->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}

		for (unsigned int i = 0; i < rNode->mNumChildren; i++) {
			recurseNode(rNode->mChildren[i], scene);
		}
	}
	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;

			vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;		

			if (mesh->mTextureCoords[0]) {
				vec2 vector;
				vector.x = mesh->mTextureCoords[0][i].x;
				vector.y = mesh->mTextureCoords[0][i].y;

				vertex.TexCoords = vector;
			}
			else {
				vertex.TexCoords = vec2(0.0f, 0.0f);
			}



			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}
			
		if (mesh->mMaterialIndex > 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> difMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
						
			textures.insert(textures.end(), difMaps.begin(), difMaps.end());

			std::vector<Texture> specMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");

			textures.insert(textures.end(), specMaps.begin(), specMaps.end());

		}
		return Mesh(vertices, indices, textures);
	}

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
		std::vector<Texture> texts;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString s;
			mat->GetTexture(type, i, &s);
			bool skip = false;
			for (unsigned int j = 0; j < texturesLoaded.size(); j++)
			{
				if (std::strcmp(texturesLoaded[j].path.data(), s.C_Str()) == 0)
				{
					texts.push_back(texturesLoaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture t;
				t.ID = textureFromFile(s.C_Str(), this->dir);
				t.type = typeName;
				t.path = s.C_Str();
				texts.push_back(t);
				texturesLoaded.push_back(t);
			}

		}
		return texts;
	}

	unsigned int textureFromFile(const char *path, const string &dir) {
		unsigned int tID;
		glGenTextures(1, &tID);

		int width, height, nrComponents;
		std::string filePath = string(path);
		filePath = dir + '/' + filePath;
		unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, tID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		}
		else {
			std::cout << "fail load at " << path << std::endl;
		}
		stbi_image_free(data);
		return tID;

	}
};
