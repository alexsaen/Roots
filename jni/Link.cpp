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

#include "Link.h"
#include "World.h"
#include "Planet.h"
#include "Tree.h"
#include "opengl.h"
#include "rand.h"
#include <algorithm>

static const int RECALC_BOUNDS_STRIDE = 10;

Link::Link(Tree *par, Planet *t): parent(par), leech(0), target(t), length(0), drawIndex(0),lastRecalcBounds(0), lastBuildBufferPointSize(0), age(0), verts(250) {
	current = parent->getPos();
	dir = parent->getDir();
	end = target->getGrowingPoint(parent->getRace(), current - target->getPos());
	dist = (end - current).length();
	points.push_back(current);
	bounds.add(current);
	state = LS_GROWING;
}

Link::~Link() {
}

void Link::stepDown(float v) {
	length -= v;
	while(points.size() > 1) {
		vec2 d = points.back() - points[points.size()-2]; 
		float l = d.length();
		if(l > v) {
			points.back() -= d * (v/l);
			return;
		} else {
			points.pop_back();
			if(lastRecalcBounds - points.size() > RECALC_BOUNDS_STRIDE) {
				lastRecalcBounds = points.size();
				recalcBounds();
			}
			v -= l;
		}
	}
	length = 0;
	points.clear();
	state = LS_DIED;
}

void Link::recalcBounds() {
	bounds = rect();
	for(std::vector<vec2>::iterator p = points.begin(); p!=points.end(); ++p)
		bounds.add(*p);
}

void Link::stepUp(float v) {
	vec2 dt = end - current;
	float dts = dt.normalize();
	if(dts <= 2*v) {
		length += dts;
		points.push_back(end);
		bounds.add(end);
		state = LS_NORMAL;
		for(std::vector<Tree*>::iterator t = target->trees.begin(); t != target->trees.end(); ++t)
			if((*t)->getRace() == parent->getRace()) {
				setLeech(*t);
				return;
			}
		setLeech( Tree::build(parent->getRace(), target, 0, randVec2()) );
		return;
	}

	static float a = 0;
	a += PI * v *0.5f;
	if(a>2*PI) 
		a -= 2*PI;
	float df = std::min(1.0f, dts/(target->radius*4.0f)); 
	mat2 m = mat2::get_rotate( sinf(a)*deg2rad(20)*df );
	dt = m*dt;

	dir += dt * 0.5f;
	dir.normalize();

	vec2 pos = current + dir * v;
	bool recalc = false;
	for(std::vector<PlanetObject*>::iterator p = planetObjects.begin(); p != planetObjects.end(); ++p) {
		PlanetObject *obj = *p;
		vec2 delta = pos - obj->getPos();
		float d = delta.normalize();
		float r = obj->getRadius();
		const float forceFactor = 2.0f;
		float forceDist = forceFactor*r;
		if(d > forceDist)
			continue;
		float force = (forceDist - d)/forceDist * v;
		if(obj == target) {
			vec2 dr = end - obj->getPos();
			dr.normalize();
			pos += delta * (std::max(0.0f, 0.5f+ dr * dir) * force);
		} else
			pos += delta * (force*1.25f);
		recalc = true;
	}
	if(recalc) {
		dir = pos - current;
		dir.normalize();

		length += v;
		current += dir*v;
	} else {
		length += (pos-current).length();
		current = pos;
	}

	points.push_back(current);
	bounds.add(current);
	lastRecalcBounds = points.size();
}

void Link::setLeech(Tree *t) {
	leech = t;
	t->addSeeder(this);
}

void Link::cut() {
	if(state == LS_DIED || state == LS_CUTTING)
		return;

	if(target) {
		if(state == LS_GROWING) {	// check black list
			float dist = points.empty() ? (points.back() - end).length() : (parent->getPos()-end).length();
			dist += length;
			parent->getPlanet()->checkBlackList(parent->getRace(), target, dist);
			target->checkBlackList(parent->getRace(), parent->getPlanet(), dist);
		}

		target->onUnlinkTarget(this);
		target = 0;
	}

	if(leech) {
		leech->removeSeeder(this);
		leech = 0;
	}
	state = LS_CUTTING;
}

void Link::buildDrawBuffer() {
	if((int)points.size() > lastBuildBufferPointSize) {
		bool setAllData = lastBuildBufferPointSize == 0;
		if(verts.size() < points.size()*2) {
			verts.resize( std::max(verts.size()+verts.size()/2, points.size()*2) );
			setAllData = true;
		}

		const float linkWidth = 0.005f;
		vec2 n;
		
		size_t startIndex = std::max(0, lastBuildBufferPointSize-1);
		for(size_t i = startIndex; i<points.size()-1; ++i) {
			vec2 &p = points[i];
			vec2 dir(points[i+1] - p);
			dir.fast_normalize();
			n = vec2(-dir.y, dir.x) * linkWidth;

			int idx = i*2;
			verts[idx]   = TreeVert(p - n, -n, float(i));
			verts[idx+1] = TreeVert(p + n,  n, float(i));
		}
		int idx = (points.size()-1)*2;
		verts[idx]   = TreeVert(points.back() - n, -n, float(points.size()-1));
		verts[idx+1] = TreeVert(points.back() + n,  n, float(points.size()-1));

		if(setAllData) 
			vertsVBO.setData(verts.size() * sizeof(TreeVert), GL_DYNAMIC_DRAW, &verts[0]);
		else
			vertsVBO.setSubData(startIndex * sizeof(TreeVert), (verts.size() - startIndex) * sizeof(TreeVert), &verts[startIndex]);
	}

	lastBuildBufferPointSize = points.size();
}

void Link::buildVBOIndex(VBOIndex &index) {
	std::vector<unsigned short> idxs;
	idxs.resize(30000);
	for(size_t i=0; i<idxs.size(); ++i) 
		idxs[i] = i*2;
	index.bind();
	index.setData(idxs.size()*sizeof(unsigned short), GL_STATIC_DRAW, &idxs[0]);
	index.unbind();
}

void Link::draw(Render *render) {
	if(points.empty())
		return;
	if(!render->getBounds().intersect(bounds))
		return;

	vertsVBO.bind();
	buildDrawBuffer();
	render->bindLinkVBOIndex();

	glEnableVertexAttribArray(ATTRIB_POSITION);
	glEnableVertexAttribArray(ATTRIB_NORMAL);
	glEnableVertexAttribArray(ATTRIB_LEVEL);

	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(TreeVert), ((TreeVert*)0)->v);
	glVertexAttribPointer(ATTRIB_NORMAL, 2, GL_FLOAT, false, sizeof(TreeVert), ((TreeVert*)0)->n);
	glVertexAttribPointer(ATTRIB_LEVEL, 1, GL_FLOAT, false, sizeof(TreeVert), &((TreeVert*)0)->lev);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, points.size()*2);
	glDrawElements(GL_LINE_STRIP, points.size(), GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(ATTRIB_POSITION);
	glDisableVertexAttribArray(ATTRIB_NORMAL);
	glDisableVertexAttribArray(ATTRIB_LEVEL);
	vertsVBO.unbind();
	VBO::unbindIndex();
}

void Link::drawBounds(Render *render) {
	render->drawRect(bounds, color4(0.7f, 0.7f, 0.7f, 1));
}


