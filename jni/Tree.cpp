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

#include "Tree.h"
#include "Genus.h"
#include "Planet.h"
#include "Link.h"
#include "opengl.h"
#include "rand.h"
#include "Render.h"
#include "World.h"
#include <algorithm>

float normalGrowingStep = 0.005f;
float normalLinkCuttingStep = 0.05f;
float normalLinkGrowingStep = 0.015f;

Tree::Tree(Genus *r, Planet *pl, const vec2 &p, const vec2 &d, float lenBallance,
				float l_up,   float lengthFactor_up,  float lengthFactorDiv_up,    float angleFactor_up,   float angleFactorDiv_up,
				float l_down, float lengthFactor_down, float lengthFactorDiv_down, float angleFactor_down, float angleFactorDiv_down, 
				Deformer *def): 
	genus(r), planet(pl), lengthBallance(lenBallance), accumulator(0), lastStep(0),
	coma(p, d, l_up, lengthFactor_up, lengthFactorDiv_up, angleFactor_up, angleFactorDiv_up, 0),
	root(p, -d, l_down, lengthFactor_down, lengthFactorDiv_down, angleFactor_down, angleFactorDiv_down, def)
{
	seed = rand() % 1000 + 1;
	growUp();
	calcVars();
}

Tree::~Tree() {
	planet->onTreeDied(this);
	genus->remove(this);
	while(!links.empty()) {
		delete links.back();
		links.pop_back();
	}
}

Tree* Tree::build(Genus *r, Planet *p, float size, const vec2& a_dir) {
	vec2 dir = a_dir;
	dir.normalize();
	vec2 pos = p->getGrowingPoint(r, dir);
	dir = pos - p->pos;
	dir.normalize();
	Tree *t = new Tree(r, p, pos, dir, r->lengthBallance,
			r->length_up, r->lengthFactor_up,  r->lengthFactorDiv_up, r->angleFactor_up, r->angleFactorDiv_up,
			r->length_down, r->lengthFactor_down, r->lengthFactorDiv_down, r->angleFactor_down, r->angleFactorDiv_down, p);
	p->add(t);
	r->add(t);
	t->growTo(size);
	return t;
}

void Tree::stepUp(float v) {
	float b=coma.getLength()/root.getLength();
	if(b<lengthBallance)
		coma.stepUp(v);
	else
		root.stepUp(v);
}

void Tree::stepDown(float v) {
	float b=coma.getLength()/root.getLength();
	if(b>lengthBallance)
		coma.stepDown(v);
	else
		root.stepDown(v);
}

void Tree::step(float v) {
	lastStep = v;
	stepPriv(v);
	calcVars();
}

void Tree::stepPriv(float v) {
	for(std::vector<Link*>::iterator l = links.begin(); l!=links.end();) {
		switch((*l)->state) {
			case Link::LS_NORMAL:
				(*l)->age++;
				break;
			case Link::LS_DIED:
				l = links.erase(l);
				continue;
			case Link::LS_CUTTING:
				{
					float linkCuttingStep = normalLinkCuttingStep * genus->linkFactor;
					if((*l)->length > linkCuttingStep) {
						(*l)->stepDown(linkCuttingStep);
						v += linkCuttingStep;
					} else {
						v += (*l)->length;
						delete (*l);
						l = links.erase(l);
						continue;
					}
				}
				break;
		}
		++l;
	}

	int linkGrowingCount = 0, linkNormalCount = 0;
	for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l) {
		switch((*l)->state) {
			case Link::LS_GROWING:
				linkGrowingCount++;
				break;
			case Link::LS_NORMAL:
				linkNormalCount++;
				break;
		}
	}

	float linkGrowingStep = normalLinkGrowingStep * genus->linkFactor;
	if(linkGrowingCount > 0) {
		float needForGroving = linkGrowingCount * linkGrowingStep;
		if(needForGroving > v + coma.getLength() + root.getLength()) {
			for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l)
				if((*l)->state == Link::LS_GROWING)
					(*l)->cut();
		} else {
			for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l) 
				if((*l)->state == Link::LS_GROWING) {
					(*l)->stepUp(linkGrowingStep);
					v -= linkGrowingStep;
				}
		}
	} 

	float cl = coma.getLength();
	if(cl<EPSILON) {
		int cutCount = 0;
		if(linkGrowingCount>0)
			for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l) {
				if((*l)->state == Link::LS_GROWING) {
					(*l)->cut();
					cutCount++;
				}

			}
		else if(linkNormalCount>0)
			for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l) {
				if((*l)->state == Link::LS_NORMAL) {
					(*l)->cut();
					cutCount++;
				}
			}

		if(cutCount>0)
			planet->playFX(SND_WEAK_TREE);
	}

	if(absf(v) < EPSILON)
		return;

	if(v>0) {
		if(cl<EPSILON) {
			v *= 0.5f;
			root.stepUp(v);
			coma.stepUp(v);
			return;
		}
		stepUp(v);
	} else {
		stepDown(-v);
	}
}

