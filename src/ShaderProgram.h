#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "GLIncludes.h"
#include <map>
#include <string>
#include <glm/mat4x4.hpp>
#include "Texture.h"
#include <memory>

namespace cgCourse
{
	class ShaderProgram
	{
	public:
		ShaderProgram(std::string _name);

		void deleteShaderProgramFromGPU();
		void bind() const;
		void unbind() const;
		void addTexture(std::string _textureVarName, unsigned int _handle);
		void addCubeMap(std::string _textureVarName, unsigned int _handle);
        void clearTextures();
		
		GLuint getUniformLocation(const std::string& _uniformName) const;

		void setUniform3fv(std::string _uniformName, glm::vec3 _value);
        void setUniformMat4fv(std::string _uniformName, glm::mat4 _value);
        void setUniformf(std::string _uniformName, float _value);
        void setUniformi(std::string _uniformName, int _value);
        void setUniform2fv(std::string _uniformName, glm::vec2 _value);
        void setUniformBlockBuffer(std::string _uniformBlockName, unsigned int _bufferObject);


	private:
		bool loadAndCompileShaderToGPU(GLuint _program, const std::string & _source);
		bool checkLinkStatusOfProgram(GLuint _program);
		std::string loadFile(const std::string & filename) const;

		GLuint program;
		std::map<std::string,unsigned int> textures;
        std::map<std::string,glm::vec3> uniforms3fv;
        std::map<std::string,glm::mat4> uniformsMat4fv;
        std::map<std::string,float> uniformsf;
        std::map<std::string,int> uniformsi;
        std::map<std::string,glm::vec2> uniforms2fv;
        std::map<std::string, unsigned int> uniformBlockBuffers;
		std::map<std::string, unsigned int> cubeMaps;

	};
}

#endif // SHADERPROGRAM_H

