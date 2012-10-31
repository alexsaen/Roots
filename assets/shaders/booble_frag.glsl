#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D 	tex0;
varying vec2 		texcoord1;

void main(void) {
	float b = texture2D(tex0, texcoord1 ).z;
	vec3 c = vec3(0.5, 0.05, 1.0) * b;
	gl_FragColor = vec4(c * 0.4, 1.0);
}

