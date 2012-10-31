#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D 	tex0;
uniform sampler2D 	tex1;
uniform float 		index;

varying vec2 		texcoord1;

void main(void) {
	vec4 p = texture2D(tex0, texcoord1 );

	float r = max(texture2D(tex1, p.xy*0.5+vec2(fract(index*23.0)*0.5, fract(index*7.0)*0.5)  ).x - 0.5, 0.0);
	float g = max(texture2D(tex1, p.xy*0.5+vec2(fract(index*17.0)*0.5, fract(index*13.0)*0.5) ).x - 0.5, 0.0);
	float b = max(texture2D(tex1, p.xy*0.5+vec2(fract(index*3.0)*0.5,  fract(index*29.0)*0.5) ).x - 0.5, 0.0);
	vec3 c = vec3(r, g, b)*0.7*(1.0-p.z) + vec3(p.z*0.25, p.z*0.25, p.z)*0.5;

	gl_FragColor = vec4(c, 1.0);
}

