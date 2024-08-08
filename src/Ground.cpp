#include "Ground.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
namespace cgCourse
{
	Ground::Ground() : Shape()
	{
		primitiveType = triangle;
				// Set geometry with respect to local origin
		positions = {
			{ -5.0f, 0.0f, -5.0f },	// index 0
			{ 5.0f, 0.0f, -5.0f },	// index 1
			{ 5.0f, 0.0f, 5.0f },	// index 2
			{ -5.0f, 0.0f, 5.0f }	// index 3
		};

		colors = {
			{ 0.7f, 0.7f, 0.9f },
			{ 0.7f, 0.7f, 0.9f },
			{ 0.7f, 0.7f, 0.9f },
			{ 0.7f, 0.7f, 0.9f }
		};

		normals = {
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f }
		};

		texCoords = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};

		faces = {
			{ 0, 1, 2 }, { 2, 3, 0 }
		};

		tangents = {
			{ 1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f }
		};
	}
}

