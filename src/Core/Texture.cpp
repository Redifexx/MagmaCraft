#include "Core/Texture.h"

using namespace Magma;

Texture::Texture(const char* filepath, bool mipmaps)
{
	// Gets file format
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filepath, 0);

	if (format == FIF_UNKNOWN)
	{
		format = FreeImage_GetFIFFromFilename(filepath);
	}
	if (format == FIF_UNKNOWN)
	{
		std::cout << "Could not determine image format for file: " << filepath << std::endl;
		return;
	}

	// Load image into temp bitmap
	FIBITMAP* tempBitmap = FreeImage_Load(format, filepath);
	if (!tempBitmap)
	{
		std::cout << "Failed to load image: " << filepath << std::endl;
		return;
	}

	// Convert to 32-bit RGBA
	FIBITMAP* bitmap = FreeImage_ConvertTo32Bits(tempBitmap);
	FreeImage_Unload(tempBitmap);

	FreeImage_FlipVertical(bitmap);

	GLuint textureID;
	glGenTextures(1, &textureID);

	if (bitmap)
	{
		int width = FreeImage_GetWidth(bitmap);
		int height = FreeImage_GetHeight(bitmap);
		unsigned char* data = FreeImage_GetBits(bitmap);

		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

		if (mipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		}

		// Default texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		m_TextureID = textureID;
	}
	else
	{
		std::cout << "Failed to process image: " << filepath << std::endl;
		glDeleteTextures(1, &textureID);
		m_TextureID = -1;
		return;
	}

	FreeImage_Unload(bitmap);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_TextureID);
}

void Texture::TexParameteri(GLenum target, GLenum pname, GLint param)
{
	glTexParameteri(target, pname, param);
}