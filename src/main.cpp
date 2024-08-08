#include "GLExample.h"

int main(int argc, char * argv[])
{
	cgCourse::GLExample app(glm::uvec2(1920, 1080), "OpenGL");
	return app.run();
}

