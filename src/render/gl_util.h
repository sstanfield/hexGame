#include "render/pc/GL/glew.h"

#include <string>
#include <iostream>
#include <sstream>

namespace hexgame { namespace render {

inline std::string codeToString(GLenum code)
{
	switch (code) {
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
			// Given when an enumeration parameter is not a legal enumeration for that function. This is given only for local problems; if the spec allows the enumeration in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
			// Given when a value parameter is not a legal value for that function. This is only given for local problems; if the spec allows the value in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
			// Given when the set of state for a command is not legal for the parameters given to that command. It is also given for commands where combinations of parameters define what the legal parameters are.

		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
			// Given when a stack popping operation cannot be done because the stack is already at its lowest point.
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
			// Given when performing an operation that can allocate memory, and the memory cannot be allocated. The results of OpenGL functions that return this error are undefined; it is allowable for partial operations to happen.
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
			// Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete.
		case GL_CONTEXT_LOST:
			return "GL_CONTEXT_LOST";
			// Given if the OpenGL context has been lost, due to a graphics card reset.
		//case GL_TABLE_TOO_LARGE1:
			//Part of the ARB_imaging extension.
	}
	return "Unknown GL Error code";
}

inline void clearGLErrors() {
	GLenum err = GL_NO_ERROR;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "Clearing stale GL ERROR, Code: " << std::hex << err << ", " << codeToString(err) << std::endl;
	}
}

inline void doGLError(std::string root) {
	GLenum err = GL_NO_ERROR;
	if ((err = glGetError()) != GL_NO_ERROR) {
		std::stringstream ss;
		ss << ", Code: " << std::hex << err << ", " << codeToString(err);
		throw std::invalid_argument("GL Error "+root+" "+ss.str());
	}
}

}
}

