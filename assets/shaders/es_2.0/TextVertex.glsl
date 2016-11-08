uniform float scalex;
uniform float scaley;
uniform float xpos;
uniform float ypos;
uniform float xoffset;
uniform float yoffset;
uniform float zoffset;

attribute vec3 Position;
attribute vec2 TexCoord;

varying vec2 oTexCoord;

void main()
{
	gl_Position = vec4((Position.x * scalex) + xpos + (xoffset),
	                   (Position.y * scaley) + ypos + yoffset,
	                   Position.z + zoffset,
	                   1.0);
	oTexCoord = TexCoord;
}
