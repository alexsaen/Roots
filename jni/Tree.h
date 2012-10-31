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

#ifndef TREE_H
#define TREE_H

#include "HalfTree.h"
#include "Genus.h"
#include <vector>

class Planet;
class Link;
class Render;

class Tree {
friend class Mind;
	float	lengthBallance, length, treeLength, accumulator, lastStep;
	HalfTree	coma, root;
	Genus	*genus;
	Planet	*planet;
	std::vector<Link*>	links;
	std::vector<Link*>	seeders;
	int		seed;

	void	calcVars();
	void	stepUp(float v);
	void	stepDown(float v);
	void	stepPriv(float v);
	bool	isMonopoly();
	void	seeding(float v);
	void	growTo(float len);

			Tree(Genus *r, Planet *pl, const vec2 &p, const vec2 &d, float lenBallance,
				float l_up,   float lengthFactor_up,  float lengthFactorDiv_up,    float angleFactor_up,   float angleFactorDiv_up,
				float l_down, float lengthFactor_down, float lengthFactorDiv_down, float angleFactor_down, float angleFactorDiv_down, 
				Deformer *def=0);
public:
	virtual	~Tree();

	static Tree* build(Genus *r, Planet *pl, float size, const vec2& dir);

	int		getCount();
	float	getLength()			{	return length;			}
	float	getTreeLength()		{	return treeLength;		}
	vec2	getPos()			{	return coma.getPos();	}
	vec2	getDir()			{	return coma.getDir();	}
	Genus*	getRace()			{	return genus;			}
	Planet*	getPlanet()			{	return planet;			}
	float	getWeakness()		{	return genus->weakness;	}
	void	step(float v);

	bool	isLinked(Planet *target, bool checkState = false);
	bool	link(Planet* target);
	bool	unlink(Planet* target);
	void	draw(Render *render, bool drawRoot);
	void	drawLinks(Render *render);
	void	drawBounds(Render *render);

	void	addSeeder(Link *l)	{	seeders.push_back(l);	}	
	void	removeSeeder(Link *l);	
	bool	canDelete()			{	return seeders.empty();	}	

	void	growUp();
	float	getGrowing()		{	return accumulator;		}
	void	addGrowing(float v)	{	accumulator += v;		}
	void	resetGrowing()		{	accumulator = 0;		}
	void	invalidate();
};




#endif
