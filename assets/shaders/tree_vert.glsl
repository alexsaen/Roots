#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

attribute vec4 position;
attribute vec4 norm;
attribute float level;

uniform float time;
uniform mat4 transform;
uniform float index;

varying float lev;

void main() {
	lev = 1.0 - 0.5 * min(0.1 * abs( mod(10000.0 - level + index, 50.0) - 25.0), 1.0);
	vec4 p = position;

	if(time >= 0.0) {
		float x = position.x * 128.0;
		float y = position.y * 128.0;
		float d = 0.5 * cos(time+x / (128.0 + 32.0 * cos(y / 64.0 + time))) * cos(y / (320.0 + 16.0 * sin(x / 64.0 )));
		p.xy += norm.xy * (d * level*level*0.002);	
	}

	gl_Position = transform * p;
}

