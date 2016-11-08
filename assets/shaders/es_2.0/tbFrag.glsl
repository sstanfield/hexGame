precision mediump float;

uniform sampler2D Texture0;

varying vec2 oTexCoord;
varying vec4 oColor;

void main()
{
	gl_FragColor = texture2D(Texture0, oTexCoord) * oColor;
}
