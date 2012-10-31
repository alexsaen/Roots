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

#ifndef PLANET_H
#define PLANET_H

#include "math2d.h"
#include "HalfTree.h"
#include "Sound.h"
#include "Genus.h"
#include <vector>

class Tree;
class Link;
class Render;

class PlanetObject {
protected:
	vec2	pos;
	float	radius, seed;
			PlanetObject(const vec2 &p, float r);
public:
	virtual ~PlanetObject()	{}
	vec2	getPos()						{	return pos;					}
	float	getRadius()						{	return radius;				}
	virtual void	draw(Render *render) = 0;
};

class BlackHole: public PlanetObject {
public:
			BlackHole(const vec2 &p, float r);
	virtual ~BlackHole();
	virtual void	draw(Render *render);
};

class Planet: public PlanetObject, public Deformer  {
friend class Link;
friend class Tree;
friend class World;
friend class Mind;
	int		index;
	float	maxLength, rich;
	bool	visible;
	std::vector<Tree*> trees;
	SoundSource	sounds[4];

	struct PlanetLink {
		Planet	*planet;
		float	distance;
		PlanetLink()														{}
		PlanetLink(Planet *p, float d): planet(p), distance(d)				{}
	};

	struct BlackPlanetLink: PlanetLink {
		Genus	*race;
		BlackPlanetLink()															{}
		BlackPlanetLink(Genus *r, Planet *p, float d): PlanetLink(p, d), race(r)		{}
	};

	struct GrowingPoint {
		Genus	*race;
		vec2	point;
		int		counter;
		GrowingPoint()														{}
		GrowingPoint(Genus *r, const vec2 &p): race(r), point(p), counter(1)	{}
	};

	std::vector<PlanetLink>			links;
	std::vector<BlackPlanetLink>	blackList;
	std::vector<GrowingPoint>		growingPoints;

	virtual bool	deform(vec2 &p);
			void	add(Tree *t);
			void	addLink(Planet *p, float d);

			vec2	getBestNewGrowingPoint(const vec2 dir);
			void	decreaseGrowingPoint(Genus *r);
			void	onTreeDied(Tree *t);
			void	onUnlinkTarget(Link *l);

			void	checkBlackListInt(Genus *r, Planet *target, float distance);
public:
			Planet(const vec2 &p, float r, float rh);
	virtual	~Planet();
	vec2	getGrowingPoint(Genus *r, const vec2 &dir);
	bool 	canSelect(Genus *r);
	void	checkBlackList(Genus *r, Planet *target, float cutDistance);
	void	clearBlackList(Genus *r);
	void	clearBlackList(Genus *r, Planet *target);
	float	blackListDistance(Genus *r, Planet *target);

	bool	inside(const vec2 &p)				{	return (p-pos).length2() <= radius*radius;	}
	float	touchDistance(const vec2 &p, float r);
	void	growUp();
	void	step();
	void	drawTrees(Render *render);
	void	drawTreeLinks(Render *render);
	Tree*	getTree(Genus* r);

	void	drawBounds(Render *render);
	float	getMaxLength()					{	return maxLength;	}

	virtual void	draw(Render *render);
			void	playFX(SoundType t);
			void	invalidate();
};

extern std::vector<PlanetObject*>	planetObjects;
extern std::vector<Planet*>	planets;
extern std::vector<BlackHole*>	blackHoles;

#endif
