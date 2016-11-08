attribute vec2 vertexPosition_modelspace;
attribute vec2 TexCoord;
attribute vec4 Color;

varying vec2 oTexCoord;
varying vec4 oColor;

uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(vertexPosition_modelspace,0,1);
	//oTexCoord = vec2(TexCoord.x,1.0-TexCoord.y);
	oTexCoord = TexCoord;
	oColor = Color;
}
