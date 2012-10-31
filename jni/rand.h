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

#ifndef RAND_H
#define RAND_H

#include "math2d.h"
#include <stdlib.h>

inline float randf(float center, float div) {
	return center + div * ( (rand()/(RAND_MAX*0.5f)) - 1.0f );
}

inline vec2 randVec2(const vec2 &dir = vec2(0,1), float da = PI) {
	return mat2::get_rotate( randf(0, da) ) * dir;
}

#endif
