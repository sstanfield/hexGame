precision mediump float;

uniform sampler2D Texture0;
uniform int useBorder;
uniform float borderThickness;
uniform vec4 borderColor;

varying vec2 oTexCoord;
varying vec2 oBarryCoord;

void main()
{
	vec4 tilecolor = texture2D(Texture0, oTexCoord);
	if (useBorder == 1 && oBarryCoord.y > borderThickness) {
		gl_FragColor = borderColor;
	} else {
		gl_FragColor = tilecolor;
	}
}
