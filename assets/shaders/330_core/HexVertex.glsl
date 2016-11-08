#version 330 core

uniform float heightFactor;
uniform float widthFactor;
uniform vec2 center;

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec2 BarryCoord;

out vec2 oTexCoord;
out vec2 oBarryCoord;

void main()
{
	vec2 nc = vec2(center.x * widthFactor,
	               center.y * heightFactor);
	gl_Position = vec4((Position.x * widthFactor)+nc.x,
	                   (Position.y * heightFactor)+nc.y,
	                   Position.z,
	                   1.0);

	oTexCoord = TexCoord;
	oBarryCoord = BarryCoord;
}
