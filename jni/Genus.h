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

#ifndef GENUS_H
#define GENUS_H

#include "color.h"
#include <vector>

class Tree;
class Planet;

class Genus {
friend class Tree;
friend class Mind;
	std::vector<Tree*>	trees;
	color4	color;
	int		index;
	float	growingFactor, weakness, linkFactor;
	float	lengthBallance, length_up, length_down;
	float	lengthFactor_up, lengthFactorDiv_up, angleFactor_up, angleFactorDiv_up;
	float	lengthFactor_down, lengthFactorDiv_down, angleFactor_down, angleFactorDiv_down;
	void	add(Tree* t);
	void	remove(Tree* t);
public:
			Genus(const color4 &c, float gFactor, float weakness, float lFactor, float lenBallance, 
				float l_up, float lengthFactor_up,  float lengthFactorDiv_up,    float angleFactor_up,   float angleFactorDiv_up,
				float l_down, float lengthFactor_down, float lengthFactorDiv_down, float angleFactor_down, float angleFactorDiv_down);
	color4	getColor()		{	return color;	}
	int		getIndex()		{	return index;	}
	bool	eliminated()	{	return trees.empty();	}
	void	clear();
};

extern std::vector<Genus*>	genuses;


#endif
