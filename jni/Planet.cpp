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

#include "Planet.h"
#include "Tree.h"
#include "opengl.h"
#include "rand.h"
#include "World.h"
#include "Link.h"
#include "utils.h"

PlanetObject::PlanetObject(const vec2 &ps, float r): pos(ps), radius(r) {
	seed = (rand() % 1000 + 3) / 100.73f;
	planetObjects.push_back(this);
}

BlackHole::BlackHole(const vec2 &p, float r): PlanetObject(p, r) {
	blackHoles.push_back(this);
}

BlackHole::~BlackHole() {
}

void BlackHole::draw(Render *render) {
	if(!render->getBounds().intersect(pos, radius))
		return;
	render->setShaderSeed(seed);
	render->drawCircle(pos, radius);
}

Planet::Planet(const vec2 &ps, float r, float rh): PlanetObject(ps, r), rich(rh) {
	maxLength = radius*radius*100.0f;
	index = planets.size();
	planets.push_back(this);
	for(int i=0; i<4; ++i)
		sounds[i].attach( Sound::instance().getBuffer((SoundType)i) );
}

Planet::~Planet() {
	for(int i=0; i<3; ++i)
		sounds[i].release();
	blackList.clear();
	while(!trees.empty()) 
		delete trees.back();
}

void Planet::add(Tree *t) {
	trees.push_back(t);
}

bool Planet::canSelect(Genus *r) {
	for(std::vector<GrowingPoint>::iterator gp = growingPoints.begin(); gp!=growingPoints.end(); ++gp)
		if(gp->race == r)
			return true;
	return false;
}

vec2 Planet::getGrowingPoint(Genus *r, const vec2 &dir) {
	for(std::vector<GrowingPoint>::iterator gp = growingPoints.begin(); gp!=growingPoints.end(); ++gp)
		if(gp->race == r) {
			gp->counter++;
			return gp->point;
		}
	growingPoints.push_back( GrowingPoint(r, getBestNewGrowingPoint(dir) ) );
	return growingPoints.back().point;
}

vec2 Planet::getBestNewGrowingPoint(const vec2 d) {
	vec2 dir(d);
	dir.normalize();
	vec2 result = pos + dir * radius;
	if(growingPoints.empty())
		return result;

	const float minDistance = 0.1f + radius*0.5f;

	vec2 tPoint[2] = { result, result };
	vec2 tDir[2] = { dir, dir };
	vec2 testPoint = result;
	vec2 testDir = dir;
	float maxDistance = 0;
	int sig = rand() & 1;

	for(int i=0; i<10; ++i) {

		float dist = minDistance; 
		for(std::vector<GrowingPoint>::iterator gp = growingPoints.begin(); gp!=growingPoints.end(); ++gp) {
			float d = (gp->point - testPoint).length();
			if(d < dist)
				dist = d;
		}
		if(dist > minDistance)
			return testPoint;
		if(dist > maxDistance) {
			maxDistance = dist;
			result = testPoint;
		}
		
		int idx = (i+sig) & 1;
		vec2 &ttPoint = tPoint[idx];
		vec2 &ttDir = tDir[idx];
		vec2 delta = vec2(-ttDir.y, ttDir.x) * (minDistance * 1.25f);
		if(idx)
			delta = -delta;
		ttDir = (ttPoint + delta) - pos;  
		ttDir.normalize();
		ttPoint = pos + ttDir * radius;
		testPoint = ttPoint;
		testDir = ttDir;
	}
	return result;
}

void Planet::decreaseGrowingPoint(Genus *r) {
	for(std::vector<GrowingPoint>::iterator gp = growingPoints.begin(); gp!=growingPoints.end(); ++gp)
		if(gp->race == r) {
			gp->counter--;
			if(gp->counter<=0) {
				*gp = growingPoints.back();
				growingPoints.pop_back();
			}
			return;
		}
}

void Planet::onTreeDied(Tree *t) {
	clearBlackList(t->getRace());
	decreaseGrowingPoint(t->getRace());
	vector_fast_remove(trees, t);
	playFX(SND_KILL_TREE);
}

void Planet::onUnlinkTarget(Link *l) {
	decreaseGrowingPoint(l->getParent()->getRace());
}

bool Planet::deform(vec2 &p) {
	float rd = radius * 0.95f; 
	vec2 r = p - pos;
	float d = r.length2();
	if(d < rd * rd) 
		return false;
	p = pos + r * ( rd / fast_sqrtf(d) );
	return true;
}

void Planet::growUp() {
	for(std::vector<Tree*>::iterator t = trees.begin(); t != trees.end(); ++t) 
		(*t)->growUp();
}

