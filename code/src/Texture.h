#pragma once

#include <GL\glew.h>

#include <iostream>



class Texture
{
public:
	
	enum class ETextureType { PNG, JPG };

	int width, height, nrChannels;
	unsigned char* data;
	unsigned int id;

	Texture();
	~Texture();
	Texture(const char* path, ETextureType type);

	void Clean();
	void Active();
};