#include "Core/Shader.h"

using namespace Magma;

Shader::Shader(const std::string& filepath, GLenum type)
	: m_Type(type)
{
	std::string source;
	std::ifstream file;

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// Opening and reading the file's content
		file.open(filepath);

		std::stringstream ss;
		ss << file.rdbuf();
		file.close();
		source = ss.str();
	}
	catch (std::ifstream::failure& e)
	{
		throw std::runtime_error("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " + filepath);
	}

	m_ID = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(m_ID, 1, &src, nullptr);
}

Shader::~Shader()
{
	glDeleteShader(m_ID);
}

bool Shader::Compile()
{
	GLint success;
	GLchar infoLog[512];

	glCompileShader(m_ID);

	// Check for compile errors
	glGetShaderiv(m_ID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(m_ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		return false;
	}

	return true;
}