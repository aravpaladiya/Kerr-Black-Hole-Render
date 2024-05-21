#include "pch.h"
#include "shader.h"
#include <vector>
using namespace glm;
using namespace std;
struct Vertex {
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
};

struct Texture {
	unsigned int ID;
	string type;
	string path;
};

class Mesh {
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	Mesh(vector<Vertex> v, vector<unsigned int> i, vector<Texture> t) {
		this->textures = t;
		this->vertices = v;
		this->indices = i;
		createMesh();
	}

	void draw(Shader &shader) {
		unsigned int NR_DIFFUSE = 1;
		unsigned int NR_SPEC = 1;

		for (int i = 0; i < textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			string number;
			string name = textures[i].type;
			if (name == "texture_diffuse") {
				number = to_string(NR_DIFFUSE++);
			}
			else if (name == "texture_specular") {
				number = to_string(NR_SPEC++);
			}

			shader.setI(name + number, i);
			glBindTexture(GL_TEXTURE_2D, textures[i].ID);
		}

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(0);
	}

private:
	void createMesh(){
		unsigned int VBO, EBO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));



	}



};
