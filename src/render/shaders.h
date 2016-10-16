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
#pragma once

#include <string>
#include <memory>

namespace hexgame { namespace render {

class Shader {
public:
	Shader(const std::string vertex_file_path,
           const std::string fragment_file_path);
	~Shader();

	unsigned int id();
	operator unsigned int() { return id(); }

	using s_ptr = std::shared_ptr<Shader>;
	using w_ptr = std::weak_ptr<Shader>;

private:
	unsigned int programID = 0;
	std::string name;
};

Shader::s_ptr LoadShadersFromFile(const std::string vertex_file_path,
                                  const std::string fragment_file_path);
void clearShaderCache();

} }
