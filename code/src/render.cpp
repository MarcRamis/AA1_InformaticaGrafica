#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.h"

///////////////////// EXTERN FUNCTIONS
extern bool loadOBJ(const char*
	path,
	std::vector < glm::vec3 >&
	out_vertices,
	std::vector < glm::vec2 >& out_uvs,
	std::vector < glm::vec3 >&
	out_normals
);

extern void ReadFile(std::vector < glm::vec3 >& out_vertices,
	std::vector < glm::vec2 >& out_uvs,
	std::vector < glm::vec3 >& out_normals, std::string path);


///////////////////// DATA STRUCTURE
struct Object
{
	float pos[3];
	float rgb[3];
	bool haveAmbient, haveDiffuse, haveSpecular;
	
	Object(float posX, float posY, float posZ, float r, float g, float b, bool _hA, bool _hD, bool _hS)
	{
		pos[0] = posX;
		pos[1] = posY;
		pos[2] = posZ;
		
		rgb[0] = r;
		rgb[1] = g;
		rgb[2] = b;

		haveAmbient = _hA;
		haveDiffuse = _hD;
		haveSpecular = _hS;
	}
};
struct WLight
{
	float ambient_color[3];
	float diffuse_color[3];
	float specular_color[3];

	float ambient, diffuse, specular;
	float lightPos[3];

	float shininess;

	WLight()
	{
		ambient_color[0] = 1.f;	// R
		ambient_color[1] = 1.f;	// G
		ambient_color[2] = 1.f;	// B

		diffuse_color[0] = 1.f;	// R
		diffuse_color[1] = 1.f;	// G
		diffuse_color[2] = 1.f;	// B

		specular_color[0] = 1.f;
		specular_color[1] = 0.f;
		specular_color[2] = 0.f;

		ambient = 0.2f;
		diffuse = 0.3f;
		specular = 0.5f;

		lightPos[0] = 11.f;
		lightPos[1] = 11.f;
		lightPos[2] = 0.f;

		shininess = 30.f;
	};
};
///////////////////// VARIABLES
glm::vec4 wPos;
WLight wLight;
Object dragon(1.f, 1.f, 1.f, 0.4f, 1.f, 1.f, false, false, false);
Object sword(-7.f,2.f,-7.f,0.4f,1.f,1.f, true,  true, false);
Object scenario(0.f,0.f,0.f,0.5f,.5,0.5f, true, true, false);

float radians = 65.f;
bool moveCamera = false;
int widthCamera = 730;
int heightCamera = 730;
bool direccionCAM = false;
float width = 16.f;

float moveWTime = 0.0f;

Shader shader;

#pragma region PROFE

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

#pragma endregion

#pragma region OLD

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

		glUniform4f(glGetUniformLocation(cubeProgram, "objColor"), scenario.rgb[0], scenario.rgb[1], scenario.rgb[2], 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "ambient_color"), wLight.ambient_color[0], wLight.ambient_color[1], wLight.ambient_color[2], 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "diffuse_color"), wLight.diffuse_color[0], wLight.diffuse_color[1], wLight.diffuse_color[2], 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "specular_color"), wLight.specular_color[0], wLight.specular_color[1], wLight.specular_color[2], 1.f);

		glUniform4f(glGetUniformLocation(cubeProgram, "ambient"), wLight.ambient, wLight.ambient, wLight.ambient, 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "diffuse"), wLight.diffuse, wLight.diffuse, wLight.diffuse, 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "specular"), wLight.specular, wLight.specular, wLight.specular, 1.f);

		glUniform4f(glGetUniformLocation(cubeProgram, "viewPos"), wPos.x, wPos.y, wPos.z, 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "dir_light"), wLight.lightPos[0], wLight.lightPos[1], wLight.lightPos[2], 1.f);
		glUniform1f(glGetUniformLocation(cubeProgram, "shininess"), wLight.shininess);

		glUniform1i(glGetUniformLocation(cubeProgram, "have_ambient"), scenario.haveAmbient);
		glUniform1i(glGetUniformLocation(cubeProgram, "have_diffuse"), scenario.haveDiffuse);
		glUniform1i(glGetUniformLocation(cubeProgram, "have_specular"), scenario.haveSpecular);

		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		t = glm::translate(glm::mat4(), glm::vec3(3.0f, 5.0f, 3.0f));
		objMat = t ;
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
////////////////////////////////////////////////// LIGHT CUBE
namespace LightCube {
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
	out vec3 vert_wPos;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = objMat * vec4(in_Normal, 0.0);\n\
	}";

	const char* cube_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	out vec4 out_Color;\n\
	uniform mat4 mv_Mat;\n\
	uniform vec4 objColor;\n\
	uniform vec4 ambient_color;\n\
	uniform vec4 diffuse_color;\n\
	void main() {\n\
		out_Color = objColor * (ambient_color * diffuse_color);\n\
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

	void LightCube()
	{
		glEnable(GL_PRIMITIVE_RESTART);
		glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(wLight.lightPos[0], wLight.lightPos[1], wLight.lightPos[2]));
		objMat = t;

		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

		glUniform4f(glGetUniformLocation(cubeProgram, "ambient_color"), wLight.ambient_color[0], wLight.ambient_color[1], wLight.ambient_color[2], 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "diffuse_color"), wLight.diffuse_color[0], wLight.diffuse_color[1], wLight.diffuse_color[2], 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "objColor"), 1.f, 1.f, 1.f, 1.f);

		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);

		glDisable(GL_PRIMITIVE_RESTART);
	}
}
////////////////////////////////////////////////// MODEL DRAGON
namespace ModelDragon
{
	GLuint modelVao;
	GLuint modelVbo[4];
	GLuint modelShaders[3];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;

