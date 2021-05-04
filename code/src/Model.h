#pragma once

#include "Shader.h"
#include "Texture.h"

#include <vector>

class Model
{
public:
	GLuint vao;
	GLuint vbo[3];

	glm::mat4 objMat = glm::mat4(1.f);

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	
	Shader shader;
	//Texture texture;
	
	Model();
	Model(Shader _shader, const char* path);

	~Model();

	void Draw();
};