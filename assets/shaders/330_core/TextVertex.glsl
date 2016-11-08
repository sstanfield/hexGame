#version 330 core

uniform float scalex;
uniform float scaley;
uniform float xpos;
uniform float ypos;
uniform float xoffset;
uniform float yoffset;
uniform float zoffset;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;

out vec2 oTexCoord;

void main()
{
	gl_Position = vec4((Position.x * scalex) + xpos + (xoffset),
	                   (Position.y * scaley) + ypos + yoffset,
	                   Position.z + zoffset,
	                   1.0);
	oTexCoord = TexCoord;
}
