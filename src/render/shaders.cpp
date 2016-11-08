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
#include "c_raii.h"
#include "GL/glew.h"
#include "render/gl_util.h"

#include <sys/stat.h>
#include <unordered_map>
#include <vector>
#include <iostream>

namespace hexgame { namespace render {

static GLuint LoadShaders(const std::vector<char>& vertex_shader,
                   const std::vector<char>& fragment_shader) {
	clearGLErrors();
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	const char *vd = vertex_shader.data();
	glShaderSource(VertexShaderID, 1, &vd, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (!Result) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1, 0);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, VertexShaderErrorMessage.data());
		std::cout <<"Unable to compile vertex shader: "+std::string(VertexShaderErrorMessage.data());
		throw std::invalid_argument("Unable to compile vertex shader: "+std::string(VertexShaderErrorMessage.data()));
	}
	doGLError("Failed on vertex compile");

	// Compile Fragment Shader
	const char *fd = fragment_shader.data();
	glShaderSource(FragmentShaderID, 1, &fd , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (!Result) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1, 0);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, FragmentShaderErrorMessage.data());
		std::cout <<"Unable to compile fragment shader: "+std::string(FragmentShaderErrorMessage.data());
		throw std::invalid_argument("Unable to compile fragment shader: "+std::string(FragmentShaderErrorMessage.data()));
	}	
	doGLError("Failed on fragment compile");

	// Link the program
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (!Result) {
		std::vector<char> ProgramErrorMessage(InfoLogLength+1, 0);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage.data());
		std::cout <<"Unable to link shader: "+std::string(ProgramErrorMessage.data());
		throw std::invalid_argument("Unable to link shader: "+std::string(ProgramErrorMessage.data()));
	}
	doGLError("Failed on link");

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	doGLError("Failed on cleanup");

	return ProgramID;
}

static std::unordered_map<std::string, Shader::w_ptr> shaderMap;

Shader::Shader(const std::string vertex_file_path,
               const std::string fragment_file_path) {
	std::cout << "Compiling shaders : " << vertex_file_path << ", " << fragment_file_path << "\n";
	struct stat st;
	if (stat(vertex_file_path.c_str(), &st) == -1) {
		throw std::system_error(std::error_code(errno, std::system_category()),
		                        "Unable to stat "+vertex_file_path);
	}
	// Read the Vertex Shader code from the file
	std::vector<char> VertexShaderCode(st.st_size+1, 0);

	char ch;
	{
		CFile file(vertex_file_path, "rb"); // read mode
		int c = 0;
		while( ( ch = fgetc(file.fp) ) != EOF )
			VertexShaderCode[c++] = ch;
	}

	if (stat(fragment_file_path.c_str(), &st) == -1) {
		throw std::system_error(std::error_code(errno, std::system_category()),
		                        "Unable to stat "+fragment_file_path);
	}
	// Read the Fragment Shader code from the file
	std::vector<char> FragmentShaderCode(st.st_size+1, 0);

	{
		CFile file(fragment_file_path, "rb"); // read mode
		int c = 0;
		while( ( ch = fgetc(file.fp) ) != EOF )
			FragmentShaderCode[c++] = ch;
	}

	programID = LoadShaders(VertexShaderCode, FragmentShaderCode);
	name = vertex_file_path+":"+fragment_file_path;
}

Shader::~Shader() {
	std::cout << "Deleting shader program: " << name << "\n";
	if (programID > 0) glDeleteProgram(programID);
}

unsigned int Shader::id()
{
	return programID;
}

Shader::s_ptr LoadShadersFromFile(const std::string vertex_file_path,
                                  const std::string fragment_file_path)
{
	std::string key = vertex_file_path+fragment_file_path;
	if (shaderMap.count(key) > 0) {
		std::shared_ptr<Shader> ptr = shaderMap[key].lock();
		if (ptr) return ptr;
		else shaderMap[key].reset();
	}
	auto ptr = std::make_shared<Shader>(vertex_file_path, fragment_file_path);
	shaderMap[key] = std::weak_ptr<Shader>(ptr);
	return ptr;
}

void clearShaderCache()
{
	std::cout << "Clearing shader cache\n";
	shaderMap.clear();
}

} }