void Planet::step() {
	if(trees.empty())
		return;

	for(;;) {
		float length = 0, sumWeakness = 0;
		for(std::vector<Tree*>::iterator t = trees.begin(); t != trees.end(); ++t) {
			float newLength = (*t)->getLength() + (*t)->getGrowing();
			if(newLength>0) {
				length += newLength;
				sumWeakness += (*t)->getWeakness();
			}
		}

		float overLength = length > maxLength ? (length - maxLength) : 0; 
		if(overLength<=EPSILON)
			break;
		for(std::vector<Tree*>::iterator t = trees.begin(); t != trees.end(); ++t) {
			float newLength = (*t)->getLength() + (*t)->getGrowing();
			if(newLength>0) 
				(*t)->addGrowing( -overLength * (*t)->getWeakness()/sumWeakness);
		}
	}

	for(std::vector<Tree*>::iterator t = trees.begin(); t != trees.end(); ) {
//		float g = (*t)->getGrowing() - overLength * (*t)->getWeakness()/sumWeakness;
		(*t)->step((*t)->getGrowing());
		if((*t)->getLength() < EPSILON && (*t)->canDelete() ) {
			Tree *tt = *t;
			t = trees.erase(t);
			delete tt;
			continue;
 		} 
		(*t)->resetGrowing();
		++t;
	}
}

Tree* Planet::getTree(Genus* r) {
	for(std::vector<Tree*>::iterator t = trees.begin(); t != trees.end(); ++t) 
		if((*t)->getRace() == r)
			return *t;
	return 0;
}

void Planet::addLink(Planet *p, float d) {
	links.push_back(PlanetLink(p, d));
}

void Planet::draw(Render *render) {
	visible = render->getBounds().intersect(pos, radius);
	if(!visible)
		return;
	render->setShaderSeed(seed);
	render->drawCircle(pos, radius);
}

void Planet::drawTrees(Render *render) {
	for(std::vector<Tree*>::iterator t = trees.begin(); t != trees.end(); ++t)
		(*t)->draw(render, visible);
}

void Planet::drawBounds(Render *render) { 
	for(std::vector<Tree*>::iterator t = trees.begin(); t != trees.end(); ++t)
		(*t)->drawBounds(render);
}

void Planet::drawTreeLinks(Render *render) {
	for(std::vector<Tree*>::iterator t = trees.begin(); t != trees.end(); ++t)
		(*t)->drawLinks(render);
}

void Planet::checkBlackList(Genus *r, Planet *target, float distance) {
	for(std::vector<PlanetLink>::iterator l = links.begin(); l != links.end(); ++l) {
		if(l->planet == target) {
			if(l->distance > distance) 
				return;
			checkBlackListInt(r, target, distance);
		} 
	}
}

void Planet::checkBlackListInt(Genus *r, Planet *target, float distance) {
	for(std::vector<BlackPlanetLink>::iterator l = blackList.begin(); l != blackList.end(); ++l) {
		if(l->race == r && l->planet == target) { 
			if(l->distance < distance)	
				l->distance = distance;
			return;
		}
	}
	blackList.push_back(BlackPlanetLink(r, target, distance));
}

void Planet::clearBlackList(Genus *r) {
	for(std::vector<BlackPlanetLink>::iterator l = blackList.begin(); l != blackList.end();) 
		if(l->race == r) {
			BlackPlanetLink &ll = *l;
			l->planet->clearBlackList(r, this);
			l = blackList.erase(l);
		} else 
			++l;
}

void Planet::clearBlackList(Genus *r, Planet *target) {
	for(std::vector<BlackPlanetLink>::iterator l = blackList.begin(); l != blackList.end(); ++l) 
		if(l->race == r && l->planet == target) {
			*l = blackList.back();
			blackList.pop_back();
			return;
		} 
}

float Planet::blackListDistance(Genus *r, Planet *target) {
	for(size_t i = 0; i < blackList.size(); ++i) {
		BlackPlanetLink bl = blackList[i];
		if(bl.race == r && bl.planet == target)  
			return bl.distance;
	}

//	for(std::vector<PlanetLink>::iterator l = links.begin(); l != links.end(); ++l) 
//		if(l->planet == target) 
//			return l->distance;

	return -1;
}

float Planet::touchDistance(const vec2 &p, float r)	{
	float dist2 = (p-pos).length2();
	float maxDist = radius+r;
	if(dist2 <= maxDist*maxDist)
		return std::max(sqrtf(dist2) - radius, 0.0f);
	return -1.0f;
}

void Planet::playFX(SoundType t) {
	if(!visible)
		return;
	if(t>3)
		return;
	sounds[t].play();
}

void Planet::invalidate() {
	for(std::vector<Tree*>::iterator t = trees.begin(); t != trees.end(); ++t)
		(*t)->invalidate();
}

std::vector<PlanetObject*> planetObjects;
std::vector<Planet*> planets;
std::vector<BlackHole*> blackHoles;

