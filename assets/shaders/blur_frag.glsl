#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D	tex0;
uniform vec2		sampleOffset;
varying vec2 		v_texcoord;

void main()
{ 
	gl_FragColor.rgb = ((((((((((((((((((( 
		   	texture2D( tex0, v_texcoord + sampleOffset*10.0).rgb * 0.009167927656011385
		  + texture2D( tex0, v_texcoord - sampleOffset*10.0).rgb * 0.009167927656011385)
		  + texture2D( tex0, v_texcoord + sampleOffset*9.0).rgb * 0.014053461291849008)
		  + texture2D( tex0, v_texcoord - sampleOffset*9.0).rgb * 0.014053461291849008)
		  + texture2D( tex0, v_texcoord + sampleOffset*8.0).rgb * 0.020595286319257885)
		  + texture2D( tex0, v_texcoord - sampleOffset*8.0).rgb * 0.020595286319257885)
		  + texture2D( tex0, v_texcoord + sampleOffset*7.0).rgb * 0.028855245532226279)
		  + texture2D( tex0, v_texcoord - sampleOffset*7.0).rgb * 0.028855245532226279)
		  + texture2D( tex0, v_texcoord + sampleOffset*6.0).rgb * 0.038650411513543079)
		  + texture2D( tex0, v_texcoord - sampleOffset*6.0).rgb * 0.038650411513543079)
		  + texture2D( tex0, v_texcoord + sampleOffset*5.0).rgb * 0.049494378859311121)
		  + texture2D( tex0, v_texcoord - sampleOffset*5.0).rgb * 0.049494378859311121)
		  + texture2D( tex0, v_texcoord + sampleOffset*4.0).rgb * 0.060594058578763092)
		  + texture2D( tex0, v_texcoord - sampleOffset*4.0).rgb * 0.060594058578763092)
		  + texture2D( tex0, v_texcoord + sampleOffset*3.0).rgb * 0.070921288047096992)
		  + texture2D( tex0, v_texcoord - sampleOffset*3.0).rgb * 0.070921288047096992)
		  + texture2D( tex0, v_texcoord + sampleOffset*2.0).rgb * 0.079358891804948081)
		  + texture2D( tex0, v_texcoord - sampleOffset*2.0).rgb * 0.079358891804948081)
		  + texture2D( tex0, v_texcoord + sampleOffset*1.0).rgb * 0.084895951965930902)
		  + texture2D( tex0, v_texcoord - sampleOffset*1.0).rgb * 0.084895951965930902)
		  + texture2D( tex0, v_texcoord).rgb * 0.086826196862124602; 

	gl_FragColor.a = 1.0;
}
