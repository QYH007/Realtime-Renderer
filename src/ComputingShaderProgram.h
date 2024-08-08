#ifndef COMPUTINGSHADERPROGRAM_H
#define COMPUTINGSHADERPROGRAM_H

#include "GLIncludes.h"
#include <string>

namespace cgCourse
{
	class ComputingShaderProgram
	{
	public:
		ComputingShaderProgram(std::string _name);

		void deleteShaderProgramFromGPU();
		void bind() const;
		void unbind() const;

		GLuint getUniformLocation(const std::string & _uniformName);
		void SetUniform1i(const std::string& name, int value);

	private:
		bool loadAndCompileShaderToGPU(GLuint _program, const std::string & _source);
		bool checkLinkStatusOfProgram(GLuint _program);
		std::string loadFile(const std::string & filename) const;

		GLuint program = 0;
	};
}

#endif // COMPUTINGSHADERPROGRAM_H

