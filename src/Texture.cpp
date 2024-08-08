#include "Texture.h"

#include <iostream>
#include <cstring>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <sstream>

#define cimg_display 0
#include "CImg.h"

using namespace cimg_library;


namespace cgCourse
{
	void Texture::loadFromFile(const std::string & _filename, bool _srgb)
	{
		CImg<unsigned char> img(_filename.c_str());

		int channels = img.spectrum();
		size.x = img.width();
		size.y = img.height();

		img.permute_axes("cxyz");
		img.mirror('z');

		glGenTextures(1, &texhandle);
		glBindTexture(GL_TEXTURE_2D, texhandle);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        switch (channels) {
            case 4:
                glTexImage2D(GL_TEXTURE_2D, 0, _srgb ? GL_SRGB8_ALPHA8 : GL_RGBA,size.x, size.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.data());
                break;
            case 3:
                glTexImage2D(GL_TEXTURE_2D, 0, _srgb ? GL_SRGB8 : GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
                break;
            case 1:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size.x, size.y, 0, GL_RED, GL_UNSIGNED_BYTE, img.data());
                break;
        }

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

    void Texture::loadData(const std::string & _filename, bool _srgb, int * height, int * width, int * channel){
        CImg<unsigned char> img(_filename.c_str());

		*channel = img.spectrum();
		*width = img.width();
		*height = img.height();
		img.permute_axes("cxyz");
		img.mirror('z');

    }

    void Texture::loadCubemap(std::vector<std::string>& faces)
    {
        glGenTextures(1, &texhandle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texhandle);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                 switch (nrChannels) {
                    case 4:
                        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                        break;
                    case 3:
                        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                        break;
                    case 1:
                        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
                        break;
                }
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

	const GLuint & Texture::getTexHandle() const
	{
		return texhandle;
	}
}

