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

#ifndef RENDER_H
#define RENDER_H

#include "math2d.h"
#include "color.h"
#include "VBO.h"
#include "FBO.h"
#include "Shader.h"
#include "Texture.h"
#include "Font.h"
#include <vector>

class World;
class RenderResource;

static const unsigned int ATTRIB_POSITION = 0;
static const unsigned int ATTRIB_TEXCOORD = 1;
static const unsigned int ATTRIB_LEVEL	  = 6;
static const unsigned int ATTRIB_NORMAL	  = 7;

struct V2TV2Vert {
	vec2	v, tv;
	V2TV2Vert()													{}
	V2TV2Vert(const vec2& av, const vec2& atv): v(av), tv(atv)	{}
};

class Render {
	friend class RenderResource;
	int		width, height;
	float	aspect, scale;
	rect	bounds;

	VBOVertex		circleVBO;
	VBOIndex		plantVBOIndex, linkVBOIndex;
	FBO				fbo, blurFbo1, blurFbo2, fboNoise, fboBackground;
	VertexShader	flatVertShader, stdVertShader, treeVertShader, linkVertShader, planetVertShader, noiseVertShader, blackHoleVertShader, fontVertShader;
	FragmentShader	blurFragShader, finalFragShader, finalFragShader2, stdFragShader, treeFragShader, linkFragShader, backgroundFragShader, planetFragShader, noiseFragShader, blackHoleFragShader, fontFragShader, boobleFragShader;
	ShaderProgram	blurShaderProgram, finalShaderProgram, finalShaderProgram2, stdShaderProgram, treeShaderProgram, linkShaderProgram, backgroundShaderProgram, planetShaderProgram, noiseShaderProgram, blackHoleShaderProgram, fontShaderProgram, boobleShaderProgram;
	
	mat4			transform;
	ShaderProgram	*curShader;
	float			animate, deform;
	Texture			planetTexture;
	Font			titleFont, simpleFont;
	int				noiseStride;

	void	generatePlanetTexture(int size);
	void	generateCircleVerts();

	void	draw(World &world, FBO *renderTarget);

					Render();
	virtual			~Render();
public:

	static	Render	&instance();
	static	void	destroy();

			void	reshape(int w, int h);
	static	void	release();

	void	draw(World &world);
	void	makeScreenshot(World &world);
	void	drawChapteSShotBegin(const char *text, const color4 &tc);
	void	drawChapteSShotEnd(const color4 &c);

	void	drawBegin(const vec2 &pos = vec2(0.0f));
	void	drawEnd(FBO *renderTarget=0);

	void	beginBooble(const mat4 &transform);

	void	setStdShader(const mat4 &transform);
	void	beginFont(const mat4 &transform);
	Font&	getFont()														{	return titleFont;	}
	Font&	getSimpleFont()													{	return simpleFont;	}
	float	getAspect()														{	return aspect;		}

	void	setShader(ShaderProgram *sp);
	void	setColor(const color4 &c);
	void	setTreeShaderSeed(int seed);
	void	setShaderSeed(float seed);
	void	setDeform(bool t);
			
	void	drawArrow(const vec2 &p1, const vec2 &p2, float arrowSize, const color4 &col);
	void	drawUnlink(const vec2 &p1, const vec2 &p2, float size);
	void	drawCurrentRace(const color4& col);
	void	drawCircle(const mat4& transform, const vec2& p, float r);
	void	drawCircle(const vec2& p, float r)								{		drawCircle(transform, p, r);	}

	const	rect &getBounds()					{	return bounds;	}	
	void	drawRect(const rect &r, const color4& c);

	void	fade(float v);

	void	bindPlantVBOIndex()					{	plantVBOIndex.bind();		}
	void	bindLinkVBOIndex()					{	linkVBOIndex.bind();		}
};

#endif 
