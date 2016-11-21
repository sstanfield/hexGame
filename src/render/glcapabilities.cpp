#include <iostream>

#include "glcapabilities.h"
#include "gl_util.h"

using namespace hexgame;
using namespace hexgame::render;

GLCapabilities GLCapabilities::instance;

GLCapabilities *GLCapabilities::i() { return &instance; }

bool GLCapabilities::setCap(unsigned int cap, bool state) {
	bool ret = false;
	clearErrors();
	bool curState = false;
	if (caps.find(cap) != caps.end()) curState = caps[cap];
	if (curState == state) return true;
	state?glEnable(cap):glDisable(cap);
	if (!clearErrors()) {
		caps[cap] = state;
		ret = true;
	}
	return ret;
}

bool GLCapabilities::enable(unsigned int cap) {
	return setCap(cap, true);
}

bool GLCapabilities::disable(unsigned int cap) {
	return setCap(cap, false);
}

bool GLCapabilities::clearErrors() {
	bool ret = false;

	GLenum err = GL_NO_ERROR;
	while((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GL Error- "<<std::hex<<err<<"\n";
		ret = true;
	}
	return ret;
}

void GLCapabilities::getState(std::map<unsigned int, bool> &state) {
	state.clear();
	std::map<unsigned int, bool>::iterator it = caps.begin();
	while(it != caps.end()) {
		state[it->first] = it->second;
		it++;
	}
}

void GLCapabilities::setState(std::map<unsigned int, bool> &state) {
	std::map<unsigned int, bool>::iterator it = state.begin();
	while(it != state.end()) {
		setCap(it->first, it->second);
		it++;
	}
	it = caps.begin();
	while(it != caps.end()) {
		if (state.find(it->first) != state.end()) setCap(it->first, false);
		it++;
	}
}

GLCapabilities::GLCapabilities() {
	caps[GL_DITHER] = true;
#ifndef __EMSCRIPTEN__
	caps[GL_MULTISAMPLE] = true;
#endif
}

