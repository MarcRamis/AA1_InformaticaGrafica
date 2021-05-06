#pragma once

#include <GL\glew.h>

#include <iostream>

class Texture
{
public:
	int width, height, nrChannels;
	unsigned char* data;
	unsigned int id;

	Texture();
	~Texture();
	Texture(const char* path);

	void Clean();
	void Active();
};