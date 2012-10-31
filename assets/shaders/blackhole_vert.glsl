#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

attribute vec4 position;
attribute vec4 texcoord;
uniform mat4 transform;
uniform float time;

varying vec2 texcoord1;

void main()
{	
	float cs = cos(time);
	float sn = sin(time);
	mat2 rotation = mat2(
		vec2( cs,  sn),
		vec2(-sn,  cs)
	);

	gl_Position = transform * position;
	texcoord1 = rotation * (texcoord.xy-vec2(0.5)) + vec2(0.5);	
}
