#ifndef CUBE_H
#define CUBE_H

#include "Shape.h"

namespace cgCourse
{
	class Cube : public Shape
	{
	public:
		Cube();
		virtual GLsizei getDrawElemCount() const override;
		
	};
}

#endif // CUBE_H

