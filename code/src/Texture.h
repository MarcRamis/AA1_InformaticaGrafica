#pragma once

#include <GL\glew.h>

#include <iostream>
#include <vector>

#include "stb_image.h"

class Texture
{
public:
	
	enum class ETextureType { NONE, PNG, JPG };

	int width, height, nrChannels;
	unsigned char* data;
	unsigned int id;

	const char* m_Path;

	Texture();
	~Texture();
	Texture(const char* path, ETextureType type);
	Texture(unsigned int _id, const char* path, ETextureType type);

	void Clean();
	void Active();
};

class CubeMap
{
public:
	unsigned int textureID;
	int width, height, nrChannels;
	unsigned char* data;

	unsigned int vao;

	CubeMap();
	CubeMap(std::vector<std::string> faces);
};