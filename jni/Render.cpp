/*  
	Copyright (c) 2012, Alexey Saenko
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/ 

#include "Render.h"
#include "RenderResource.h"

#include "opengl.h"
#include "World.h"
#include "Planet.h"
#include "Link.h"
#include "Genus.h"
#include "FBO.h"
#include "Shader.h"
#include "Texture.h"
#include "utils.h"

static std::vector<RenderResource*>	resources;

RenderResource::RenderResource() {
	resources.push_back(this);
}

RenderResource::~RenderResource() {
	vector_fast_remove(resources, this);
	release();
}

Render::Render(): scale(0.25f), transform(1.0f), curShader(0), animate(0), deform(0), noiseStride(3)
{}

Render::~Render() {
	release();
}

static Render *render = 0;
Render& Render::instance() {
	if(!render)
		render = new Render();
	return *render;
}

void Render::destroy() {
	if(render) {
		delete render;
		render = 0;
	}
}

void Render::release() {
	for(std::vector<RenderResource*>::iterator r = resources.begin(); r != resources.end(); ++r)
		(*r)->release();
}

void Render::reshape(int w, int h) {
	width = w;
	height = h;
	aspect = (float) width / (float) height;

	fbo.reshape(w, h);
	fboBackground.reshape(w, h);
	blurFbo1.reshape(w/2, h/2);
	blurFbo2.reshape(w/2, h/2);
	fboNoise.reshape(w/4, h/4);

	flatVertShader.load("flat_vert.glsl");

	blurFragShader.load("blur_frag.glsl");
	blurShaderProgram.attach(&flatVertShader, &blurFragShader); 
	blurShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	blurShaderProgram.bindAttrib(ATTRIB_TEXCOORD, "texcoord");
	blurShaderProgram.link();

	backgroundFragShader.load("background_frag.glsl"); 
	backgroundShaderProgram.attach(&flatVertShader, &backgroundFragShader);
	backgroundShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	backgroundShaderProgram.bindAttrib(ATTRIB_TEXCOORD, "texcoord");
	backgroundShaderProgram.link();

	noiseVertShader.load("noise_vert.glsl");
	noiseFragShader.load("noise_frag.glsl"); 
	noiseShaderProgram.attach(&noiseVertShader, &noiseFragShader);
	noiseShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	noiseShaderProgram.bindAttrib(ATTRIB_TEXCOORD, "texcoord");
	noiseShaderProgram.link();

	finalFragShader.load("final_frag.glsl");
	finalShaderProgram.attach(&flatVertShader, &finalFragShader);
	finalShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	finalShaderProgram.bindAttrib(ATTRIB_TEXCOORD, "texcoord");
	finalShaderProgram.link();

	finalFragShader2.load("final_frag2.glsl");
	finalShaderProgram2.attach(&flatVertShader, &finalFragShader2);
	finalShaderProgram2.bindAttrib(ATTRIB_POSITION, "position");
	finalShaderProgram2.bindAttrib(ATTRIB_TEXCOORD, "texcoord");
	finalShaderProgram2.link();

	treeVertShader.load("tree_vert.glsl"); 
	treeFragShader.load("tree_frag.glsl");
	treeShaderProgram.attach(&treeVertShader, &treeFragShader);
	treeShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	treeShaderProgram.bindAttrib(ATTRIB_NORMAL, "norm");
	treeShaderProgram.bindAttrib(ATTRIB_LEVEL, "level");
	treeShaderProgram.link();

	linkVertShader.load("link_vert.glsl"); 
	linkFragShader.load("link_frag.glsl");
	linkShaderProgram.attach(&linkVertShader, &linkFragShader);
	linkShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	linkShaderProgram.bindAttrib(ATTRIB_NORMAL, "norm");
	linkShaderProgram.bindAttrib(ATTRIB_LEVEL, "level");
	linkShaderProgram.link();

	planetVertShader.load("planet_vert.glsl"); 
	planetFragShader.load("planet_frag.glsl"); 
	planetShaderProgram.attach(&planetVertShader, &planetFragShader);
	planetShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	planetShaderProgram.bindAttrib(ATTRIB_TEXCOORD, "texcoord");
	planetShaderProgram.link();

	blackHoleVertShader.load("blackhole_vert.glsl"); 
	blackHoleFragShader.load("blackhole_frag.glsl"); 
	blackHoleShaderProgram.attach(&blackHoleVertShader, &blackHoleFragShader);
	blackHoleShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	blackHoleShaderProgram.bindAttrib(ATTRIB_TEXCOORD, "texcoord");
	blackHoleShaderProgram.link();

	boobleFragShader.load("booble_frag.glsl"); 
	boobleShaderProgram.attach(&planetVertShader, &boobleFragShader);
	boobleShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	boobleShaderProgram.bindAttrib(ATTRIB_TEXCOORD, "texcoord");
	boobleShaderProgram.link();

	stdVertShader.load("std_vert.glsl"); 
	stdFragShader.load("std_frag.glsl");
	stdShaderProgram.attach(&stdVertShader, &stdFragShader);
	stdShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	stdShaderProgram.link();

	fontVertShader.load("font_vert.glsl");
	fontFragShader.load("font_frag.glsl");
	fontShaderProgram.attach(&fontVertShader, &fontFragShader); 
	fontShaderProgram.bindAttrib(ATTRIB_POSITION, "position");
	fontShaderProgram.bindAttrib(ATTRIB_TEXCOORD, "texcoord");
	fontShaderProgram.link();

	generatePlanetTexture(64);
	generateCircleVerts();
//	glDisable( GL_DITHER );

	titleFont.init("assets/fonts/NEXTG___.TTF", 25);
	simpleFont.init("assets/fonts/Cuprum-Regular.ttf", 25);

	HalfTree::buildVBOIndex(plantVBOIndex);
	Link::buildVBOIndex(linkVBOIndex);
	noiseStride = 3;
}

static void fillRect(const vec2 *tv=0) {
	static const vec2 verts[4]  = { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1)  };
	static const vec2 tverts[4] = { vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)  };

	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(vec2), verts);
	glEnableVertexAttribArray(ATTRIB_TEXCOORD);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, false, sizeof(vec2), tv ? tv : tverts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(ATTRIB_POSITION);
	glDisableVertexAttribArray(ATTRIB_TEXCOORD);

}

void Render::setShader(ShaderProgram *sp) {
	if(curShader == sp)
		return;
	curShader = sp;
	if(curShader) 
		curShader->use();
	else
		ShaderProgram::unuse();
}

void Render::setColor(const color4 &c) {
	if(curShader)
		curShader->uniform(SU_COLOR, c);
}

void Render::setTreeShaderSeed(int seed) {
	treeShaderProgram.uniform(SU_INDEX, animate + seed);
}

void Render::setShaderSeed(float seed) {
	curShader->uniform(SU_INDEX, seed);
}

void Render::setDeform(bool t) {
	treeShaderProgram.uniform(SU_TIME, float(t ? deform : -1.0f) );
}

void Render::drawBegin(const vec2 &pos) {
	animate += 0.5f;
	if(animate > 1000)
		animate = 0;

	deform += 0.01f;

	glViewport( 0, 0, width, height );
	glClearColor(0, 0, 0, 0);
	glDisable(GL_DEPTH_TEST);

	glDisable(GL_BLEND);

//	pass 0, render noise
	noiseStride++;
	if(noiseStride>=2) {
		noiseStride = 0;
		glViewport( 0, 0, fboNoise.width, fboNoise.height );
		fboNoise.bind();

		setShader(&noiseShaderProgram);
		noiseShaderProgram.uniform("offset", (pos*scale)*0.05f);
		noiseShaderProgram.uniform("scale", 1.0f+(1.0f-scale)*0.2f);
		noiseShaderProgram.uniform(SU_TIME, deform*0.05f);
		float dw = aspect*0.5f;
		const vec2 tverts[4] = { vec2(-dw, -0.5f), vec2(dw, -0.5f), vec2(dw, 0.5f), vec2(-dw, 0.5f)  };
		fillRect(tverts);
		fboNoise.unbind();
	}

//	pass 1
	fbo.bind();

	glViewport( 0, 0, fbo.width, fbo.height );
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

}

void Render::draw(World &world) {
	draw(world, 0);
}

void Render::makeScreenshot(World &world) {
	draw(world, &fboBackground);
}

void Render::draw(World &world, FBO *renderTarget) {
	scale = world.getScale();
	vec2 pos = world.getPos();
	mat4 viewMat = mat4::get_ortho(-aspect, aspect, -1, 1);
	transform = viewMat * mat4::get_scale(scale, scale, scale) * mat4::get_translate(-pos.x, -pos.y, 0);
	drawBegin(pos);

	bounds=rect(vec2(-aspect, -1) /scale + pos, vec2(aspect, 1) / scale + pos);
	if(!planets.empty()) {
		setShader(&planetShaderProgram);
		planetShaderProgram.uniform(SU_TRANSFORM, transform);
		planetShaderProgram.uniform(SU_TEX0, 0);
		planetShaderProgram.uniform(SU_TEX1, 1);

		glActiveTexture(GL_TEXTURE0);
		glEnable( GL_TEXTURE_2D );
		planetTexture.bind();
		glActiveTexture(GL_TEXTURE1);
		glEnable( GL_TEXTURE_2D );
		fboNoise.bindTexture();

		for(std::vector<Planet*>::iterator p = planets.begin(); p!=planets.end(); ++p)
			(*p)->draw(this);
	
		glActiveTexture(GL_TEXTURE1);
		glDisable( GL_TEXTURE_2D );
		glActiveTexture(GL_TEXTURE0);
		glDisable( GL_TEXTURE_2D );
	}

	if(!blackHoles.empty()) {
		setShader(&blackHoleShaderProgram);
		blackHoleShaderProgram.uniform(SU_TRANSFORM, transform);
		blackHoleShaderProgram.uniform(SU_TEX0, 0);
		blackHoleShaderProgram.uniform(SU_TIME, deform);

		glEnable( GL_TEXTURE_2D );
		planetTexture.bind();

		for(std::vector<BlackHole*>::iterator h = blackHoles.begin(); h!=blackHoles.end(); ++h)
			(*h)->draw(this);
	
		glDisable( GL_TEXTURE_2D );
	}

	setShader(&treeShaderProgram);
	treeShaderProgram.uniform(SU_TRANSFORM, transform);

	glDisable(GL_BLEND);

	for(std::vector<Planet*>::iterator p = planets.begin(); p!=planets.end(); ++p) 
		(*p)->drawTrees(this);

//	glEnable(GL_BLEND);

	setShader(&linkShaderProgram);
	linkShaderProgram.uniform(SU_TRANSFORM, transform);
	linkShaderProgram.uniform(SU_INDEX, animate);

	for(std::vector<Planet*>::iterator p = planets.begin(); p!=planets.end(); ++p) 
		(*p)->drawTreeLinks(this);

	const color4 &titleColor = world.getTitleColor();
	if(titleColor.a > 0) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		beginFont(viewMat);
		float titleFontSize = 0.21f;
		setColor(titleColor);
		float w = titleFont.width(world.getLevelTitle());
		titleFont.draw(-w*titleFontSize*0.5f, 1-titleFontSize, titleFontSize, world.getLevelTitle());
		glDisable(GL_BLEND);
	}
	world.drawFlashText();
	drawEnd(renderTarget);
}

void Render::beginBooble(const mat4 &transform) {
	setShader(&boobleShaderProgram);
	boobleShaderProgram.uniform(SU_TRANSFORM, transform);
	boobleShaderProgram.uniform(SU_TEX0, 0);
	glActiveTexture(GL_TEXTURE0);
	glEnable( GL_TEXTURE_2D );
	planetTexture.bind();
}

void Render::drawEnd(FBO *renderTarget) {
//	glBlendFunc(GL_ONE, GL_ONE);

	fbo.unbind();
	glDisable(GL_BLEND);

//	pass 2
	glEnable(GL_TEXTURE_2D);

	setShader(&blurShaderProgram);
	blurShaderProgram.uniform(SU_TEX0, 0);
	blurShaderProgram.uniform("sampleOffset", vec2(1.0f/blurFbo1.width, 0.0f) );

	blurFbo1.bind();
	glViewport( 0, 0, blurFbo1.width, blurFbo1.height );
	fbo.bindTexture();
	fillRect();
	fbo.unbindTexture();

//	pass 3
	blurShaderProgram.uniform("sampleOffset", vec2(0.0f, 1.0f/blurFbo2.height) );

	blurFbo2.bind();
	blurFbo1.bindTexture();
	fillRect();
	blurFbo1.unbindTexture();
	blurFbo2.unbind();

// pass 4
	blurFbo1.bind();
	glViewport( 0, 0, blurFbo1.width, blurFbo1.height );
	setShader(&backgroundShaderProgram);
	backgroundShaderProgram.uniform(SU_TEX0, 0);
	fboNoise.bindTexture();
	fillRect();
	blurFbo1.unbind();

// pass 5
	if(renderTarget) {
		renderTarget->bind();
		glViewport( 0, 0, renderTarget->width, renderTarget->height );
	} else
		glViewport( 0, 0, width, height );

	glActiveTexture(GL_TEXTURE0);
	glEnable( GL_TEXTURE_2D );
	blurFbo1.bindTexture();

	glActiveTexture(GL_TEXTURE1);
	glEnable( GL_TEXTURE_2D );
	fbo.bindTexture();

	glActiveTexture(GL_TEXTURE2);
	glEnable( GL_TEXTURE_2D );
	blurFbo2.bindTexture();

	setShader(&finalShaderProgram);
	finalShaderProgram.uniform(SU_TEX0, 0);
	finalShaderProgram.uniform(SU_TEX1, 1);
	finalShaderProgram.uniform(SU_TEX2, 2);
//		finalShaderProgram.uniform(SU_COLOR, color4(0,0,0,0));
	finalShaderProgram.uniform("sampleOffset", vec2(1.0f/fbo.width*0.7f, 1.0f/fbo.height*0.7f) );

	fillRect();

	glActiveTexture(GL_TEXTURE2);
	glDisable( GL_TEXTURE_2D );
	Texture::unbind();
	glActiveTexture(GL_TEXTURE1);
	glDisable( GL_TEXTURE_2D );
	Texture::unbind();
	glActiveTexture(GL_TEXTURE0);
	glDisable( GL_TEXTURE_2D );
	Texture::unbind();

	if(renderTarget) {
		renderTarget->unbind();
		glViewport( 0, 0, width, height );
	} else {	
		setShader(&stdShaderProgram);
		stdShaderProgram.uniform(SU_TRANSFORM, transform);
	}

//	drawRect(bounds, color4(1,0,1,1));
//	for(std::vector<Planet*>::iterator p = planets.begin(); p!=planets.end(); ++p) 
//		(*p)->drawBounds(this);
}

void Render::drawChapteSShotBegin(const char *text, const color4 &tc) {
	mat4 viewMat;
	viewMat.ortho(-aspect, aspect, -1, 1); 

	fbo.bind();

	glViewport( 0, 0, fbo.width, fbo.height );
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	beginFont(viewMat);
	float titleFontSize = 0.21f;
	setColor(tc);
	float w = titleFont.width(text);
	titleFont.draw(-w*titleFontSize*0.5f, 0, titleFontSize, text);
}

void Render::drawChapteSShotEnd(const color4 &c) {
	fbo.unbind();
	glDisable(GL_BLEND);

//	pass 2
	glEnable(GL_TEXTURE_2D);

	setShader(&blurShaderProgram);
	blurShaderProgram.uniform(SU_TEX0, 0);
	blurShaderProgram.uniform("sampleOffset", vec2(1.0f/blurFbo1.width, 0.0f) );

	blurFbo1.bind();
	glViewport( 0, 0, blurFbo1.width, blurFbo1.height );
	fbo.bindTexture();
	fillRect();
	fbo.unbindTexture();

//	pass 3
	blurShaderProgram.uniform("sampleOffset", vec2(0.0f, 1.0f/blurFbo2.height) );

	blurFbo2.bind();
	blurFbo1.bindTexture();
	fillRect();
	blurFbo1.unbindTexture();
	blurFbo2.unbind();

/////////////////////

	glViewport( 0, 0, width, height );
	glActiveTexture(GL_TEXTURE0);
	glEnable( GL_TEXTURE_2D );
	fboBackground.bindTexture();

	glActiveTexture(GL_TEXTURE1);
	glEnable( GL_TEXTURE_2D );
	fbo.bindTexture();

	glActiveTexture(GL_TEXTURE2);
	glEnable( GL_TEXTURE_2D );
	blurFbo2.bindTexture();

	setShader(&finalShaderProgram2);
	finalShaderProgram2.uniform(SU_TEX0, 0);
	finalShaderProgram2.uniform(SU_TEX1, 1);
	finalShaderProgram2.uniform(SU_TEX2, 2);
	finalShaderProgram2.uniform(SU_COLOR, c);

	fillRect();

	glActiveTexture(GL_TEXTURE2);
	glDisable( GL_TEXTURE_2D );
	Texture::unbind();
	glActiveTexture(GL_TEXTURE1);
	glDisable( GL_TEXTURE_2D );
	Texture::unbind();
	glActiveTexture(GL_TEXTURE0);
	glDisable( GL_TEXTURE_2D );
	fboBackground.unbindTexture();

	setShader(0);
}


void Render::beginFont(const mat4 &transform) {
	setShader(&fontShaderProgram);
	fontShaderProgram.uniform(SU_TRANSFORM, transform);
	fontShaderProgram.uniform(SU_TEX0, 0);
}

void Render::setStdShader(const mat4 &transform) {
	setShader(&stdShaderProgram);
	stdShaderProgram.uniform(SU_TRANSFORM, transform);
}

void Render::drawCurrentRace(const color4& col) {
	mat4 tr = mat4::get_ortho(-aspect, aspect, -1, 1);
	stdShaderProgram.uniform(SU_TRANSFORM, tr);

	const vec2 pos(-aspect + 0.1f, 1 - 0.1f); 
	const float size = 0.05f;	
	static vec2 verts[] = { vec2(pos.x-size, pos.y-size), vec2(pos.x+size, pos.y-size), 
							vec2(pos.x+size, pos.y+size), vec2(pos.x-size, pos.y+size)  };

	setColor(col);
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(vec2), verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(ATTRIB_POSITION);
}

void Render::drawArrow(const vec2 &p1, const vec2 &p2, float arrowSize, const color4 &col) {
	vec2 dir = p2 - p1;
	dir.normalize();
	vec2 dp =  vec2(-dir.y, dir.x) * 0.5f;
	vec2 d1 = (- dir + dp) * arrowSize; 
	vec2 d2 = (- dir - dp) * arrowSize; 

	vec2 verts[] = { p2, p1, p2, p2+d1, p2, p2+d2 };
	setColor(col);

	glEnable(GL_BLEND);
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(vec2), &verts[0]);
	glDrawArrays(GL_LINES, 0, 6);
	glDisableVertexAttribArray(ATTRIB_POSITION);
	glDisable(GL_BLEND);
}

void Render::generatePlanetTexture(int size) {
	std::vector<unsigned char> buffer(size*size*4);
	memset(&buffer[0], 0, buffer.size());
	int size2 = size/2;
	int cidx = size2*size+size2;
	for(int y=0; y<size2; ++y)
		for(int x=0; x<size2; ++x) {
			float d = std::min(sqrtf(float(x*x+y*y)) / size2, 1.0f);
			int idx0 = cidx+y*size+x;
			int idx1 = cidx-1+y*size-x;
			int idx2 = cidx-size-y*size+x;
			int idx3 = cidx-size-1-y*size-x;
			float xx = float(x+0.5f)/size;
			float yy = float(y+0.5f)/size;
			if(d<1.0f && d>0.0f) {
				float w = (1.0f-cosf(d*PI_2))/d;
				xx*=w;
				yy*=w;
			}
			buffer[idx0*4] = buffer[idx2*4] = (unsigned char)((0.5f+xx)*255.0f);
			buffer[idx1*4] = buffer[idx3*4] = (unsigned char)((0.5f-xx)*255.0f);
			buffer[idx0*4+1] = buffer[idx1*4+1] = (unsigned char)((0.5f+yy)*255.0f);
			buffer[idx2*4+1] = buffer[idx3*4+1] = (unsigned char)((0.5f-yy)*255.0f);

			unsigned char c = (unsigned char)(d*d*d*d*d * 255.0f);
			buffer[idx0*4+2] = buffer[idx1*4+2] = buffer[idx2*4+2] = buffer[idx3*4+2] = c;

//			unsigned char cc = (unsigned char)(std::max(std::min((0.8f-8.0f*d*d), 1.0f), 0.0f) * 255.0f);
//			buffer[idx0*4+3] = buffer[idx1*4+3] = buffer[idx2*4+3] = buffer[idx3*4+3] = cc;
		}

	for(int y=0; y<size; ++y)
		for(int x=0; x<size; ++x) {
			vec2 position = vec2(x/float(size-1), y/float(size-1))* 2.0f - vec2(1.0, 1.0);
			float th = atan2f(position.y, position.x) / (2.0f*PI);
			float dd = position.length();
			float d = dd > 0 ? 1.0f/dd : 0;
			float a = 0.5f + 0.5f * cosf((th + d) * 2.0f*PI);
			int idx = (y*size+x) * 4;
			buffer[idx + 3] = (unsigned char)(a*a * dd*dd * (255 - buffer[idx + 2]));
		}

	planetTexture.init(&buffer[0], size, size, 4);
}

static const int circleVertCount = 100+1;	// TODO its will be depended to resolution of screen
void Render::generateCircleVerts() {
	std::vector<V2TV2Vert> circleVerts(circleVertCount);
	circleVerts[0] = V2TV2Vert(vec2(0, 0), vec2(0.5f, 0.5f));
	for(size_t i = 0; i < circleVertCount-1; ++i) {
		float a = 2.0f * PI * i / (circleVertCount-2);
		vec2 delta(cosf(a), sinf(a));
		circleVerts[i+1] = V2TV2Vert(delta, vec2(0.5f, 0.5f) + delta * 0.5f);
	}

	circleVBO.bind();
	circleVBO.setData(circleVerts.size() * sizeof(V2TV2Vert), GL_STATIC_DRAW, &circleVerts[0]);
	circleVBO.unbind();
}

void Render::drawCircle(const mat4& transform, const vec2& p, float r) {
	mat4 t = transform * mat4::get_translate(p.x, p.y, 0) * mat4::get_scale(r, r, r);
	curShader->uniform(SU_TRANSFORM, t);
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glEnableVertexAttribArray(ATTRIB_TEXCOORD);
	circleVBO.bind();
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(V2TV2Vert), ((V2TV2Vert*)0)->v);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, false, sizeof(V2TV2Vert), ((V2TV2Vert*)0)->tv);
	glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertCount);
	glDisableVertexAttribArray(ATTRIB_POSITION);
	glDisableVertexAttribArray(ATTRIB_TEXCOORD);
	circleVBO.unbind();
}

void Render::drawUnlink(const vec2 &p1, const vec2 &p2, float size) {
	vec2 dir = p2 - p1;
	dir.normalize();
	vec2 dp =  vec2(-dir.y, dir.x);
	vec2 d1 = (- dir + dp) * size; 
	vec2 d2 = (- dir - dp) * size; 

	vec2 verts[] = { p2, p1, p2-d1, p2+d1, p2-d2, p2+d2 };
	setColor(color4(1,0,0,1));

	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(vec2), &verts[0]);
	glDrawArrays(GL_LINES, 0, 6);
	glDisableVertexAttribArray(ATTRIB_POSITION);

}

void Render::drawRect(const rect &r, const color4& c) {
	setColor(c);
	vec2 verts[] = { r.lb, vec2(r.lb.x, r.rt.y), r.rt, vec2(r.rt.x, r.lb.y) };
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(vec2), &verts[0]);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
//	static unsigned char idxes[] = { 0, 2, 1, 3 };
//	glDrawElements(GL_LINES, 4, GL_UNSIGNED_BYTE, idxes);
	glDisableVertexAttribArray(ATTRIB_POSITION);
}

void Render::fade(float v) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	setShader(&stdShaderProgram);
	stdShaderProgram.uniform(SU_TRANSFORM, mat4(1.0f));
	stdShaderProgram.uniform(SU_COLOR, color4(0,0,0,v));

	fillRect();
	glDisable(GL_BLEND);
}


