#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D 	tex0, tex1, tex2;
uniform vec4		color;

varying vec2 		v_texcoord;

float rand(vec2 co){
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}


vec3 grayScale(vec3 col) { 
//    	float d = dot(col, vec3(0.2125, 0.7154, 0.0721)); 
    float d = dot(col, vec3(0.33, 0.33, 0.33)); 
	return vec3(d, d, d);
}

void main() { 
	vec3 bg = texture2D(tex0, v_texcoord).xyz;
	if(color.a > 0.0)
		bg = bg * (1.0 - color.a) + ((grayScale(bg) * (0.7 + rand(v_texcoord)*0.3)) * color.rgb) * color.a;

	vec3 col = texture2D(tex1, v_texcoord).xyz;
	col += (col.gbr+col.brg)*0.3;

	vec3 c = texture2D(tex2, v_texcoord).rgb * 5.0;
	float t = clamp( (c.r+c.b+c.b)*2.0, 0.0, 1.0);               
	c = c.rgb * t + ((c.gbr+c.rgb)*0.5) * (1.0 - t);

	gl_FragColor.rgb = bg + col*color.a + c*color.a;
	gl_FragColor.a = 1.0;
}

