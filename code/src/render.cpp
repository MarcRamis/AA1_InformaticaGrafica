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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// VARIABLES
#pragma region Variables

glm::vec4 wPos;			// Camera pos
float moveWTime = 0.0f; // move in world time

// Random explode
float randomX, randomY, randomZ;
glm::vec3 random;

// DISCARD EFFECT
bool isMatrix = false;
float displaceX = 5.0f;
float displaceY = 5.0f;

// MODELS
Model *simpleCube;
Model *simpleCubeOutline;
Model *floorCube;
Model *skyBoxCube;
Model *car;
Model *carStencil;

CubeMap *skyBox;

// PHONG SHADER
Light phong = Light(glm::vec4(0.f,0.f,1.f, 1.f), 
	glm::vec4(1.f,1.f,1.f,1.f), glm::vec4(1.f, 1.f, 1.f,1.f), glm::vec4(1.f, 0.f, 0.f,1.f),
	0.5f,0.5f,0.5f,1.f);

// CAR INSTANCING --> 10
glm::mat4 objMatCar[10];
glm::vec3 objPosCar[10];
glm::vec3 randomPosition[10];

#pragma endregion

// TEACHER UTILS
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
	const float FOV = glm::radians(65.f);
	const float zNear = 0.01f;
	const float zFar = 100.f;

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

// FRAMEBUFFER
#pragma region Framebuffer

namespace FB {

	unsigned int fbo;
	Texture fbo_Tex;
	
	void Init()
	{
		// INIT FBO TEXTURE
		glGenFramebuffers(1, &fbo);
		// CREATE TEXTURE AS WE CREATE IT IN TEXTURE
		fbo_Tex = Texture("res/white.jpg", Texture::ETextureType::NONE);
		// BIND TEXTURE
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_Tex.id, 0);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void Draw(glm::mat4 _MVP, glm::mat4 _ModelView, glm::mat4 _projection)
	{
		//we store the current values in a temporary variable
		glm::mat4 t_mvp = _MVP;
		glm::mat4 t_mv = _ModelView;

		// we set up our framebuffer and draw into it
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		//glClearColor(1.f, 1.f, 1.f, 1.f);
		glViewport(0, 0, 800, 800);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClear(GL_COLOR_BUFFER_BIT);
		//glEnable(GL_DEPTH_TEST);
		_MVP = _projection;
		_ModelView = glm::mat4(1.f);

		//we restore the previous conditions
		_MVP = t_mvp;
		_ModelView = t_mv;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//we set up a texture where to draw our FBO:
		glViewport(0, 0, fbo_Tex.width, fbo_Tex.height);
		glBindTexture(GL_TEXTURE_2D, fbo_Tex.id);
	}	
}

#pragma endregion

// STENCIL BUFFER
#pragma region StencilBuffer

namespace StencilBuffer{
	void EnableStencil()
	{
		glEnable(GL_STENCIL_TEST);
		glClear(GL_STENCIL_BUFFER_BIT); // Clear color &  stencil buffer (0 by default)
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	}
	void On()
	{
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
	}
	void Off()
	{
		glStencilMask(0x00);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	}

	void EnableDepth()
	{
		glEnable(GL_DEPTH_TEST);
	}
	void DisableDepth()
	{
		glDisable(GL_DEPTH_TEST);
	}
}

#pragma endregion

#pragma region AA3

bool DifferenceBetweenTwoPoints(glm::vec3 pos1, glm::vec3 pos2, float diff)
{
	float xDifference = pos1.x - pos2.x;
	float yDifference = pos1.y - pos2.y;
	float zDifference = pos1.z - pos2.z;
	float distance = glm::sqrt(glm::pow(xDifference, 2) - glm::pow(yDifference,2) - glm::pow(zDifference, 2));

	if (distance < diff)
	{
		return true;
	}
}
glm::vec3 GetRandomPositionXZ(float min, float max)
{
	return glm::vec3(glm::linearRand(min, max), 0.f, glm::linearRand(min, max));
}
void UpdatePosition(glm::vec3& pos, glm::vec3& newPos)
{
	pos += 0.01f * newPos;
}

