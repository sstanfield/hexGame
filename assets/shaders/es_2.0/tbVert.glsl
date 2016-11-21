attribute vec2 Position;
attribute vec2 TexCoord;
attribute vec4 Color;

varying vec2 oTexCoord;
varying vec4 oColor;

uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(Position,0,1);
	oTexCoord = TexCoord;
	oColor = Color;
}
