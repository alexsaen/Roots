#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform vec4 color;

varying float lev;

void main() {
	gl_FragColor = color * lev;
	gl_FragColor.a = 1.0;
}