void MoveCar()
{
	glm::mat4 t, s;

	for (int i = 0; i < 10; i++)
	{
		UpdatePosition(objPosCar[i], randomPosition[i]);

		if (DifferenceBetweenTwoPoints(objPosCar[i], randomPosition[i], 0.1f))
		{
			randomPosition[i] = GetRandomPositionXZ(-10.f, 10.f);
		}

		t = glm::translate(glm::mat4(), glm::vec3(objPosCar[i]));
		s = glm::scale(glm::mat4(), glm::vec3(0.02f, 0.02f, 0.02f));

		objMatCar[i] = t * s;
	}
}

void InitModels()
{
	srand(time(nullptr));
	
	// Init CubeMap
	std::vector<std::string> faces
	{
			"res/skybox/right.jpg",
			"res/skybox/left.jpg",
			"res/skybox/top.jpg",
			"res/skybox/bottom.jpg",
			"res/skybox/front.jpg",
			"res/skybox/back.jpg"
	};
	skyBox = new CubeMap(faces);
	
	skyBoxCube = new Model(Shader("res/files/vert_cubemap.vs", "res/files/frag_cubemap.fs", nullptr), "res/cube.obj",
		ObjectParameters(glm::vec3(0.f, 0.f, 0.f), glm::vec4(1.f, 1.f, 1.f, 1.f), false, false, false),
		Texture(nullptr, Texture::ETextureType::NONE));
	
	// Init Models
	simpleCube = new Model(Shader("res/files/vert.vs", "res/files/frag.fs", "res/files/geo.gs"), "res/cube.obj",
		ObjectParameters(glm::vec3(0.f, 0.f, 0.f), glm::vec4(1.f, 1.f, 1.f, 1.f), true, true, false),
		Texture("res/iron.jpg", Texture::ETextureType::JPG));
	
	simpleCubeOutline = new Model(Shader("res/files/vert.vs", "res/files/frag_only_color.fs", "res/files/geo.gs"), "res/cube.obj",
		ObjectParameters(glm::vec3(0.f, 0.f, 0.f), glm::vec4(0.4f, 1.f, 1.f, 1.f), true, true, false),
		Texture("res/iron.jpg", Texture::ETextureType::JPG));
	
	floorCube = new Model(Shader("res/files/vert.vs", "res/files/frag.fs", "res/files/geo.gs"), "res/cube.obj",
		ObjectParameters(glm::vec3(0.f, -1.f, 0.f), glm::vec4(1.f, 1.f, 1.f, 1.f), true, true, false),
		Texture("res/iron2.jpg", Texture::ETextureType::JPG));
	
	// CAR INICIALIZATION
	stbi_set_flip_vertically_on_load(true);
	car = new Model(Shader("res/files/vert_instancing_car.vs", "res/files/frag_car_nowindow.fs", "res/files/geo.gs"), "res/Camaro.obj",
		ObjectParameters(glm::vec3(0.f, 0.f, 0.f), glm::vec4(1.f, 1.f, 1.f, 1.f), true, true, false),
		Texture("res/Camaro/Camaro_AlbedoTransparency_alt.png", Texture::ETextureType::PNG));
	stbi_set_flip_vertically_on_load(false);

	glm::mat4 t, s;
	for (int i = 0; i < 10; i++)
	{
		randomPosition[i] = GetRandomPositionXZ(-10.f, 10.f);;
		objPosCar[i] = glm::vec3(glm::linearRand(-10.f, 10.f), 0.f, glm::linearRand(-10.f, 10.f));
		t = glm::translate(glm::mat4(), glm::vec3(objPosCar[i]));
		s = glm::scale(glm::mat4(), glm::vec3(0.02f, 0.02f, 0.02f));
		objMatCar[i] = t * s;
	}
	carStencil = new Model(Shader("res/files/vert_instancing_car.vs", "res/files/frag_only_color.fs", "res/files/geo.gs"), "res/Camaro.obj",
		ObjectParameters(glm::vec3(0.f, 0.f, 0.f), glm::vec4(1.f, 1.f, 1.f, 0.5f), true, true, false),
		Texture("res/Camaro/Camaro_AlbedoTransparency_alt.png", Texture::ETextureType::PNG));

}
void SetValuesSkyBox(Model *model, unsigned int id)
{
	model->shader.Use();
	
	model->shader.SetMatrix("objMat", 1, GL_FALSE, glm::value_ptr(model->objMat));
	model->shader.SetMatrix("mvpMat", 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

	model->shader.SetFloat("skybox", id);

	model->DrawTriangles();
}
void SetValues(Model *model, glm::mat4 _t, glm::mat4 _s)
{
	float currentTime = ImGui::GetTime();

	model->shader.Use();
	
	model->objMat = _t * _s;
	
	model->shader.SetMatrix("objMat", 1, GL_FALSE, glm::value_ptr(model->objMat));
	model->shader.SetMatrix("mv_Mat", 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
	model->shader.SetMatrix("mvpMat", 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

	model->shader.SetFloat("objColor", model->obj.color.x, model->obj.color.y, model->obj.color.z, model->obj.color.w);
	
	model->shader.SetFloat("dir_light", phong.pos.x, phong.pos.y, phong.pos.z, 1.f);
	model->shader.SetFloat("ambient_color", phong.ambient_color.x, phong.ambient_color.y, phong.ambient_color.z, phong.ambient_color.w);
	model->shader.SetFloat("diffuse_color", phong.diffuse_color.x, phong.diffuse_color.y, phong.diffuse_color.z, phong.diffuse_color.w);
	model->shader.SetFloat("specular_color", phong.specular_color.x, phong.specular_color.y, phong.specular_color.z, phong.specular_color.w);

	if (model->obj.haveAmbient) model->shader.SetFloat("ambient_strength", phong.ambient_strength, phong.ambient_strength, phong.ambient_strength, 1.f);
	else model->shader.SetFloat("ambient_strength", phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, 1.f);
	if (model->obj.haveDiffuse) model->shader.SetFloat("diffuse_strength", phong.diffuse_strength, phong.diffuse_strength, phong.diffuse_strength, 1.f);
	else model->shader.SetFloat("diffuse_strength", phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, 1.f);
	if (model->obj.haveSpecular) model->shader.SetFloat("specular_strength", phong.specular_strength, phong.specular_strength, phong.specular_strength, 1.f);
	else model->shader.SetFloat("specular_strength", phong.specular_strength * 0.f, phong.specular_strength * 0.f, phong.specular_strength * 0.f, 1.f);

	model->shader.SetFloat("shininess", phong.shininess);

	model->shader.SetFloat("viewPos", wPos.x, wPos.y, wPos.z, 1.f);

	model->shader.SetFloat("time", currentTime);
	model->shader.SetFloat("random", random.x, random.y, random.z);

	model->shader.SetInt("isMatrix", isMatrix);
	
	if (isMatrix) {
		model->shader.SetFloat("displaceX", displaceX);
		model->shader.SetFloat("displaceY", displaceY);
	}
	else {
		model->shader.SetFloat("displaceX", 0.f);
		model->shader.SetFloat("displaceY", 0.f);
	}
	model->texture.Active();
	model->shader.SetInt("ourTexture", 0);

	//FB::Draw(RenderVars::_MVP, RenderVars::_modelView, RenderVars::_projection);
}
void SetValuesInstanced(Model* model, unsigned int instancesToDraw, glm::mat4 objMat[])
{
	float currentTime = ImGui::GetTime();

	model->shader.Use();
	
	for (int i = 0; i < instancesToDraw; i++)
	{
		model->shader.SetMatrix("objMat[" + std::to_string(i) + "]", 1, GL_FALSE, glm::value_ptr(objMat[i]));
	}

	model->shader.SetMatrix("mv_Mat", 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
	model->shader.SetMatrix("mvpMat", 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

	model->shader.SetFloat("objColor", model->obj.color.x, model->obj.color.y, model->obj.color.z, model->obj.color.w);

	model->shader.SetFloat("dir_light", phong.pos.x, phong.pos.y, phong.pos.z, 1.f);
	model->shader.SetFloat("ambient_color", phong.ambient_color.x, phong.ambient_color.y, phong.ambient_color.z, phong.ambient_color.w);
	model->shader.SetFloat("diffuse_color", phong.diffuse_color.x, phong.diffuse_color.y, phong.diffuse_color.z, phong.diffuse_color.w);
	model->shader.SetFloat("specular_color", phong.specular_color.x, phong.specular_color.y, phong.specular_color.z, phong.specular_color.w);

	if (model->obj.haveAmbient) model->shader.SetFloat("ambient_strength", phong.ambient_strength, phong.ambient_strength, phong.ambient_strength, 1.f);
	else model->shader.SetFloat("ambient_strength", phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, phong.ambient_strength * 0.f, 1.f);
	if (model->obj.haveDiffuse) model->shader.SetFloat("diffuse_strength", phong.diffuse_strength, phong.diffuse_strength, phong.diffuse_strength, 1.f);
	else model->shader.SetFloat("diffuse_strength", phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, phong.diffuse_strength * 0.f, 1.f);
	if (model->obj.haveSpecular) model->shader.SetFloat("specular_strength", phong.specular_strength, phong.specular_strength, phong.specular_strength, 1.f);
	else model->shader.SetFloat("specular_strength", phong.specular_strength * 0.f, phong.specular_strength * 0.f, phong.specular_strength * 0.f, 1.f);

	model->shader.SetFloat("shininess", phong.shininess);

	model->shader.SetFloat("viewPos", wPos.x, wPos.y, wPos.z, 1.f);

	model->shader.SetFloat("time", currentTime);
	model->shader.SetFloat("random", random.x, random.y, random.z);

	model->texture.Active();
	model->shader.SetInt("ourTexture", 0);
}

void RenderModels()
{
	glm::mat4 t, s;
	
	glDepthMask(GL_FALSE);
	t = glm::translate(glm::mat4(), glm::vec3(skyBoxCube->obj.pos));
	s = glm::scale(glm::mat4(), glm::vec3(50.f, 50.f, 50.f));
	skyBoxCube->objMat = t * s;
	SetValuesSkyBox(skyBoxCube, skyBox->textureID);
	glDepthMask(GL_TRUE);
	
	// NO INSTANCING
	//t = glm::translate(glm::mat4(), glm::vec3(car->obj.pos));
	//s = glm::scale(glm::mat4(), glm::vec3(0.02f, 0.02f, 0.02f));
	//SetValues(car,t,s);
	//car->DrawTriangles();

	StencilBuffer::EnableStencil();
	StencilBuffer::Off();		// Here we draw all that doesn't contain an outline
	
	t = glm::translate(glm::mat4(), glm::vec3(floorCube->obj.pos));
	s = glm::scale(glm::mat4(), glm::vec3(100.f, 1.0f, 100.f));
	SetValues(floorCube, t, s);
	floorCube->DrawTriangles();
	
	StencilBuffer::On();	// Here we draw all that will contain an outline
	
	MoveCar();
	SetValuesInstanced(car, 10, objMatCar);
	car->DrawTrianglesInstanced(10);

	StencilBuffer::Off();
	StencilBuffer::DisableDepth();	// Here we draw the outline of the same object.
									// It must be bigger than the object

	SetValuesInstanced(carStencil, 10, objMatCar);
	carStencil->DrawTrianglesInstanced(10);
	
	StencilBuffer::On();	// Set on because if not, it will draw the object in the screen
	StencilBuffer::EnableDepth();
}

#pragma endregion

// RENDER STATE
void GLinit(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);

	// Setup shaders & geometry
	Axis::setupAxis();
	
	InitModels();
}
void GLcleanup() {
	Axis::cleanupAxis();	
}
void GLrender(float dt) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));
	
	RV::_MVP = RV::_projection * RV::_modelView;
	
	// Get camera position
	wPos = glm::vec4(0.f, 0.f, 0.f, 1.f);
	glm::mat4 view_inv = glm::inverse(RV::_modelView);
	wPos = view_inv * wPos;

	Axis::drawAxis();
	
	RenderModels();
	
	ImGui::Render();
}

// INTERFACE
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

		
		#pragma region OLD
		
#pragma endregion
		//
		//if (ImGui::TreeNode("Car"))
		//{
		//	ImGui::DragFloat3("Translate", glm::value_ptr(car->obj.pos), 0.f, 1.f);
		//	ImGui::ColorEdit4("Object Color", glm::value_ptr(car->obj.color));
		//	ImGui::Checkbox("Active ambient", &car->obj.haveAmbient);
		//	ImGui::Checkbox("Active diffuse", &car->obj.haveDiffuse);
		//	ImGui::Checkbox("Active specular", &car->obj.haveSpecular);
		//	
		//	ImGui::TreePop();
		//}
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