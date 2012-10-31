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

#include "Genus.h"
#include "Tree.h"
#include "Planet.h"
#include <algorithm>

Genus::Genus(const color4 &c, 
				float gFactor, float a_weakness, float lFactor, float lenBallance,
				float l_up, float lFactor_up,  float lFactorDiv_up,    float aFactor_up,   float aFactorDiv_up,
				float l_down, float lFactor_down, float lFactorDiv_down, float aFactor_down, float aFactorDiv_down): 
	color(c), growingFactor(gFactor), weakness(a_weakness), linkFactor(lFactor), lengthBallance(lenBallance), length_up(l_up), length_down(l_down),
	lengthFactor_up(lFactor_up),  lengthFactorDiv_up(lFactorDiv_up), angleFactor_up(aFactor_up), angleFactorDiv_up(aFactorDiv_up),
	lengthFactor_down(lFactor_down), lengthFactorDiv_down(lFactorDiv_down), angleFactor_down(aFactor_down), angleFactorDiv_down(aFactorDiv_down)
{
	index = genuses.size();
	genuses.push_back(this);
}

void Genus::clear() {
	while(!trees.empty())
		delete trees.back();
}

void Genus::add(Tree* t) {
	trees.push_back(t);
}

void Genus::remove(Tree *t) {
	std::vector<Tree*>::iterator it = std::find(trees.begin(), trees.end(), t);
	if(it != trees.end()) {
		*it = trees.back();
		trees.pop_back();
	}
}

std::vector<Genus*> genuses;
