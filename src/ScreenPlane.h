#ifndef SCREENPLANE_H
#define SCREENPLANE_H

#include "GLIncludes.h"
#include <vector>
#include <glm/mat4x4.hpp>

namespace cgCourse
{
	class ScreenPlane
	{
	public:
		ScreenPlane() = default;
        ScreenPlane(float bottomleft_x, float bottomleft_y, float upperright_x, float upperright_y){
            positions = {
                { bottomleft_x, upperright_y },
                { bottomleft_x, bottomleft_y },
                { upperright_x, bottomleft_y },

                { bottomleft_x, upperright_y },
                { upperright_x, bottomleft_y },
                { upperright_x, upperright_y },
            };
            texCoords = {
                { 0.0f, 1.0f },
                { 0.0f, 0.0f },
                { 1.0f, 0.0f },

                { 0.0f, 1.0f },
                { 1.0f, 0.0f },
                { 1.0f, 1.0f },
            };
        
        };
		virtual ~ScreenPlane();

		bool createVertexArray( GLuint posAttribLoc, GLuint texAttribLoc);

		virtual void draw() const;

	protected:
		GLuint vaoID = 0;
		GLuint posBufferID = 0;
		GLuint texCoordsBufferID = 0;

	public:
        std::vector<glm::vec2> positions;
        std::vector<glm::vec2> texCoords;
	};
}

#endif // SCREENPLANE_H

