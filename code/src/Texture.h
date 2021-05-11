#pragma once

#include <GL\glew.h>

#include <iostream>



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

	void Clean();
	void Active();
};