bool Tree::link(Planet* target) {
	if(target == planet)
		return false;
	if(isLinked(target))
		return false;
	links.push_back( new Link(this, target) );
	planet->playFX(SND_NEW_ROOTLET);
	return true;
}

bool Tree::isLinked(Planet *target, bool checkState) {
	for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l)
		if((*l)->target == target) 
			return checkState ? (*l)->state == Link::LS_NORMAL: true;
	return false;
}

bool Tree::unlink(Planet* target) {
	for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l)
		if((*l)->target == target) {
			(*l)->cut();
			planet->playFX(SND_OUT_ROOTLET);
			return true;
		}
	return false;
}

int	Tree::getCount() {
	return coma.getCount()+root.getCount();
}

void Tree::draw(Render *render, bool drawRoot) {
	render->setTreeShaderSeed(seed);
	render->setColor(genus->color);
	coma.draw(render);
	if(drawRoot) {
		render->setDeform(false);
		root.draw(render);
		render->setDeform(true);
	}
}

void Tree::drawLinks(Render *render) {
	render->setColor(genus->color);
	for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l) 
		(*l)->draw(render);
}

void Tree::calcVars() {
	length = treeLength = coma.getLength()+root.getLength();
	for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l) 
		length += (*l)->length;
}

void Tree::growTo(float len) {
	while(length<len) 
		step( len-length+0.01f );
}

void Tree::growUp() {
	float growing = normalGrowingStep * ( 1.0f + log((float)coma.getLength() + 1.0f) ) * genus->growingFactor * planet->rich;
	int cnt = 0;
	for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l) 
		if((*l)->state == Link::LS_NORMAL)
			cnt++;
	if(cnt > 0) {
		if(isMonopoly()) {
			seeding(growing / cnt);
		} else {
			growing /= cnt + 1;
			seeding(growing);
			accumulator += growing;
		}
	}  else {
		accumulator += growing;
	}
}

void Tree::seeding(float v) {
	for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l) 
		if((*l)->state == Link::LS_NORMAL)
			(*l)->leech->accumulator += v;
} 

void Tree::removeSeeder(Link *l) {
	std::vector<Link*>::iterator it = std::find(seeders.begin(), seeders.end(), l);
	if(it != seeders.end()) {
		*it = seeders.back();
		seeders.pop_back();
	}
}

bool Tree::isMonopoly() {
	if(planet->trees.size()==1) 
		if(getLength() >= planet->maxLength)
			return true;
	return false;
}

void Tree::drawBounds(Render *render) { 
	coma.drawBounds(render); 
	root.drawBounds(render); 
	for(std::vector<Link*>::iterator l = links.begin(); l!=links.end(); ++l) 
		(*l)->drawBounds(render);
}

void Tree::invalidate()	{	
	root.invalidate(); 
	coma.invalidate();	
	for(std::vector<Link*>::iterator l = links.begin(); l != links.end(); ++l)
		(*l)->invalidate();
}