	int width, height, nrChannels;
	unsigned char* data;
	unsigned int texture;

	//GLubyte modelIdx[] = {	
	//0, 1, 2, 3, UCHAR_MAX,
	//4, 5, 6, 7, UCHAR_MAX,
	//8, 9, 10, 11, UCHAR_MAX,
	//12, 13, 14, 15, UCHAR_MAX,
	//16, 17, 18, 19, UCHAR_MAX,
	//20, 21, 22, 23, UCHAR_MAX
	//};

	const char* model_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
	}";
	
	const char* model_geomShader = 
		"#version 330\n\
		layout (triangles) in;\n\
		layout (triangle_strip, max_vertices = 6) out;\n\
		uniform float moveWTime;\n\
		void main()\n\
		{\n\
			for(int i = 0; i < 3; i++) { \n\
				vec4 offset = vec4(5.0,0.0,0.0,0.0);\n\
				gl_Position = gl_in[i].gl_Position + offset;\n\
				EmitVertex();\n\
			}\n\
			EndPrimitive();\n\
			for(int i = 0; i < 3; i++) { \n\
				gl_Position = gl_in[i].gl_Position + vec4(moveWTime, moveWTime, 0.0, 0.0); \n\
				EmitVertex(); \n\
			}\n\
			EndPrimitive(); \n\
	}";

	const char* model_fragShader =
		"#version 330\n\
	out vec4 out_Color;\n\
	uniform vec4 objColor;\n\
	void main() {\n\
		out_Color = objColor;\n\
	}";

	void Init() {
		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		// Vertices
		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// Normals
		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// Uvs
		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);
		
		//glPrimitiveRestartIndex(UCHAR_MAX);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelVbo[3]);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(modelIdx), modelIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "modelDragonVert");
		modelShaders[1] = compileShader(model_geomShader, GL_GEOMETRY_SHADER, "modelDragonGeom");
		modelShaders[2] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "modelDragonFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glAttachShader(modelProgram, modelShaders[2]);
		
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		glBindAttribLocation(modelProgram, 2, "in_TexCoord");
		
		linkProgram(modelProgram);
	}

	void InitTexture()
	{
		// CARGAR TEXTURA
		data = stbi_load("res/brick2.jpg", &width, &height, &nrChannels, 0);
		
		// CREAR TEXTURA	
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}

		// Parámetros de la imagen
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		//float borderColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void Clean() {
		glDeleteBuffers(3, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
		glDeleteShader(modelShaders[2]);

		stbi_image_free(data);
		glDeleteTextures(1, &texture);
	}

	void Render(float dt)
	{
		glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(0.f, 5.f, 0.0f));
		//glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(0.2f, 0.2f, 0.2f));
		objMat = t;
		
		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);

		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

		glUniform4f(glGetUniformLocation(modelProgram, "objColor"), dragon.rgb[0], dragon.rgb[1], dragon.rgb[2], 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "ambient_color"), wLight.ambient_color[0], wLight.ambient_color[1], wLight.ambient_color[2], 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "diffuse_color"), wLight.diffuse_color[0], wLight.diffuse_color[1], wLight.diffuse_color[2], 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "specular_color"), wLight.specular_color[0], wLight.specular_color[1], wLight.specular_color[2], 1.f);

		glUniform4f(glGetUniformLocation(modelProgram, "ambient"), wLight.ambient, wLight.ambient, wLight.ambient, 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "diffuse"), wLight.diffuse, wLight.diffuse, wLight.diffuse, 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "specular"), wLight.specular, wLight.specular, wLight.specular, 1.f);

		glUniform4f(glGetUniformLocation(modelProgram, "viewPos"), wPos.x, wPos.y, wPos.z, 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "dir_light"), wLight.lightPos[0], wLight.lightPos[1], wLight.lightPos[2], 1.f);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), wLight.shininess);
		
		moveWTime = cos(dt);
		glUniform1f(glGetUniformLocation(modelProgram, "moveWTime"), moveWTime);
		std::cout << moveWTime << std::endl;

		glUniform1i(glGetUniformLocation(modelProgram, "have_ambient"), dragon.haveAmbient);
		glUniform1i(glGetUniformLocation(modelProgram, "have_diffuse"), dragon.haveDiffuse);
		glUniform1i(glGetUniformLocation(modelProgram, "have_specular"), dragon.haveSpecular);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(modelProgram, "ourTexture"), 0);
		
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		glDrawElements(GL_TRIANGLE_STRIP, uvs.size(), GL_UNSIGNED_INT, 0);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}
///////////////////////////////////////////////// MODEL SWORD
namespace ModelSword
{
	GLuint modelVao;
	GLuint modelVbo[4];
	GLuint modelShaders[2];
	GLuint modelProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;

