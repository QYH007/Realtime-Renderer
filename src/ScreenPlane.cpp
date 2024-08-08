#include "ScreenPlane.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cgCourse
{         


	ScreenPlane::~ScreenPlane()
	{
		glDeleteBuffers(1, &posBufferID);
		glDeleteBuffers(1, &texCoordsBufferID);
		glDeleteVertexArrays(1, &vaoID);
	};

	bool ScreenPlane::createVertexArray(GLuint posAttribLoc, GLuint texAttribLoc)
	{
		// check if all buffer locations are somehow defined
		if( (posAttribLoc == GLuint(-1)) || (texAttribLoc == GLuint(-1)))
		{
			return false;
		}

		// Initialize Vertex Array Object
		glGenVertexArrays(1, &vaoID);
		glBindVertexArray(vaoID);

		// Initialize buffer objects with geometry data
		// for positions
		glGenBuffers(1, &posBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, posBufferID);
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2), positions.data(), GL_STATIC_DRAW);
		//GLint posAttribLoc = glGetAttribLocation(shaderProgram, "position");
		glVertexAttribPointer(posAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(posAttribLoc);

        glGenBuffers(1, &texCoordsBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, texCoordsBufferID);
        glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(texAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(texAttribLoc);

		// Reset state
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return true;
	}

	void ScreenPlane::draw() const
	{
		glBindVertexArray(vaoID);
        glDrawArrays(GL_TRIANGLES, 0, 6);
		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		// glDrawElements(GL_TRIANGLES, 3 * faces.size(), GL_UNSIGNED_INT, 0);
		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

