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

#include "CircleText.h" 
#include "Render.h"
#include "color.h"
#include "utf8/unchecked.h"

CircleText::CircleText(const vec2& p, float r, const char* t): pos(p), radius(r), fontSize(0) {
	const char *start = t;
	for(;;) {
		unsigned int cp=utf8::unchecked::next(t);
		for(; cp && cp!='\n'; cp=utf8::unchecked::next(t));
		text.push_back(std::string(start, t-start-1));
		if(!cp)
			break;
		start = t;
	}
}

void CircleText::draw(Render& render, const mat4& transform, const color4 &col) {
	Font &font = render.getFont();
	if(fontSize==0) {
		float maxWidth = 0;
		int idx = 0;
		for(size_t i = 0; i<text.size(); ++i) {
			float w = font.width(text[i].c_str());
			if(w > maxWidth) {
				maxWidth = w;
				idx = i;
			}
		}
		fontSize = radius*1.7f / maxWidth;
		if(text.size() > 1) {
			for(int i=0; i<5; ++i) {
				float dy = (idx - text.size()*0.5f + 0.5f) * fontSize;
				fontSize = 0.5f * (fontSize + sqrt(radius*radius - dy*dy) * 1.7f / maxWidth);
			}
		}
	}

	render.beginFont(transform);
//	float w = font.width(text.c_str());
//	float fs = radius*1.7f / w;
	render.setColor(col);

	float y = pos.y + (text.size()*0.5f - 0.8f) * fontSize;
	for(size_t i=0; i<text.size(); ++i) {
		float w = font.width(text[i].c_str());
		font.draw(pos.x - w*fontSize*0.5f, y, fontSize, text[i].c_str());
		y -= fontSize;
	}

}
