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

#ifndef HALFTREE_H
#define HALFTREE_H

#include "math2d.h"
#include "VBO.h"
#include <vector>

class Render;

struct Deformer {
	virtual bool	deform(vec2 &p) = 0;
	virtual			~Deformer()	{}
};

struct color4;

struct TreeVert {
	vec2	v;
	vec2	n;
	float	lev;
	TreeVert()											{}
	TreeVert(const vec2 &av, const vec2 &an, float l): v(av), n(an), lev(l)	{}
};	

class HalfTree {
	enum InvalidateStatus {
		IS_VALID,
		IS_LAST_POINT,
		IS_ALL,
		IS_REBUILD
	};

	struct Node {
		vec2 pos, dir;
		float length, curLength;
		Node *left, *right, *parent;

				Node(const vec2 &p, const vec2 &d, float l, Node *par): pos(p), dir(d), length(l), curLength(0), left(0), right(0), parent(par) {}
				~Node();
		vec2	end()				{	return pos+dir*length;	}
	};

	rect	bounds;
	Deformer	*deformer;
	float	lengthFactor, lengthFactorDiv, angleFactor, angleFactorDiv;

	int		count, deep;
	float	length;
	Node	*root, *current;
	int		iterator, maxIterator;
	void	leftBranch();
	void	nextBranch();
	void	prevBranch();

	Node	*makeNode(const vec2 &pos, const vec2 &dir, float length, Node *parent);
	Node	*makeLeft(Node *n);
	Node	*makeRight(Node *n);

	std::vector<TreeVert>	verts;
	VBOVertex	vertsVBO;

	InvalidateStatus	invStatus;
	int			vertCount;
	TreeVert	vertSample;
	void	invalidate(InvalidateStatus invSt);

	void	buildDrawBuffer();
	void	buildDrawBufferLeft(Node *n, int idx, float level, TreeVert* verts);
	void	buildDrawBufferRight(Node *n, int idx, float level, TreeVert* verts);
	void	updateDrawBufferLastPoint();

	void	addBounds(HalfTree::Node *n);
	void	recalcBounds();
public:
			HalfTree(const vec2 &p, const vec2 &d, float l, float lengthFactor, float lengthFactorDiv, float angleFactor, float angleFactorDiv, Deformer *def=0);
	void	stepUp(float v);
	void	stepDown(float v);
	int		getCount()			{	return count;		}
	float	getLength();
	vec2	getPos()			{	return root->pos;	}
	vec2	getDir()			{	return root->dir;	}

	void	draw(Render *render);
	void	drawBounds(Render *render);

	void	invalidate()		{	invalidate(IS_REBUILD);	}
	static	void buildVBOIndex(VBOIndex &index);
};

#endif
