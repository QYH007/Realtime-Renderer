#include "Skybox.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cgCourse
{         
	Skybox::~Skybox()
	{
		glDeleteBuffers(1, &posBufferID);
		glDeleteVertexArrays(1, &vaoID);
	};

	bool Skybox::createVertexArray(GLuint posAttribLoc)
	{
		// check if all buffer locations are somehow defined
		if( (posAttribLoc == GLuint(-1)))
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
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
		//GLint posAttribLoc = glGetAttribLocation(shaderProgram, "position");
		glVertexAttribPointer(posAttribLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(posAttribLoc);

		// Reset state
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return true;
	}

	void Skybox::draw() const
	{
		glBindVertexArray(vaoID);
        glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
}

