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

#ifndef CIRCLETEXT_H
#define CIRCLETEXT_H

#include "math2d.h"
#include <string>
#include <vector>

class Render;
struct color4;

class CircleText {
	vec2	pos;
	float	radius, fontSize;
	std::vector<std::string>	text;
public:
			CircleText(const vec2& pos, float r, const char* text);
	void	draw(Render& render, const mat4& transform, const color4 &col);
	float	getRadius()		{	return radius;	}
	vec2	getPos()		{	return pos;		}
};

#endif
