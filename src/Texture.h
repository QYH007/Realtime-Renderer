#ifndef TEXTURE_H
#define TEXTURE_H

#include "GLIncludes.h"
#include <string>
#include <glm/vec2.hpp>
#include <vector>

namespace cgCourse
{
	class Texture
	{
	public:
		void loadFromFile(const std::string & filename, bool _srgb = false);
		void loadData(const std::string & _filename, bool _srgb, int * height, int * width, int * channel);
		const GLuint & getTexHandle() const;
		unsigned char * getData() const;
		void loadCubemap(std::vector<std::string>& faces);

	private:
		GLuint texhandle = 0;

		// Texture info
		glm::vec2 size;
		uint8_t bitsPerPixel = 0;
	};
}

#endif // TEXTURE_H

