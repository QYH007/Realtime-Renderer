#ifndef SKYBOX_H
#define SKYBOX_H

#include "GLIncludes.h"
#include <vector>
#include <glm/mat4x4.hpp>

namespace cgCourse
{
	class Skybox
	{
	public:
        Skybox(){
            positions = {
                {-1.0f,   1.0f,  -1.0f},
                {-1.0f,  -1.0f,  -1.0f},
                {1.0f,  -1.0f,  -1.0f},
                {1.0f,  -1.0f,  -1.0f},
                {1.0f,   1.0f,  -1.0f},
                {-1.0f,   1.0f,  -1.0f},

                {-1.0f,  -1.0f,   1.0f},
                {-1.0f,  -1.0f,  -1.0f},
                {-1.0f,   1.0f,  -1.0f},
                {-1.0f,   1.0f,  -1.0f},
                {-1.0f,   1.0f,   1.0f},
                {-1.0f,  -1.0f,   1.0f},

                {1.0f,  -1.0f,  -1.0f},
                {1.0f,  -1.0f,   1.0f},
                {1.0f,   1.0f,   1.0f},
                {1.0f,   1.0f,   1.0f},
                {1.0f,   1.0f,  -1.0f},
                {1.0f,  -1.0f,  -1.0f},

                {-1.0f,  -1.0f,   1.0f},
                {-1.0f,   1.0f,   1.0f},
                {1.0f,   1.0f,   1.0f},
                {1.0f,   1.0f,   1.0f},
                {1.0f,  -1.0f,   1.0f},
                {-1.0f,  -1.0f,   1.0f},

                {-1.0f,   1.0f,  -1.0f},
                {1.0f,   1.0f,  -1.0f},
                {1.0f,   1.0f,   1.0f},
                {1.0f,   1.0f,   1.0f},
                {-1.0f,   1.0f,   1.0f},
                {-1.0f,   1.0f,  -1.0f},

                {-1.0f,  -1.0f,  -1.0f},
                {-1.0f,  -1.0f,   1.0f},
                {1.0f,  -1.0f,  -1.0f},
                {1.0f,  -1.0f,  -1.0f},
                {-1.0f,  -1.0f,   1.0f},
                {1.0f,  -1.0f,   1.0f},
            };
        
        };
		virtual ~Skybox();

		bool createVertexArray( GLuint posAttribLoc);

		virtual void draw() const;

	protected:
		GLuint vaoID = 0;
		GLuint posBufferID = 0;

	public:
        std::vector<glm::vec3> positions;
	};
}

#endif // SKYBOX_H

