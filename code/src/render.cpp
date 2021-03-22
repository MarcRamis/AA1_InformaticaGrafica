#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <cassert>

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>

#include "GL_framework.h"

///////////////////// LOAD MODEL
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

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

//////////////////////////////////////////////////
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

struct WLight
{
	float r, g, b;
	float ambient, diffuse, specular;
	float lX, lY, lZ;

	WLight(float _r, float _g, float _b, float _ambient, float _diffuse, float _specular, float _lX, float _lY, float _lZ)
		: r(_r), g(_g), b(_b), ambient(_ambient), diffuse(_diffuse), specular(_specular), lX(_lX), lY(_lY), lZ(_lZ) {};
};
WLight wLight(0.5f, 0.f, 0.f, 0.5f, 1.f, 0.5f, 1.f, 0.f, 0.f);

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
out vec3 vert_wPos;\n\
uniform mat4 objMat;\n\
uniform mat4 mv_Mat;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
	vert_Normal = objMat * vec4(in_Normal, 0.0);\n\
}";
	//////////////// AMBIENT + DIFFUSE
	/*
	const char* cube_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	out vec4 out_Color;\n\
	uniform mat4 mv_Mat;\n\
	uniform vec4 color;\n\
	uniform vec4 dir_light; \n\
	uniform vec4 ambient; \n\
	uniform vec4 diffuse; \n\
	void main() {\n\
		vec4 ambientComp = color * ambient; \n\
		vec4 diffuseComp = dot(vert_Normal, normalize(dir_light)) * diffuse * color; \n\
		out_Color = ambientComp + diffuseComp;\n\
	}";
	*/
	//////////////// AMBIENT + DIFFUSE + SPECULAR
	const char* cube_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	in vec3 vert_wPos;\n\
	out vec4 out_Color;\n\
	uniform mat4 mv_Mat;\n\
	uniform vec4 color;\n\
	uniform vec4 dir_light; \n\
	uniform vec4 ambient; \n\
	uniform vec4 diffuse; \n\
	uniform vec4 viewPos; \n\
	uniform vec4 specular; \n\
	void main() {\n\
		vec4 ambientComp = color * ambient; \n\
		vec4 diffuseComp = dot(vert_Normal, normalize(dir_light)) * diffuse * color; \n\
		vec4 viewDir = normalize(viewPos - vert_wPos);\n\
		vec4 reflectDir = reflect(-dir_light, vert_Normal);\n\
		vec4 specularComp = pow(max(dot(viewDir, reflectDir),0.0), 255) * specular * color ;\n\
		out_Color = ambientComp + diffuseComp + specularComp;\n\
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
	void drawCube() {
		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "ambient"), 0.5f, 0.5f, 0.5f, 0.5f);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}
	void drawTwoCubes() {
		glEnable(GL_PRIMITIVE_RESTART);
		glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(-1.0f, 2.0f, 3.0f));
		objMat = t;

		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), wLight.r, wLight.g, wLight.b, 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "ambient"), wLight.ambient, wLight.ambient, wLight.ambient, 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "diffuse"), wLight.diffuse, wLight.diffuse, wLight.diffuse, 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "specular"), wLight.specular, wLight.specular, wLight.specular, 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "wPos"), wLight.specular, wLight.specular, wLight.specular, 1.f);
		glUniform4f(glGetUniformLocation(cubeProgram, "dir_light"), wLight.lX, wLight.lX, wLight.lX, 1.f);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		
		t = glm::translate(glm::mat4(), glm::vec3(1.0f, 2.0f, 3.0f));
		objMat = t;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);

		glDisable(GL_PRIMITIVE_RESTART);
	}
}
////////////////////////////////////////////////// MODEL
namespace Model
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
		uniform mat4 objMat;\n\
		uniform mat4 mv_Mat;\n\
		uniform mat4 mvpMat;\n\
		void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = objMat * vec4(in_Normal, 0.0);\n\
		}";

	//////////////// NO MATERIAL
	/*
	const char* model_fragShader =
		"#version 330\n\
		in vec4 vert_Normal;\n\
		out vec4 out_Color;\n\
		uniform mat4 mv_Mat;\n\
		uniform vec4 color;\n\
		uniform vec4 ambient;\n\
		void main() {\n\
			out_Color = color;\n\
		}";
		*/
	//////////////// AMBIENT + DIFFUSE
	const char* model_fragShader =
		"#version 330\n\
	in vec4 vert_Normal;\n\
	out vec4 out_Color;\n\
	uniform mat4 mv_Mat;\n\
	uniform vec4 color;\n\
	uniform vec4 dir_light; \n\
	uniform vec4 ambient; \n\
	uniform vec4 diffuse; \n\
	void main() {\n\
		vec4 ambientComp = color * ambient; \n\
		vec4 diffuseComp = dot(vert_Normal, normalize(dir_light)) * diffuse * color; \n\
		out_Color = ambientComp + diffuseComp;\n\
	}";
	
	//////////////// DIFFUSE
	/*const char* model_fragShader =
		"#version 330\n\
		in vec4 vert_Normal;\n\
		out vec4 out_Color;\n\
		uniform mat4 mv_Mat;\n\
		uniform vec4 color;\n\
		uniform vec4 directional_light; \n\
		uniform vec4 diffuse; \n\
		void main() {\n\
		out_Color = dot(vert_Normal, normalize(directional_light)) * diffuse * color;\n\
		}";
		*/

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
	void Render()
	{
		//glEnable(GL_PRIMITIVE_RESTART);

		glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));
		objMat = t;

		glBindVertexArray(modelVao);
		glUseProgram(modelProgram);

		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

		glUniform4f(glGetUniformLocation(modelProgram, "color"), wLight.r, wLight.g, wLight.b, 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "ambient"), wLight.ambient, wLight.ambient, wLight.ambient, 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "diffuse"), wLight.diffuse, wLight.diffuse, wLight.diffuse, 1.f);
		glUniform4f(glGetUniformLocation(modelProgram, "dir_light"), wLight.lX, wLight.lY, wLight.lZ, 1.f);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		
		glUseProgram(0);
		glBindVertexArray(0);
		//glDisable(GL_PRIMITIVE_RESTART);
	}
}
/////////////////////////////////////////////////
GLuint program;
GLuint VAO;		// VAO (Vertex Array Object)
				// Maintains all of the state related to input of the
				// OpenGL pipeline.

