#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



Texture::Texture() {}
Texture::Texture(const char* path, ETextureType type) 
{
	if (path != nullptr)
	{
		// LOAD TEXTURE
		data = stbi_load(path, &width, &height, &nrChannels, 0);

		// CREATE TEXTURE
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		switch (type)
		{
			case ETextureType::JPG:
				if (data) {
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
					glGenerateMipmap(GL_TEXTURE_2D);
				}
				break;
			case ETextureType::PNG:
				if (data) {
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
					glGenerateMipmap(GL_TEXTURE_2D);
				}
				break;
		}


		// IMAGE PARAMETERS
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "No texture" << std::endl;
	}
}

Texture::~Texture()
{
	//stbi_image_free(data);
}

void Texture::Clean()
{
	//stbi_image_free(data);
	glDeleteTextures(1, &id);
}

void Texture::Active()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
}