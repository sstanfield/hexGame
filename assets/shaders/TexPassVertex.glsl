#version 330 core

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;

out vec2 oTexCoord;

void main()
{
	gl_Position = Position;
	oTexCoord = TexCoord;
}
