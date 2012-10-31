#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

attribute vec4 position;
attribute vec2 texcoord;
varying vec2 v_texcoord;

uniform vec2 offset;
uniform float scale;

void main()
{	
	gl_Position = position;
	v_texcoord = (texcoord + offset) * scale;	
}
