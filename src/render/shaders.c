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
#include "shaders.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "GL/glew.h"

GLuint LoadShaders(const char *vertex_shader,const char *fragment_shader) {
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
//	printf("Compiling shader : %s\n", vertex_file_path);
	glShaderSource(VertexShaderID, 1, &vertex_shader , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	char VertexShaderErrorMessage[InfoLogLength];
	VertexShaderErrorMessage[0] = 0;
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, VertexShaderErrorMessage);
	if (VertexShaderErrorMessage[0]) fprintf(stdout, "%s\n", VertexShaderErrorMessage);

	// Compile Fragment Shader
//	printf("Compiling shader : %s\n", fragment_file_path);
//	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &fragment_shader , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	char FragmentShaderErrorMessage[InfoLogLength];
	FragmentShaderErrorMessage[0] = 0;
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, FragmentShaderErrorMessage);
	if (FragmentShaderErrorMessage[0]) fprintf(stdout, "%s\n", FragmentShaderErrorMessage);

	// Link the program
//	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	char ProgramErrorMessage[ InfoLogLength<1?1:InfoLogLength ];
	ProgramErrorMessage[0] = 0;
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage);
	if (ProgramErrorMessage[0]) fprintf(stdout, "%s\n", ProgramErrorMessage);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

// XXX TODO- better error handling.
GLuint LoadShadersFromFile(const char *vertex_file_path,
						   const char *fragment_file_path) {
	printf("Compiling shaders : %s, %s\n", vertex_file_path, fragment_file_path);
	struct stat st;
	if (stat(vertex_file_path, &st)) return 0;
	// Read the Vertex Shader code from the file
	char VertexShaderCode[st.st_size+1];
	memset(VertexShaderCode, 0, st.st_size+1);

	FILE *fp;
	char ch;

	fp = fopen(vertex_file_path,"rb"); // read mode
	if( fp == NULL )
	{
		perror("Error while opening the file.\n");
		return 0;
	}
	int c = 0;
	while( ( ch = fgetc(fp) ) != EOF )
		VertexShaderCode[c++] = ch;
	fclose(fp);

	if (stat(fragment_file_path, &st)) return 0;
	// Read the Fragment Shader code from the file
	char FragmentShaderCode[st.st_size+1];
	memset(FragmentShaderCode, 0, st.st_size+1);

	fp = fopen(fragment_file_path,"rb"); // read mode
	if( fp == NULL )
	{
		perror("Error while opening the file.\n");
		return 0;
	}
	c = 0;
	while( ( ch = fgetc(fp) ) != EOF )
		FragmentShaderCode[c++] = ch;
	fclose(fp);

	return LoadShaders(VertexShaderCode, FragmentShaderCode);
}
