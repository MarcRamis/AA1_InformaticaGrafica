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

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#include "Model.h"

///////////////////// VARIABLES
glm::vec4 wPos;

float width = 16.f;
float radians = 65.f;
bool moveCamera = false;


float moveWTime = 0.0f;

Model *simpleCube;
Model *scenario;
Model *sword;

Light phong = Light(glm::vec4(0.f,0.f,1.f, 1.f), 
	glm::vec4(1.f,1.f,1.f,1.f), glm::vec4(1.f, 1.f, 1.f,1.f), glm::vec4(1.f, 0.f, 0.f,1.f),
	0.2f,0.3f,0.5f,30.f);

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

#pragma endregion

// DOLLY FUNCTION
void MoveCamera()
{
	//ACTIVATE DOLLY EFFECT
	if (moveCamera)
	{
		float distance = RV::panv[2] - simpleCube->obj.pos.z;
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
	// SIMPLE CUBLE
	simpleCube = new Model(Shader("res/files/vert.vs", "res/files/frag.fs", "res/files/geo.gs"), "res/cube.obj",
		ObjectParameters(glm::vec3(1.f, 1.f, 1.f), glm::vec4(0.4f, 1.f, 1.f, 1.f), true, true, false),
		Texture("res/brick2.jpg"));

	// SWORD
	sword = new Model(Shader("res/files/vert.vs", "res/files/frag.fs", "res/files/geo.gs"), "res/espada.obj",
		ObjectParameters(glm::vec3(-7.f, 2.f, -7.f), glm::vec4(0.4f, 1.f, 1.f, 1.f), true, true, false),
		Texture("res/brick.jpg"));

	// SCENARIO
	scenario = new Model(Shader("res/files/vert.vs", "res/files/frag.fs", "res/files/geo.gs"), "res/cube.obj",
		ObjectParameters(glm::vec3(0.f, -1.f, 0.f), glm::vec4(0.5f, 0.5f, 0.5f, 1.f), true, true, false),
		Texture("res/brick.jpg"));
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

	sword->shader.SetFloat("dir_light", phong.pos.x, phong.pos.y, phong.pos.z, 1.f);
	sword->shader.SetFloat("ambient_color", phong.ambient_color.x, phong.ambient_color.y, phong.ambient_color.z, phong.ambient_color.w);
	sword->shader.SetFloat("diffuse_color", phong.diffuse_color.x, phong.diffuse_color.y, phong.diffuse_color.z, phong.diffuse_color.w);
	sword->shader.SetFloat("specular_color", phong.specular_color.x, phong.specular_color.y, phong.specular_color.z, phong.specular_color.w);
	sword->shader.SetFloat("ambient_strength", phong.ambient_strength, phong.ambient_strength, phong.ambient_strength, 1.f);
	sword->shader.SetFloat("diffuse_strength", phong.diffuse_strength, phong.diffuse_strength, phong.diffuse_strength, 1.f);
	sword->shader.SetFloat("specular_strength", phong.specular_strength, phong.specular_strength, phong.specular_strength, 1.f);
	sword->shader.SetFloat("shininess", phong.shininess);

	sword->texture.Active();
	sword->shader.SetInt("ourTexture", 0);

	sword->Draw();

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
	scenario->shader.SetFloat("ambient_strength", phong.ambient_strength, phong.ambient_strength, phong.ambient_strength, 1.f);
	scenario->shader.SetFloat("diffuse_strength", phong.diffuse_strength, phong.diffuse_strength, phong.diffuse_strength, 1.f);
	scenario->shader.SetFloat("specular_strength", phong.specular_strength, phong.specular_strength, phong.specular_strength, 1.f);
	scenario->shader.SetFloat("shininess", phong.shininess);

	scenario->shader.SetFloat("viewPos", wPos.x, wPos.y, wPos.z, 1.f);

	scenario->texture.Active();
	scenario->shader.SetInt("ourTexture", 0);

	scenario->Draw();

#pragma endregion

	// SIMPLE CUBE
#pragma region Simple Cube
	simpleCube->shader.Use();

	// Translate position
	t = glm::translate(glm::mat4(), glm::vec3(simpleCube->obj.pos));
	simpleCube->objMat = t;

	simpleCube->shader.SetMatrix("objMat", 1, GL_FALSE, glm::value_ptr(simpleCube->objMat));
	simpleCube->shader.SetMatrix("mv_Mat", 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
	simpleCube->shader.SetMatrix("mvpMat", 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));

	simpleCube->shader.SetFloat("objColor", simpleCube->obj.color.x, simpleCube->obj.color.y, simpleCube->obj.color.z, simpleCube->obj.color.w);

	simpleCube->shader.SetFloat("dir_light", phong.pos.x, phong.pos.y, phong.pos.z, 1.f);
	simpleCube->shader.SetFloat("ambient_color", phong.ambient_color.x, phong.ambient_color.y, phong.ambient_color.z, phong.ambient_color.w);
	simpleCube->shader.SetFloat("diffuse_color", phong.diffuse_color.x, phong.diffuse_color.y, phong.diffuse_color.z, phong.diffuse_color.w);
	simpleCube->shader.SetFloat("specular_color", phong.specular_color.x, phong.specular_color.y, phong.specular_color.z, phong.specular_color.w);
	simpleCube->shader.SetFloat("ambient_strength", phong.ambient_strength, phong.ambient_strength, phong.ambient_strength, 1.f);
	simpleCube->shader.SetFloat("diffuse_strength", phong.diffuse_strength, phong.diffuse_strength, phong.diffuse_strength, 1.f);
	simpleCube->shader.SetFloat("specular_strength", phong.specular_strength, phong.specular_strength, phong.specular_strength, 1.f);
	simpleCube->shader.SetFloat("shininess", phong.shininess);

	simpleCube->shader.SetFloat("viewPos", wPos.x, wPos.y, wPos.z, 1.f);

	moveWTime = cos(currentTime);
	simpleCube->shader.SetFloat("moveWTime", moveWTime);

	simpleCube->texture.Active();
	simpleCube->shader.SetInt("ourTexture", 0);

	simpleCube->Draw();

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
	simpleCube->texture.Clean();
}

#pragma region Dolly Effect



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
			//ImGui::Checkbox("Active ambient", &scenario->obj.haveAmbient);
			//ImGui::Checkbox("Active diffuse", &scenario->obj.haveDiffuse);
			//ImGui::Checkbox("Active specular", &scenario->obj.haveSpecular);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Simple Cube"))
		{
			ImGui::DragFloat3("Translate", glm::value_ptr(simpleCube->obj.pos), 0.f, 1.f);
			ImGui::ColorEdit4("Object Color", glm::value_ptr(simpleCube->obj.color));
			//ImGui::Checkbox("Active ambient", &simpleCube->obj.haveAmbient);
			//ImGui::Checkbox("Active diffuse", &simpleCube->obj.haveDiffuse);
			//ImGui::Checkbox("Active specular", &simpleCube->obj.haveSpecular);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Sword"))
		{
			ImGui::DragFloat3("Translate", glm::value_ptr(sword->obj.pos), 0.f, 1.f);
			ImGui::ColorEdit4("Object Color", glm::value_ptr(sword->obj.color));
			//ImGui::Checkbox("Active ambient", &sword->obj.haveAmbient);
			//ImGui::Checkbox("Active diffuse", &sword->obj.haveDiffuse);
			//ImGui::Checkbox("Active specular", &sword->obj.haveSpecular);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Camera Properties - Dolly"))
		{
			ImGui::SliderFloat("Width", &width, 5, 30);
			ImGui::SliderFloat("PosZ", &RV::panv[2], -20, -6);
			ImGui::Checkbox("Dolly Effect", &moveCamera);

			ImGui::TreePop();
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