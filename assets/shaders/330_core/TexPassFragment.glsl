#version 330 core

uniform sampler2D Texture0;

in vec2 oTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = texture(Texture0, oTexCoord);
}
