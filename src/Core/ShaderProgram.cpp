#include "Core/ShaderProgram.h"

using namespace Magma;

ShaderProgram::ShaderProgram()
{
	m_ID = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(m_ID);
}

void ShaderProgram::AttachShader(const Shader& shader)
{
	glAttachShader(m_ID, shader.GetID());
}

bool ShaderProgram::Link()
{
	glLinkProgram(m_ID);

	GLint success;
	GLchar infoLog[512];
	glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		return false;
	}

	return true;
}

void ShaderProgram::Use() const
{
	glUseProgram(m_ID);
}

void ShaderProgram::SetUniform(const std::string& name, int value) const
{
	GLint location = glGetUniformLocation(m_ID, name.c_str());

	if (location == -1)
	{
		std::cout << "WARNING::SHADER_PROGRAM::UNIFORM_NOT_FOUND: " << name << std::endl;
		return;
	}

	glUniform1i(location, value);
}

void ShaderProgram::SetUniform(const std::string& name, float value) const
{
	GLint location = glGetUniformLocation(m_ID, name.c_str());

	if (location == -1)
	{
		std::cout << "WARNING::SHADER_PROGRAM::UNIFORM_NOT_FOUND: " << name << std::endl;
		return;
	}

	glUniform1f(location, value);
}

void ShaderProgram::SetUniform(const std::string& name, const glm::mat4& mat) const
{
	GLint location = glGetUniformLocation(m_ID, name.c_str());

	if (location == -1)
	{
		std::cout << "WARNING::SHADER_PROGRAM::UNIFORM_NOT_FOUND: " << name << std::endl;
		return;
	}

	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}