GLuint VBO;		// VBF (Vertex Buffer Object)

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
	std::vector < glm::vec3 >& out_normals);

// A vertex shader that assigns a static position to the vertex
static const GLchar* vertex_shader_source[] = {
	"#version 330\n"
	"layout(location = 0) in vec3 aPos;"
	"\n"
	"void main() {\n"
	"gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0);\n"
	"}"
};

// A fragment shader that assigns a static color
static const GLchar* fragment_shader_source[] = {
	"#version 330\n"
	"uniform vec4 aCol;\n"
	"out vec4 color;\n"

	"void main() {\n"
	"color = aCol;\n"
	"}"
};

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
	Cube::setupCube();

	// Read & Load model
	ReadFile(Model::vertices, Model::uvs, Model::normals);
	bool res = loadOBJ("res/dragon.obj", Model::vertices, Model::uvs, Model::normals);
	// Init model
	Model::Init();

	/////////////////////////////////////////////////////
}
void GLcleanup() {
	Axis::cleanupAxis();
	Cube::cleanupCube();

	/////////////////////////////////////////////////////TODO
	// Do your cleanup code here
	// ...
	// ...
	// ...
	/////////////////////////////////////////////////////////
}

void GLrender(float dt) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_MVP = RV::_projection * RV::_modelView;

	Axis::drawAxis();
	Cube::drawTwoCubes();

	/////////////////////////////////////////////////////TODO
	Model::Render();
	/////////////////////////////////////////////////////////

	ImGui::Render();
}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		/////////////////////////////////////////////////////////
		ImGui::SliderFloat("R", &wLight.r, 0.0f, 1.0f);
		ImGui::SliderFloat("G", &wLight.g, 0.0f, 1.0f);
		ImGui::SliderFloat("B", &wLight.b, 0.0f, 1.0f);
		
		ImGui::SliderFloat("Ambient", &wLight.ambient, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse", &wLight.diffuse, 0.0f, 1.0f);
		ImGui::SliderFloat("Specular", &wLight.specular, 0.0f, 1.0f);
		
		ImGui::SliderFloat("Light X", &wLight.lX, -1.f, 1.0f);
		ImGui::SliderFloat("Light Y", &wLight.lY, -1.f, 1.0f);
		ImGui::SliderFloat("Light Z", &wLight.lZ, -1.f, 1.0f);
		//ImGui::SliderFloat("Specular", &f, 0.0f, 1.0f);
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