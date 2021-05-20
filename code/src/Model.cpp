#include "Model.h"

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

Model::Model(Shader _shader, const char* path, ObjectParameters objParameters, Texture _texture) : shader(_shader), obj(objParameters), texture(_texture)
{
	// READ MODEL
	ReadFile(vertices, uvs, normals, path);
	bool res = loadOBJ(path, vertices, uvs, normals);
	
	// INITIALIZE MODEL
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(3, vbo);
	
	// Vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Normals
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Uvs
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Model::~Model()
{
	vertices.clear();
	normals.clear();
	uvs.clear();

	glDeleteBuffers(3, vbo);
	glDeleteVertexArrays(1, &vao);
}

void Model::ActiveDepth()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_ALWAYS); 
}

void Model::ActiveStencil()
{

}

void Model::DrawTriangles()
{
#pragma region Render objects
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	
	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);

#pragma endregion
}
void Model::DrawPoints()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, vertices.size());

	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);
}

//void Model::DrawFrameBuffer(glm::mat4 _MVP, glm::mat4 _ModelView, glm::mat4 _projection)
//{
//	//we store the current values in a temporary variable
//	glm::mat4 t_mvp = _MVP;
//	glm::mat4 t_mv = _ModelView;
//
//	// we set up our framebuffer and draw into it
//	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//	//glClearColor(1.f, 1.f, 1.f, 1.f);
//	glViewport(0, 0, 800, 800);
//	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	//glClear(GL_COLOR_BUFFER_BIT);
//	//glEnable(GL_DEPTH_TEST);
//	_MVP = _projection;
//	_ModelView = glm::mat4(1.f);
//	
//	//we restore the previous conditions
//	_MVP = t_mvp;
//	_ModelView = t_mv;
//	
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	
//	//we set up a texture where to draw our FBO:
//	glViewport(0, 0, fbo_Tex.width, fbo_Tex.height);
//	glBindTexture(GL_TEXTURE_2D, fbo_Tex.id);
//
//	DrawTriangles();
//}