	//GLubyte modelIdx[] = {
	//0, 1, 2, 3, UCHAR_MAX,
	//4, 5, 6, 7, UCHAR_MAX,
	//8, 9, 10, 11, UCHAR_MAX,
	//12, 13, 14, 15, UCHAR_MAX,
	//16, 17, 18, 19, UCHAR_MAX,
	//20, 21, 22, 23, UCHAR_MAX
	//};

	const char* model_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	out vec4 vert_Normal;\n\
	out vec4 fragPos;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0); \n\
		mat4 normalMat; \n\ = transpose(inverse(objMat));\n\
		vert_Normal = normalMat * vec4(in_Normal, 0.0);\n\
		fragPos = vec4(objMat * vec4(in_Position, 1.0));\n\
	}";

	const char* model_fragShader =
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

	void Init() {

		glGenVertexArrays(1, &modelVao);
		glBindVertexArray(modelVao);
		glGenBuffers(3, modelVbo);

		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		//glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs.data(), GL_STATIC_DRAW);
		//glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//glEnableVertexAttribArray(1);
		
		glBindBuffer(GL_ARRAY_BUFFER, modelVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		//glPrimitiveRestartIndex(UCHAR_MAX);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelVbo[3]);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(modelIdx), modelIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}

	void Clean() {
		glDeleteBuffers(3, modelVbo);
		glDeleteVertexArrays(1, &modelVao);

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}

	void Render(float dt)
	{
		glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(cos(sword.pos[0] * dt), sword.pos[1], sword.pos[2]));
		glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(0.2f, 0.2f, 0.2f));
		objMat = t * s;
		
		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);

		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

		glUniform4f(glGetUniformLocation(modelProgram, "objColor"), sword.rgb[0], sword.rgb[1], sword.rgb[2], 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "ambient_color"), wLight.ambient_color[0], wLight.ambient_color[1], wLight.ambient_color[2], 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "diffuse_color"), wLight.diffuse_color[0], wLight.diffuse_color[1], wLight.diffuse_color[2], 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "specular_color"), wLight.specular_color[0], wLight.specular_color[1], wLight.specular_color[2], 1.f);

		glUniform4f(glGetUniformLocation(modelProgram, "ambient"), wLight.ambient, wLight.ambient, wLight.ambient, 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "diffuse"), wLight.diffuse, wLight.diffuse, wLight.diffuse, 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "specular"), wLight.specular, wLight.specular, wLight.specular, 1.f);

		glUniform4f(glGetUniformLocation(modelProgram, "viewPos"), wPos.x, wPos.y, wPos.z, 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "dir_light"), wLight.lightPos[0], wLight.lightPos[1], wLight.lightPos[2], 1.f);
		glUniform1f(glGetUniformLocation(modelProgram, "shininess"), wLight.shininess);

		glUniform1i(glGetUniformLocation(modelProgram, "have_ambient"), sword.haveAmbient);
		glUniform1i(glGetUniformLocation(modelProgram, "have_diffuse"), sword.haveDiffuse);
		glUniform1i(glGetUniformLocation(modelProgram, "have_specular"), sword.haveSpecular);

		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glUseProgram(0);
		glBindVertexArray(0);
	}
}
/////////////////////////////////////////////////

#pragma endregion

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

