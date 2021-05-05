#pragma once

#include "Shader.h"
#include "Texture.h"

#include <vector>

struct Light
{
	glm::vec3 pos;

	glm::vec3 ambient_color, diffuse_color, specular_color;
	float ambient_strength, diffuse_strength, specular_strength;
	float shininess;
	
	Light(glm::vec3 _pos, glm::vec3 a_color, glm::vec3 d_color, glm::vec3 s_color,  
		float a_str, float d_str, float s_str, float _shininess)
		: pos(_pos), ambient_color(a_color), diffuse_color(), specular_color(s_color),
		ambient_strength(a_str), diffuse_strength(d_str), specular_strength(s_str), shininess(_shininess){}
};

struct ObjectParameters
{
	glm::vec3 pos;
	glm::vec3 color;
	bool haveAmbient, haveDiffuse, haveSpecular;

	ObjectParameters(glm::vec3 _pos, glm::vec3 _color, bool _ambient, bool _diffuse, bool _specular)
		: pos(_pos), color(_color), haveAmbient(_ambient), haveDiffuse(_diffuse), haveSpecular(_specular){}
};

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
	ObjectParameters obj;

	Model();
	Model(Shader _shader, const char* path, ObjectParameters objParameters);

	~Model();

	void Draw();
};