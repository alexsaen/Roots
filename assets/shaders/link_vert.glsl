#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

attribute vec4 position;
attribute vec4 norm;
attribute float level;

uniform mat4 transform;
uniform float  index;

varying float lev;

void main() {
	float l = (0.05 * abs(mod(10000.0 - level + index, 40.0) - 20.0));
	lev = 0.75 + 0.25 * l;
	vec4 p = position;
	p.xy += norm.xy * (l * l * l * 3.0 - 0.75);
	gl_Position = transform * p;
}

