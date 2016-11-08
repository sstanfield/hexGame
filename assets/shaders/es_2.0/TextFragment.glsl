precision mediump float;

uniform sampler2D Texture0;
uniform vec4 textColor;

varying vec2 oTexCoord;

void main()
{
	vec4 texcolor = texture2D(Texture0, oTexCoord);
	if (texcolor.r >= .48) {
		float alpha = smoothstep(.48, .52, texcolor.r);
		if (texcolor.r < .51) {
			gl_FragColor.r = 0;
			gl_FragColor.g = 0;
			gl_FragColor.b = 0;
			gl_FragColor.a = alpha * textColor.a;
		} else {
			gl_FragColor.r = textColor.r;
			gl_FragColor.g = textColor.g;
			gl_FragColor.b = textColor.b;
			gl_FragColor.a = alpha * textColor.a;
		}
	} else gl_FragColor.a = 0;
}
