#version 330 core

uniform sampler2D Texture0;
uniform int useBorder;
uniform float borderThickness;
uniform vec4 borderColor;

in vec2 oTexCoord;
in vec2 oBarryCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 tilecolor = texture(Texture0, oTexCoord);
	if (useBorder == 1 && oBarryCoord.y > borderThickness) {
		outColor = borderColor;
	} else {
		outColor = tilecolor;
	}
}
