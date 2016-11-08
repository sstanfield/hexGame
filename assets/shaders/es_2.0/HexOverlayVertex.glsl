uniform float heightFactor;
uniform float widthFactor;
uniform vec2 center;

attribute vec4 Position;

void main()
{
	vec2 nc = vec2(center.x * widthFactor,
	               center.y * heightFactor);
	gl_Position = vec4((Position.x * widthFactor)+nc.x,
	                   (Position.y * heightFactor)+nc.y,
	                   Position.z,
	                   1.0);
}
