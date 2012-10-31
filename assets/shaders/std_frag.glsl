#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform vec4 color;

void main() {
	gl_FragColor = color;
}
