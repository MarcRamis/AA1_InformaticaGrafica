#pragma once

//#include <GL\glew.h>
//#include <glm\gtc\type_ptr.hpp>
//#include <glm\gtc\matrix_transform.hpp>
//
//GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name = "") {
//	GLuint shader = glCreateShader(shaderType);
//	glShaderSource(shader, 1, &shaderStr, NULL);
//	glCompileShader(shader);
//	GLint res;
//	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
//	if (res == GL_FALSE) {
//		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
//		char* buff = new char[res];
//		glGetShaderInfoLog(shader, res, &res, buff);
//		fprintf(stderr, "Error Shader %s: %s", name, buff);
//		delete[] buff;
//		glDeleteShader(shader);
//		return 0;
//	}
//	return shader;
//}
//void linkProgram(GLuint program) {
//	glLinkProgram(program);
//	GLint res;
//	glGetProgramiv(program, GL_LINK_STATUS, &res);
//	if (res == GL_FALSE) {
//		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
//		char* buff = new char[res];
//		glGetProgramInfoLog(program, res, &res, buff);
//		fprintf(stderr, "Error Link: %s", buff);
//		delete[] buff;
//	}
//}