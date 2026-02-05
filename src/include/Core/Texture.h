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
			Texture(const char* filepath, bool mipmaps = true);
			~Texture();
			GLuint GetID() const { return m_TextureID; }
			void TexParameteri(GLenum target, GLenum pname, GLint param);
        private:
            GLuint m_TextureID;
	};
}