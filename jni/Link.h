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

#ifndef LINK_H
#define LINK_H

#include "math2d.h"
#include "VBO.h"
#include "HalfTree.h"
#include <vector>

class Planet;
class Tree;
class Render;

class Link {
friend class Tree;
friend class Mind;
	Planet	*target;
	Tree	*parent, *leech;
	vec2	dir, end, current;
	rect	bounds;
	float	dist, length;
	int		drawIndex, lastRecalcBounds, lastBuildBufferPointSize, age;
	std::vector<vec2> points;
	std::vector<TreeVert> verts;

	enum LinkState {
		LS_GROWING,
		LS_NORMAL,
		LS_CUTTING,
		LS_DIED
	};
	LinkState state;	
	void	setLeech(Tree *t);
	void	recalcBounds();

	VBOVertex	vertsVBO;	
	void	buildDrawBuffer();
public:
			Link(Tree *par, Planet *p);
			~Link();
	Tree*	getParent()				{	return parent;	}
	void	stepUp(float v);
	void	stepDown(float v);
	void	cut();
	void	draw(Render *render);
	void	drawBounds(Render *render);

			void	invalidate()	{	lastBuildBufferPointSize = 0; }
	static	void	buildVBOIndex(VBOIndex &index);

};

#endif
