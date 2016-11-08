uniform float heightFactor;
uniform float widthFactor;
uniform float centerhFactor;
uniform float centerwFactor;
uniform vec2 center;

attribute vec4 Position;

void main()
{
	vec2 nc = vec2(center.x * centerwFactor,
	               center.y * centerhFactor);
	gl_Position = vec4((Position.x * widthFactor)+nc.x,
	                   (Position.y * heightFactor)+nc.y,
	                   -.1,
	                   1.0);
}
