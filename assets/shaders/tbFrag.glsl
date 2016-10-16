#version 330 core

uniform sampler2D Texture0;
uniform float opacity;

in vec2 oTexCoord;
in vec4 oColor;

layout(location = 0) out vec4 outColor;

void main()
{
//	outColor = vec4(1,0,0,1);//texture2D(Texture0, oTexCoord);
	//outColor = texture2D(Texture0, oTexCoord) * opacity;
	outColor = texture(Texture0, oTexCoord) * oColor;
	//vec4 tmp = texture2D(Texture0, oTexCoord);
	//outColor = vec4(tmp.xyz, opacity);
}
