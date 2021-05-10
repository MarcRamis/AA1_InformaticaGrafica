#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\random.hpp>
#include <cstdio>
#include <cassert>

// IMGUI
#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>

#include "GL_framework.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include <glm/gtx/string_cast.hpp>

#include "Model.h"

///////////////////// VARIABLES
glm::vec4 wPos; // Camera pos
float moveWTime = 0.0f; // move in world time

// DOLLY EFFECT
float width = 16.f;
float radians = 65.f;
bool moveCamera = false;

// Random explode
float randomX, randomY, randomZ;
glm::vec3 random;

// DISCARD EFFECT
bool isMatrix = false;
float displaceX = 5.0f;
float displaceY = 5.0f;

Model *billboard;
Model *scenario;
Model *sword;

Light phong = Light(glm::vec4(0.f,0.f,1.f, 1.f), 
	glm::vec4(1.f,1.f,1.f,1.f), glm::vec4(1.f, 1.f, 1.f,1.f), glm::vec4(1.f, 0.f, 0.f,1.f),
	0.2f,0.3f,0.5f,30.f);

///////////////////// TEACHER FUNCTIONS
#pragma region Teacher functions

GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name = "") {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char* buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
void linkProgram(GLuint program) {
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char* buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}

///////// fw decl
namespace ImGui {
	void Render();
}
namespace Axis {
	void setupAxis();
	void cleanupAxis();
	void drawAxis();
}
////////////////

namespace RenderVars {
	float FOV = glm::radians(radians);
	const float zNear = 1.f;
	const float zFar = 50.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -5.f, -15.f };
	float rota[2] = { 0.f, 0.f };
}
namespace RV = RenderVars;

