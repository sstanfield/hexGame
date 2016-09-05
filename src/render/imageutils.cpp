/*
Copyright (c) 2015-2016 Steven Stanfield

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would
   be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
*/
#include "GL/glew.h"
#include "imageutils.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_HDR
#define STBI_ONLY_PNG
#include "gui/tb/thirdparty/stb_image.h"


//#include "gui/tb/tb_bitmap_fragment.h"

#include <stdlib.h>

// Uses the STBI code for PNG loading, should NOT be used for untrusted files.
unsigned int read_png_file(const char* file_name)
{
	int width = 0;
	int height = 0;
	int pixelSize = 0;

	FILE *fp = fopen(file_name, "rb");
	unsigned char *tex_mem = stbi_load_from_file(fp, &width, &height, &pixelSize, 4);
	fclose(fp);
	
	//tb::TBImageLoader *img = tb::TBImageLoader::CreateFromFile(file_name);

	GLuint texture = 0;
	if (tex_mem) {
	//if (img && img->Data()) {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		//glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, img->Width(), img->Height(), 0, GL_RGBA,
		//		GL_UNSIGNED_BYTE, img->Data());
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, tex_mem);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_2D);

		//delete img;
		free(tex_mem);
	}

	return texture;
}
