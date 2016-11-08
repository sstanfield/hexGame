attribute vec4 Position;
attribute vec2 TexCoord;

varying vec2 oTexCoord;

void main()
{
	gl_Position = Position;
	oTexCoord = TexCoord;
}
