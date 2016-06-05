#version 330 core

uniform sampler2D Texture0;
uniform vec4 textColor;

in vec2 oTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 texcolor = texture2D(Texture0, oTexCoord);
	if (texcolor.r >= .48) {
		float alpha = smoothstep(.48, .52, texcolor.r);
		if (texcolor.r < .51) {
			outColor.r = 0;
			outColor.g = 0;
			outColor.b = 0;
			outColor.a = alpha * textColor.a;
		} else {
			outColor.r = textColor.r;
			outColor.g = textColor.g;
			outColor.b = textColor.b;
			outColor.a = alpha * textColor.a;
		}
	} else outColor.a = 0;
}