void GLResize(int width, int height) {
	glViewport(0, 0, width, height);
	if (height != 0) RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	else RV::_projection = glm::perspective(RV::FOV, 0.f, RV::zNear, RV::zFar);
}
void GLmousecb(MouseEvent ev) {
	if (RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) {
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch (ev.button) {
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	}
	else {
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}

////////////////////////////////////////////////// AXIS
namespace Axis {
	GLuint AxisVao;
	GLuint AxisVbo[3];
	GLuint AxisShader[2];
	GLuint AxisProgram;

	float AxisVerts[] = {
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};
	float AxisColors[] = {
		1.0, 0.0, 0.0, 1.0,
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, 1.0, 1.0
	};
	GLubyte AxisIdx[] = {
		0, 1,
		2, 3,
		4, 5
	};
	const char* Axis_vertShader =
		"#version 330\n\
in vec3 in_Position;\n\
in vec4 in_Color;\n\
out vec4 vert_color;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	vert_color = in_Color;\n\
	gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
}";
	const char* Axis_fragShader =
		"#version 330\n\
in vec4 vert_color;\n\
out vec4 out_Color;\n\
void main() {\n\
	out_Color = vert_color;\n\
}";

	void setupAxis() {
		glGenVertexArrays(1, &AxisVao);
		glBindVertexArray(AxisVao);
		glGenBuffers(3, AxisVbo);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisColors, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AxisVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, AxisIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		AxisShader[0] = compileShader(Axis_vertShader, GL_VERTEX_SHADER, "AxisVert");
		AxisShader[1] = compileShader(Axis_fragShader, GL_FRAGMENT_SHADER, "AxisFrag");

		AxisProgram = glCreateProgram();
		glAttachShader(AxisProgram, AxisShader[0]);
		glAttachShader(AxisProgram, AxisShader[1]);
		glBindAttribLocation(AxisProgram, 0, "in_Position");
		glBindAttribLocation(AxisProgram, 1, "in_Color");
		linkProgram(AxisProgram);
	}
	void cleanupAxis() {
		glDeleteBuffers(3, AxisVbo);
		glDeleteVertexArrays(1, &AxisVao);

		glDeleteProgram(AxisProgram);
		glDeleteShader(AxisShader[0]);
		glDeleteShader(AxisShader[1]);
	}
	void drawAxis() {
		glBindVertexArray(AxisVao);
		glUseProgram(AxisProgram);
		glUniformMatrix4fv(glGetUniformLocation(AxisProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}
////////////////////////////////////////////////// CUBE
namespace Cube {
	GLuint cubeVao;
	GLuint cubeVbo[3];
	GLuint cubeShaders[2];
	GLuint cubeProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	extern const float halfW = 0.5f;
	int numVerts = 24 + 6; // 4 vertex/face * 6 faces + 6 PRIMITIVE RESTART

						   //   4---------7
						   //  /|        /|
						   // / |       / |
						   //5---------6  |
						   //|  0------|--3
						   //| /       | /
						   //|/        |/
						   //1---------2
	glm::vec3 verts[] = {
		glm::vec3(-halfW, -halfW, -halfW),
		glm::vec3(-halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW, -halfW),
		glm::vec3(-halfW,  halfW, -halfW),
		glm::vec3(-halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW, -halfW)
	};
	glm::vec3 norms[] = {
		glm::vec3(0.f, -1.f,  0.f),
		glm::vec3(0.f,  1.f,  0.f),
		glm::vec3(-1.f,  0.f,  0.f),
		glm::vec3(1.f,  0.f,  0.f),
		glm::vec3(0.f,  0.f, -1.f),
		glm::vec3(0.f,  0.f,  1.f)
	};

	glm::vec3 cubeVerts[] = {
		verts[1], verts[0], verts[2], verts[3],
		verts[5], verts[6], verts[4], verts[7],
		verts[1], verts[5], verts[0], verts[4],
		verts[2], verts[3], verts[6], verts[7],
		verts[0], verts[4], verts[3], verts[7],
		verts[1], verts[2], verts[5], verts[6]
	};
	glm::vec3 cubeNorms[] = {
		norms[0], norms[0], norms[0], norms[0],
		norms[1], norms[1], norms[1], norms[1],
		norms[2], norms[2], norms[2], norms[2],
		norms[3], norms[3], norms[3], norms[3],
		norms[4], norms[4], norms[4], norms[4],
		norms[5], norms[5], norms[5], norms[5]
	};
	GLubyte cubeIdx[] = {
		0, 1, 2, 3, UCHAR_MAX,
		4, 5, 6, 7, UCHAR_MAX,
		8, 9, 10, 11, UCHAR_MAX,
		12, 13, 14, 15, UCHAR_MAX,
		16, 17, 18, 19, UCHAR_MAX,
		20, 21, 22, 23, UCHAR_MAX
	};

	const char* cube_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	out vec4 vert_Normal;\n\
	out vec4 fragPos;\n\
	mat4 normalMat;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		normalMat = transpose(inverse(objMat));\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = normalMat * vec4(in_Normal, 0.0);\n\
		fragPos = vec4(objMat * vec4(in_Position, 1.0));\n\
	}";

	const char* cube_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	in vec4 fragPos;\n\
	out vec4 out_Color;\n\
	uniform vec4 objColor;\n\
	uniform vec4 dir_light; \n\
	uniform vec4 ambient; \n\
	uniform vec4 ambient_color; \n\
	uniform vec4 diffuse; \n\
	uniform vec4 diffuse_color; \n\
	uniform vec4 specular; \n\
	uniform vec4 specular_color; \n\
	uniform vec4 viewPos; \n\
	uniform float shininess;\n\
	uniform bool have_ambient;\n\
	uniform bool have_diffuse;\n\
	uniform bool have_specular;\n\
	void main() {\n\
		vec4 ambientComp = ambient_color * ambient; \n\
		vec4 diffuseComp = dot(vert_Normal, normalize(dir_light)) * diffuse * diffuse_color; \n\
		vec4 viewDir = viewPos - fragPos;\n\
		vec4 reflectDir = reflect(normalize(-dir_light), vert_Normal);\n\
		vec4 specularComp = pow(max(dot(normalize(viewDir), reflectDir),0.0),shininess) * specular * specular_color ;\n\
		if(have_ambient && have_diffuse && have_specular)\n\
		{\n\
		out_Color = objColor * (ambientComp + diffuseComp + specularComp);\n\
		}\n\
		else if(have_ambient && have_diffuse)\n\
		{\n\
		out_Color = objColor * (ambientComp + diffuseComp);\n\
		}\n\
		else if(have_ambient && have_specular)\n\
		{\n\
		out_Color = objColor * (ambientComp + specularComp);\n\
		}\n\
		else if(have_diffuse && have_specular)\n\
		{\n\
		out_Color = objColor * (diffuseComp + specularComp);\n\
		}\n\
		else if(have_ambient)\n\
		{\n\
		out_Color = objColor * (ambientComp);\n\
		}\n\
		else if(have_diffuse)\n\
		{\n\
		out_Color = objColor * (diffuseComp);\n\
		}\n\
		else if(have_specular)\n\
		{\n\
		out_Color = objColor * (specularComp);\n\
		}\n\
		else\n\
		{\n\
		out_Color = objColor;\n\
		}\n\
	}";

	void setupCube() {
		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);
		glGenBuffers(3, cubeVbo);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNorms), cubeNorms, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glPrimitiveRestartIndex(UCHAR_MAX);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cubeShaders[0] = compileShader(cube_vertShader, GL_VERTEX_SHADER, "cubeVert");
		cubeShaders[1] = compileShader(cube_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		cubeProgram = glCreateProgram();
		glAttachShader(cubeProgram, cubeShaders[0]);
		glAttachShader(cubeProgram, cubeShaders[1]);
		glBindAttribLocation(cubeProgram, 0, "in_Position");
		glBindAttribLocation(cubeProgram, 1, "in_Normal");
		linkProgram(cubeProgram);
	}
	void cleanupCube() {
		glDeleteBuffers(3, cubeVbo);
		glDeleteVertexArrays(1, &cubeVao);

		glDeleteProgram(cubeProgram);
		glDeleteShader(cubeShaders[0]);
		glDeleteShader(cubeShaders[1]);
	}
	void updateCube(const glm::mat4& transform) {
		objMat = transform;
	}
	void DrawScenario() {
		glEnable(GL_PRIMITIVE_RESTART);
		glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(0.0f, -0.6f, 0.0f));
		glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(20.0f, 1.0f, 20.0f));
		objMat = t * s;

		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

		//glUniform4f(glGetUniformLocation(cubeProgram, "objColor"), simple.rgb[0], scenario.rgb[1], scenario.rgb[2], 1.f);
		//glUniform4f(glGetUniformLocation(cubeProgram, "ambient_color"), wLight.ambient_color[0], wLight.ambient_color[1], wLight.ambient_color[2], 1.f);
		//glUniform4f(glGetUniformLocation(cubeProgram, "diffuse_color"), wLight.diffuse_color[0], wLight.diffuse_color[1], wLight.diffuse_color[2], 1.f);
		//glUniform4f(glGetUniformLocation(cubeProgram, "specular_color"), wLight.specular_color[0], wLight.specular_color[1], wLight.specular_color[2], 1.f);
		//
		//glUniform4f(glGetUniformLocation(cubeProgram, "ambient"), wLight.ambient, wLight.ambient, wLight.ambient, 1.f);
		//glUniform4f(glGetUniformLocation(cubeProgram, "diffuse"), wLight.diffuse, wLight.diffuse, wLight.diffuse, 1.f);
		//glUniform4f(glGetUniformLocation(cubeProgram, "specular"), wLight.specular, wLight.specular, wLight.specular, 1.f);
		//
		//glUniform4f(glGetUniformLocation(cubeProgram, "viewPos"), wPos.x, wPos.y, wPos.z, 1.f);
		//glUniform4f(glGetUniformLocation(cubeProgram, "dir_light"), wLight.lightPos[0], wLight.lightPos[1], wLight.lightPos[2], 1.f);
		//glUniform1f(glGetUniformLocation(cubeProgram, "shininess"), wLight.shininess);
		//
		//glUniform1i(glGetUniformLocation(cubeProgram, "have_ambient"), scenario.haveAmbient);
		//glUniform1i(glGetUniformLocation(cubeProgram, "have_diffuse"), scenario.haveDiffuse);
		//glUniform1i(glGetUniformLocation(cubeProgram, "have_specular"), scenario.haveSpecular);

		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		t = glm::translate(glm::mat4(), glm::vec3(3.0f, 5.0f, 3.0f));
		objMat = t;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		t = glm::translate(glm::mat4(), glm::vec3(3.0f, 5.0f, 8.0f));
		s = glm::scale(glm::mat4(), glm::vec3(1.0f, 10.0f, 1.0f));
		objMat = t * s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		t = glm::translate(glm::mat4(), glm::vec3(4.0f, 2.0f, 8.0f));
		s = glm::scale(glm::mat4(), glm::vec3(1.0f, 6.0f, 1.0f));
		objMat = t * s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		t = glm::translate(glm::mat4(), glm::vec3(4.0f, 2.0f, 7.0f));
		s = glm::scale(glm::mat4(), glm::vec3(1.0f, 6.0f, 1.0f));
		objMat = t * s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		t = glm::translate(glm::mat4(), glm::vec3(-7.0f, 1.0f, -7.0f));
		s = glm::scale(glm::mat4(), glm::vec3(2.0f, 2.0f, 5.0f));
		objMat = t * s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		for (size_t i = 0; i < 5; i++)
		{
			t = glm::translate(glm::mat4(), glm::vec3(i * 3, 2.0f, -10.0f));
			objMat = t;
			glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
			glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		}

		glUseProgram(0);
		glBindVertexArray(0);

		glDisable(GL_PRIMITIVE_RESTART);
	}
}

