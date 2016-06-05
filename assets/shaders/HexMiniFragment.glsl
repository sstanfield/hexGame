#version 330 core

uniform vec4 tileColor;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = tileColor;
}