#pragma region OLD
	Cube::setupCube();
	LightCube::setupCube();
	
	// Setup models 
	// DRAGON
	ReadFile(ModelDragon::vertices, ModelDragon::uvs, ModelDragon::normals, "res/cube.obj");
	bool res = loadOBJ("res/cube.obj", ModelDragon::vertices, ModelDragon::uvs, ModelDragon::normals);
	ModelDragon::Init();
	ModelDragon::InitTexture();

	// SWORD
	ReadFile(ModelSword::vertices, ModelSword::uvs, ModelSword::normals, "res/pipe.obj");
	res = loadOBJ("res/pipe.obj", ModelSword::vertices, ModelSword::uvs, ModelSword::normals);
	ModelSword::Init();

	/////////////////////////////////////////////////////

#pragma endregion
}
void GLcleanup() {
	Axis::cleanupAxis();

#pragma region OLD

	Cube::cleanupCube();
	
	/////////////////////////////////////////////////////TODO
	LightCube::cleanupCube();
	ModelDragon::Clean();
	ModelSword::Clean();
	/////////////////////////////////////////////////////////

#pragma endregion
}

#pragma region Dolly Effect

// DOLLY FUNCTION
void MoveCamera()
{
	//ACTIVATE DOLLY EFFECT
	if (moveCamera)
	{
		float distance = RV::panv[2] - dragon.pos[2];
		RV::FOV = 2.f * glm::atan(0.5f * width / glm::abs(distance));
	}
	//RESET FOV
	else
	{
		RV::FOV = glm::radians(radians);
	}
}

#pragma endregion

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
	
	float currentTime = ImGui::GetTime();	// GETTING TIME

#pragma region OLD
	Cube::DrawScenario();
	LightCube::LightCube();

	/////////////////////////////////////////////////////TODO
	
	ModelSword::Render(currentTime);
	ModelDragon::Render(currentTime);
	/////////////////////////////////////////////////////////

#pragma endregion

	ImGui::Render();
}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		/////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("Phong Shading"))
		{
			ImGui::DragFloat3("Light position", wLight.lightPos);
			
			ImGui::SliderFloat3("Ambient Color", wLight.ambient_color, 0.f, 1.f);
			ImGui::SliderFloat3("Diffuse Color", wLight.diffuse_color, 0.f, 1.f);
			ImGui::SliderFloat3("Specular Color", wLight.specular_color, 0.f, 1.f);

			ImGui::SliderFloat("Ambient Strength", &wLight.ambient, 0.0f, 1.0f);
			ImGui::SliderFloat("Diffuse Strength", &wLight.diffuse, 0.0f, 1.0f);
			ImGui::SliderFloat("Specular Strength", &wLight.specular, 0.0f, 1.0f);
			ImGui::SliderFloat("Shininess", &wLight.shininess, 0, 255);
		}

		if (ImGui::CollapsingHeader("Scenario"))
		{
			ImGui::SliderFloat3("Object Color", scenario.rgb, 0.f, 1.f);
			ImGui::Checkbox("Active ambient", &scenario.haveAmbient);
			ImGui::Checkbox("Active diffuse", &scenario.haveDiffuse);
			ImGui::Checkbox("Active specular", &scenario.haveSpecular);
		}

		if (ImGui::CollapsingHeader("Dragon"))
		{
			ImGui::DragFloat3("Object Position", dragon.pos, 0.f, 1.f);
			ImGui::SliderFloat3("Object Color", dragon.rgb, 0.f, 1.f);
			ImGui::Checkbox("Active ambient", &dragon.haveAmbient);
			ImGui::Checkbox("Active diffuse", &dragon.haveDiffuse);
			ImGui::Checkbox("Active specular", &dragon.haveSpecular);
		}
		
		if (ImGui::CollapsingHeader("Sword"))
		{
			ImGui::DragFloat3("Object Position", sword.pos, 0.f, 1.f);
			ImGui::SliderFloat3("Object Color", sword.rgb, 0.f, 1.f);
			ImGui::Checkbox("Active ambient", &sword.haveAmbient);
			ImGui::Checkbox("Active diffuse", &sword.haveDiffuse);
			ImGui::Checkbox("Active specular", &sword.haveSpecular);
		}

		if (ImGui::CollapsingHeader("Camera Properties - Dolly"))
		{
			ImGui::SliderFloat("Width", &width, 5, 30);
			ImGui::SliderFloat("PosZ", &RV::panv[2], -20, -6);
			ImGui::Checkbox("Dolly Effect", &moveCamera);
		}

		/////////////////////////////////////////////////////////
		ImGui::ShowTestWindow();
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