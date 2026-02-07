#pragma once

#include <FreeImage.h>
#include <glad/glad.h>
#include <iostream>

// allows the creation of openGL textures
// until more texture types are required, options are limited
namespace Magma
{
	class Texture
	{
		public:
			// for normal image textures
			Texture(const char* filepath, 
				bool mipmaps = true,
				GLint internalFormat = GL_SRGB_ALPHA,
				GLenum format = GL_RGB,
				GLenum type = GL_UNSIGNED_BYTE
			);

			// for data
			Texture(int width, int height, GLenum target, GLint internalFormat, GLenum format, GLenum type, const void* data);
			~Texture();
			GLuint GetID() const { return m_TextureID; }
			void TexParameteri(GLenum target, GLenum pname, GLint param);
        private:
            GLuint m_TextureID = -1; // let wrap around
	};
}