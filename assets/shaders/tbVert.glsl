#version 330 core

layout(location = 0) in vec2 vertexPosition_modelspace;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Color;

out vec2 oTexCoord;
out vec4 oColor;

uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(vertexPosition_modelspace,0,1);
	//oTexCoord = vec2(TexCoord.x,1.0-TexCoord.y);
	oTexCoord = TexCoord;
	oColor = Color;
}