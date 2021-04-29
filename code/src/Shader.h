#pragma once

#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

//extern GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name);
//extern void linkProgram(GLuint program);

class Shader
{
public:
	
	// PROGRAM ID
	GLuint program;

	//Constructor must read and build the shader
	Shader();
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);
	// Use/Activate the shader
	void Use();

	//Utility uniform functions
	void SetFloat(const std::string &name, float val1, float val2, float val3, float val4) const;
	void SetFloat(const std::string &name, float val1, float val2, float val3) const;
	void SetFloat(const std::string &name, float val1) const;
	void SetInt(const std::string &name, int val1) const;
};