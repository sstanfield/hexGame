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

#include <stdlib.h>
#include <png.h>

unsigned int read_png_file(const char* file_name)
{
	int width, height;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;
	char header[8];    // 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");
	if (!fp) {
		printf("[read_png_file] File %s could not be opened for reading\n", file_name);
		return -1;
	}
	fread(header, 1, 8, fp);
	if (png_sig_cmp((png_bytep)header, 0, 8)) {
		printf("[read_png_file] File %s is not recognized as a PNG file\n", file_name);
		return -1;
	}

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		printf("[read_png_file] png_create_read_struct failed\n");
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		printf("[read_png_file] png_create_info_struct failed\n");
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("[read_png_file] Error during init_io");
		return -1;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);

	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("[read_png_file] Error during read_image");
		return -1;
	}

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	int rowBytes = png_get_rowbytes(png_ptr,info_ptr);
	png_byte *tex_mem = (png_byte *)malloc(height * rowBytes);
	int y;
	for (y=0; y<height; y++)
		row_pointers[height - 1 - y] = (png_byte*) &tex_mem[y * rowBytes];

	png_read_image(png_ptr, row_pointers);

	free(row_pointers);
	if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA) {
		printf("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)\n",
				PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));
		free(tex_mem);
		return -1;
	}

	fclose(fp);

	GLuint texture;
	glGenTextures(1, &texture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0, GL_RGBA,
	             GL_UNSIGNED_BYTE, tex_mem);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);//GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	free(tex_mem);

	return texture;
}
