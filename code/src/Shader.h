#pragma once

#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	
	// PROGRAM ID
	GLuint program;

	//Constructor must read and build the shader
	Shader();
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);
	
	~Shader();

	// Use/Activate the shader
	void Use();

	//Utility uniform functions
	void SetMatrix(const std::string& name, int size, bool transpose, const float *value);
	void SetFloat(const std::string &name, float val1, float val2, float val3, float val4) const;
	void SetFloat(const std::string &name, float val1, float val2, float val3) const;
	void SetFloat(const std::string &name, float val1) const;
	void SetInt(const std::string &name, int val1) const;
};