#pragma endregion

// DOLLY FUNCTION
void MoveCamera()
{
	//ACTIVATE DOLLY EFFECT
	if (moveCamera)
	{
		float distance = RV::panv[2] - billboard->obj.pos.z;
		RV::FOV = 2.f * glm::atan(0.5f * width / glm::abs(distance));
	}
	//RESET FOV
	else
	{
		RV::FOV = glm::radians(radians);
	}
}

void InitModels()
{
	srand(time(nullptr));
	randomX = glm::linearRand(1.f, 10.f);
	randomY = glm::linearRand(1.f, 10.f);
	randomZ = glm::linearRand(1.f, 10.f);
	std::cout << "Random X: " << randomX << std::endl;
	std::cout << "Random Y: " << randomY << std::endl;
	std::cout << "Random Z: " << randomZ << std::endl;
	
	random = glm::vec3(randomX, randomY, randomZ);

	// BILLBOARD
	billboard = new Model(Shader("res/files/vert.vs", "res/files/frag_billboard.fs", "res/files/geo_billboard.gs"), "res/cube.obj",
		ObjectParameters(glm::vec3(1.f, 4.f, -15.f), glm::vec4(1.f, 1.f, 1.f, 1.f), true, true, false),
		Texture("res/tree.png", Texture::ETextureType::PNG));
	
	billboard->vertices.clear();
	billboard->normals.clear();
	billboard->uvs.clear();
	billboard->vertices.push_back(glm::vec3(0.0, 0.0, 0.0));
	billboard->normals.push_back(glm::vec3(0.0, 0.0, 0.0));
	billboard->uvs.push_back(glm::vec3(0.0, 0.0, 0.0));

	// SWORD
	sword = new Model(Shader("res/files/vert.vs", "res/files/frag.fs", "res/files/geo_explode.gs"), "res/espada.obj",
		ObjectParameters(glm::vec3(-7.f, 2.f, -7.f), glm::vec4(0.4f, 1.f, 1.f, 1.f), true, true, false),
		Texture("res/brick.jpg", Texture::ETextureType::JPG));

	// SCENARIO
	scenario = new Model(Shader("res/files/vert.vs", "res/files/frag.fs", "res/files/geo.gs"), "res/cube.obj",
		ObjectParameters(glm::vec3(0.f, -1.f, 0.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f), true, true, false),
		Texture("res/brick.jpg", Texture::ETextureType::JPG));
}

