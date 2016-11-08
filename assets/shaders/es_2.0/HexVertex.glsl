uniform float heightFactor;
uniform float widthFactor;
uniform vec2 center;

attribute vec4 Position;
attribute vec2 TexCoord;
attribute vec2 BarryCoord;

varying vec2 oTexCoord;
varying vec2 oBarryCoord;

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
