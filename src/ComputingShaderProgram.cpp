#include "ComputingShaderProgram.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <iostream>
#include <fstream>

namespace cgCourse
{
	ComputingShaderProgram::ComputingShaderProgram(std::string _name)
	{
		program = 0;

		// load shader source code
		std::string ShaderString = loadFile(_name + ".shader");

		// Build and compile our shader program consisting of
		// a vertex and fragment shader
		unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);

		loadAndCompileShaderToGPU(computeShader, ShaderString);
		
		// Link shaders
		program = glCreateProgram();
		// check if it exists
		assert(program);
		// attach the shaders to the shader program
		glAttachShader(program, computeShader);

		// link the program to make it ready to use by the GPU
		glLinkProgram(program);
		glValidateProgram(program);
		// check the link status of the program and show linking errors
		// if there are any
		checkLinkStatusOfProgram(program);

		// shader programs can be marked as to delete. We don't have
		// to do this here if it is done later anywhen in the program. An explanation is here:
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDeleteShader.xhtml
		//
		glDeleteShader(computeShader);
	}

	std::string ComputingShaderProgram::loadFile(const std::string & _filename) const
	{
		std::string result;
		std::ifstream stream(_filename.c_str());

		if(!stream.is_open())
			return result;

		stream.seekg(0, std::ios::end);
		result.reserve(stream.tellg());
		stream.seekg(0, std::ios::beg);

		result.assign(	std::istreambuf_iterator<char>(stream),
						std::istreambuf_iterator<char>()
						);

		return result;
	}
	void ComputingShaderProgram::SetUniform1i(const std::string& name, int value){
		
		glUniform1i(getUniformLocation(name), value);
	}


	bool ComputingShaderProgram::loadAndCompileShaderToGPU(GLuint _program, const std::string & _source)
	{
		GLint status;
		GLchar errorMessage[512];

		const char* src = _source.c_str();
		glShaderSource(_program, 1, &src, nullptr);
		glCompileShader(_program);
		glGetShaderiv(_program, GL_COMPILE_STATUS, &status);
		if(!status)
		{
			glGetShaderInfoLog(_program, 512, 0, errorMessage);
			std::cout << "Fragment Shader compilation error:" << std::endl;
			std::cout << errorMessage << std::endl;
			return false;
		}
		return true;
	}

	bool ComputingShaderProgram::checkLinkStatusOfProgram(GLuint _program)
	{
		GLint status;
		GLchar errorMessage[512];
		glGetProgramiv(_program, GL_LINK_STATUS, &status);
		if(!status)
		{
			glGetProgramInfoLog(_program, 512, 0, errorMessage);
			std::cerr << "compute shader Linking error:" << std::endl;
			std::cerr << errorMessage << std::endl;
			return false;
		}
		return true;
	}

	GLuint ComputingShaderProgram::getUniformLocation(const std::string & _uniformName)
	{
		int location = glGetUniformLocation(program, _uniformName.c_str());
		if (location == -1) {
			std::cout << "Warning: uniform " << _uniformName << " doesn't exist!" << std::endl;
		}
		return glGetUniformLocation(program, _uniformName.c_str());
	}

	void ComputingShaderProgram::deleteShaderProgramFromGPU()
	{
		glDeleteProgram(program);
	}

	void ComputingShaderProgram::bind() const
	{
		glUseProgram(program);
	}

	void ComputingShaderProgram::unbind() const
	{
		glUseProgram(0);
	}
}

