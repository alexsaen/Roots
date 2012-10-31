#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D 	tex0;
varying vec2 		v_texcoord;


float ridgedMF(vec2 p, int octaves, float lacunarity, float gain) {
	float sum = 0.0;
	float amplitude = 0.5;
	float frequency = 1.0;
	float prev = 1.0;
	float div = 0.0;

	for (int i = 0; i < octaves; i++) {
		float n = texture2D(tex0, p * frequency + vec2(0.0, float(octaves-i)/float(octaves)*0.5) ).x;
		sum += n * amplitude * prev;
		div += amplitude;
		prev = n;
		frequency *= lacunarity;
		amplitude *= gain;
	}

	return sum;
}

void main() {
	float c = max(0.5*ridgedMF( v_texcoord*0.25, 3, 1.9, 1.5)*1.25-0.25, 0.0);
	gl_FragColor = vec4(c*c, 0.0, c, 1.0);
}

