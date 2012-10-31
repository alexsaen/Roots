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

#ifndef BUTTON_H
#define BUTTON_H

#include "Chapter.h"
#include "CircleText.h"
#include "color.h"
#include <string>
#include <vector>

class ClickEvent {
public:
	virtual	void	onClick(Control *c)=0;
};

class Button: public Control {
	CircleText	ctext;
	ClickEvent	*event;
	color4	colors[2];
	virtual	float	distance(const vec2&p);
public:
					Button(const vec2& pos, float r, const char* text, const color4& c1, const color4 &c2, ClickEvent *e=0);
	virtual	void	onClick();
	virtual	void	draw(Render& render);
};

#endif
