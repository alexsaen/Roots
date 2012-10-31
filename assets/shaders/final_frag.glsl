#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D 	tex0, tex1, tex2;
uniform vec2		sampleOffset;
varying vec2 		v_texcoord;

void main() { 
	vec3 bg = texture2D(tex0, v_texcoord).xyz;

	vec3 col = texture2D(tex1, v_texcoord).xyz;
	col += (col.gbr+col.brg)*0.3;

	vec3 c = texture2D(tex2, v_texcoord).rgb * 5.0;
	float t = clamp( (c.r+c.b+c.b)*2.0, 0.0, 1.0);               
	c = c.rgb * t + ((c.gbr+c.rgb)*0.5) * (1.0 - t);

	gl_FragColor.rgb = bg + col + c;
	gl_FragColor.a = 1.0;
}

