#pragma once
class Texture
{
public:
	int width, height, nrChannels;
	unsigned char* data;
	unsigned int id;

	Texture();
};