// RENDER MODELS
void RenderModels()
{
	float currentTime = ImGui::GetTime();	// GETTING TIME
	glm::mat4 t; // matrix for rotation
	glm::mat4 s; // matrix for scale

	// SWORD
#pragma region Sword

	sword->shader.Use();

	t = glm::translate(glm::mat4(), glm::vec3(sword->obj.pos));
	s = glm::scale(glm::mat4(), glm::vec3(0.2f, 0.2f, 0.2f));

	sword->objMat = t * s;

	sword->shader.SetMatrix("objMat", 1, GL_FALSE, glm::value_ptr(sword->objMat));
	sword->shader.SetMatrix("mv_Mat", 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
	sword->shader.SetMatrix("mvpMat", 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

	sword->shader.SetFloat("objColor", sword->obj.color.x, sword->obj.color.y, sword->obj.color.z, sword->obj.color.w);

	// PHONG UNIFORMS
	sword->shader.SetFloat("dir_light", phong.pos.x, phong.pos.y, phong.pos.z, 1.f);
	sword->shader.SetFloat("ambient_color", phong.ambient_color.x, phong.ambient_color.y, phong.ambient_color.z, phong.ambient_color.w);
	sword->shader.SetFloat("diffuse_color", phong.diffuse_color.x, phong.diffuse_color.y, phong.diffuse_color.z, phong.diffuse_color.w);
	sword->shader.SetFloat("specular_color", phong.specular_color.x, phong.specular_color.y, phong.specular_color.z, phong.specular_color.w);

	if (sword->obj.haveAmbient) sword->shader.SetFloat("ambient_strength", phong.ambient_strength, phong.ambient_strength, phong.ambient_strength, 1.f);
	else sword->shader.SetFloat("ambient_strength", phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, 1.f);
	if (sword->obj.haveDiffuse) sword->shader.SetFloat("diffuse_strength", phong.diffuse_strength, phong.diffuse_strength, phong.diffuse_strength, 1.f);
	else sword->shader.SetFloat("diffuse_strength", phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, 1.f);
	if (sword->obj.haveSpecular) sword->shader.SetFloat("specular_strength", phong.specular_strength, phong.specular_strength, phong.specular_strength, 1.f);
	else sword->shader.SetFloat("specular_strength", phong.specular_strength * 0.f, phong.specular_strength * 0.f, phong.specular_strength * 0.f, 1.f);
	
	sword->shader.SetFloat("shininess", phong.shininess);

	sword->shader.SetFloat("time", currentTime);
	sword->shader.SetFloat("random", random.x,random.y,random.z);
	
	sword->shader.SetInt("isMatrix", isMatrix);
	sword->shader.SetFloat("displaceX", displaceX);
	sword->shader.SetFloat("displaceY", displaceY);

	sword->texture.Active();
	sword->shader.SetInt("ourTexture", 0);

	sword->DrawTriangles();

#pragma endregion

	// SCENARIO
#pragma region Scenario

	scenario->shader.Use();

	t = glm::translate(glm::mat4(), glm::vec3(scenario->obj.pos));
	s = glm::scale(glm::mat4(), glm::vec3(20.0f, 1.0f, 20.0f));
	scenario->objMat = t * s;

	scenario->shader.SetMatrix("objMat", 1, GL_FALSE, glm::value_ptr(scenario->objMat));
	scenario->shader.SetMatrix("mv_Mat", 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
	scenario->shader.SetMatrix("mvpMat", 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

	scenario->shader.SetFloat("objColor", scenario->obj.color.x, scenario->obj.color.y, scenario->obj.color.z, scenario->obj.color.w);

	scenario->shader.SetFloat("dir_light", phong.pos.x, phong.pos.y, phong.pos.z, 1.f);
	scenario->shader.SetFloat("ambient_color", phong.ambient_color.x, phong.ambient_color.y, phong.ambient_color.z, phong.ambient_color.w);
	scenario->shader.SetFloat("diffuse_color", phong.diffuse_color.x, phong.diffuse_color.y, phong.diffuse_color.z, phong.diffuse_color.w);
	scenario->shader.SetFloat("specular_color", phong.specular_color.x, phong.specular_color.y, phong.specular_color.z, phong.specular_color.w);

	if (scenario->obj.haveAmbient) scenario->shader.SetFloat("ambient_strength", phong.ambient_strength, phong.ambient_strength, phong.ambient_strength, 1.f);
	else scenario->shader.SetFloat("ambient_strength", phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, 1.f);
	if (scenario->obj.haveDiffuse) scenario->shader.SetFloat("diffuse_strength", phong.diffuse_strength, phong.diffuse_strength, phong.diffuse_strength, 1.f);
	else scenario->shader.SetFloat("diffuse_strength", phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, 1.f);
	if (scenario->obj.haveSpecular) scenario->shader.SetFloat("specular_strength", phong.specular_strength, phong.specular_strength, phong.specular_strength, 1.f);
	else scenario->shader.SetFloat("specular_strength", phong.specular_strength * 0.f, phong.specular_strength * 0.f, phong.specular_strength * 0.f, 1.f);
	
	scenario->shader.SetFloat("shininess", phong.shininess);

	scenario->shader.SetFloat("viewPos", wPos.x, wPos.y, wPos.z, 1.f);

	scenario->shader.SetInt("isMatrix", isMatrix);
	scenario->shader.SetFloat("displaceX", displaceX);
	scenario->shader.SetFloat("displaceY", displaceY);

	scenario->texture.Active();
	scenario->shader.SetInt("ourTexture", 0);

	scenario->DrawTriangles();

#pragma endregion

	// SIMPLE CUBE
#pragma region Simple Cube
	billboard->shader.Use();

	// Translate position
	t = glm::translate(glm::mat4(), glm::vec3(billboard->obj.pos));
	billboard->objMat = t;

	billboard->shader.SetMatrix("objMat", 1, GL_FALSE, glm::value_ptr(billboard->objMat));
	billboard->shader.SetMatrix("mv_Mat", 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
	billboard->shader.SetMatrix("mvpMat", 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

	billboard->shader.SetFloat("objColor", billboard->obj.color.x, billboard->obj.color.y, billboard->obj.color.z, billboard->obj.color.w);
	
	billboard->shader.SetFloat("dir_light", phong.pos.x, phong.pos.y, phong.pos.z, 1.f);
	billboard->shader.SetFloat("ambient_color", phong.ambient_color.x, phong.ambient_color.y, phong.ambient_color.z, phong.ambient_color.w);
	billboard->shader.SetFloat("diffuse_color", phong.diffuse_color.x, phong.diffuse_color.y, phong.diffuse_color.z, phong.diffuse_color.w);
	billboard->shader.SetFloat("specular_color", phong.specular_color.x, phong.specular_color.y, phong.specular_color.z, phong.specular_color.w);
	
	if (billboard->obj.haveAmbient) billboard->shader.SetFloat("ambient_strength", phong.ambient_strength, phong.ambient_strength, phong.ambient_strength, 1.f);
	else billboard->shader.SetFloat("ambient_strength", phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, 1.f);
	if (billboard->obj.haveDiffuse) billboard->shader.SetFloat("diffuse_strength", phong.diffuse_strength, phong.diffuse_strength, phong.diffuse_strength, 1.f);
	else billboard->shader.SetFloat("diffuse_strength", phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, 1.f);
	if (billboard->obj.haveSpecular) billboard->shader.SetFloat("specular_strength", phong.specular_strength, phong.specular_strength, phong.specular_strength, 1.f);
	else billboard->shader.SetFloat("specular_strength", phong.specular_strength * 0.f, phong.specular_strength * 0.f, phong.specular_strength * 0.f, 1.f);

	billboard->shader.SetFloat("shininess", phong.shininess);

	billboard->shader.SetFloat("viewPos", wPos.x, wPos.y, wPos.z, 1.f);

	billboard->shader.SetInt("isMatrix", isMatrix);
	billboard->shader.SetFloat("displaceX", displaceX);
	billboard->shader.SetFloat("displaceY", displaceY);
	
	moveWTime = cos(currentTime);
	billboard->shader.SetFloat("moveWTime", moveWTime);

	billboard->shader.SetFloat("time", currentTime);
	billboard->shader.SetFloat("random", random.x,random.y,random.z);

	billboard->texture.Active();
	billboard->shader.SetInt("ourTexture", 0);
	
	billboard->DrawPoints();

#pragma endregion
}

void GLinit(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);

	// Setup shaders & geometry
	Axis::setupAxis();
	
	InitModels();
}
void GLcleanup() {
	Axis::cleanupAxis();

	sword->texture.Clean();
	scenario->texture.Clean();
	billboard->texture.Clean();
}

void GLrender(float dt) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RV::_projection = glm::perspective(RV::FOV, (float)800.f / (float)600.f, RV::zNear, RV::zFar);

	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));
	
	RV::_MVP = RV::_projection * RV::_modelView;
	
	// Get camera position
	wPos = glm::vec4(0.f, 0.f, 0.f, 1.f);
	glm::mat4 view_inv = glm::inverse(RV::_modelView);
	wPos = view_inv * wPos;
	MoveCamera();	// dolly effect

	Axis::drawAxis();
	
	RenderModels();
	ImGui::Render();
}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		/////////////////////////////////////////////////////////
		
		if (ImGui::TreeNode("Phong Shading"))
		{
			ImGui::SliderFloat3("Light Position", glm::value_ptr(phong.pos), -1.f, 1.f);

			ImGui::ColorEdit4("Ambient Color", glm::value_ptr(phong.ambient_color));
			ImGui::ColorEdit4("Diffuse Color", glm::value_ptr(phong.diffuse_color));
			ImGui::ColorEdit4("Specular Color", glm::value_ptr(phong.specular_color));

			ImGui::SliderFloat("Ambient Strength", &phong.ambient_strength, 0.0f, 1.0f);
			ImGui::SliderFloat("Diffuse Strength", &phong.diffuse_strength, 0.0f, 1.0f);
			ImGui::SliderFloat("Specular Strength", &phong.specular_strength, 0.0f, 1.0f);
			ImGui::SliderFloat("Shininess", &phong.shininess, 0, 255);
			
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Scenario"))
		{
			ImGui::DragFloat3("Translate", glm::value_ptr(scenario->obj.pos), 0.f, 1.f);
			ImGui::ColorEdit4("Object Color", glm::value_ptr(scenario->obj.color));
			ImGui::Checkbox("Active ambient", &scenario->obj.haveAmbient);
			ImGui::Checkbox("Active diffuse", &scenario->obj.haveDiffuse);
			ImGui::Checkbox("Active specular", &scenario->obj.haveSpecular);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Simple Cube"))
		{
			ImGui::DragFloat3("Translate", glm::value_ptr(billboard->obj.pos), 0.f, 1.f);
			ImGui::ColorEdit4("Object Color", glm::value_ptr(billboard->obj.color));
			ImGui::Checkbox("Active ambient", &billboard->obj.haveAmbient);
			ImGui::Checkbox("Active diffuse", &billboard->obj.haveDiffuse);
			ImGui::Checkbox("Active specular", &billboard->obj.haveSpecular);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Sword"))
		{
			ImGui::DragFloat3("Translate", glm::value_ptr(sword->obj.pos), 0.f, 1.f);
			ImGui::ColorEdit4("Object Color", glm::value_ptr(sword->obj.color));
			ImGui::Checkbox("Active ambient", &sword->obj.haveAmbient);
			ImGui::Checkbox("Active diffuse", &sword->obj.haveDiffuse);
			ImGui::Checkbox("Active specular", &sword->obj.haveSpecular);

			ImGui::TreePop();
		}

		ImGui::Checkbox("Dolly Effect", &moveCamera);
		if (moveCamera)
		{
			ImGui::SliderFloat("Width", &width, 5, 30);
			ImGui::SliderFloat("PosZ", &RV::panv[2], -20, -6);
		}
		
		ImGui::Checkbox("Discard Effect", &isMatrix);
		if (isMatrix)
		{
			ImGui::DragFloat("Displace in X", &displaceX, 0.1f, 0.0f,50.f);
			ImGui::DragFloat("Displace in Y", &displaceY, 0.1f, 0.0f,50.f);
		}

		/////////////////////////////////////////////////////////
	}
	// .........................
	
	ImGui::End();
	
	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	bool show_test_window = false;
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}