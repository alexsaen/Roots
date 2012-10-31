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

#include "Button.h"
#include "Render.h"
#include "utf8/unchecked.h"

Button::Button(const vec2& p, float r, const char* t, const color4& c1, const color4 &c2, ClickEvent *e): ctext(p, r, t), event(e) {
	colors[0] = c1;
	colors[1] = c2;
}

void Button::onClick() {
	if(event)
		event->onClick(this);
}

void Button::draw(Render &render) {
	const color4& col = colors[hover];

	render.setStdShader(parent->getTransform());
	render.setColor(col);

	static std::vector<vec2> verts(40);
	for(size_t i = 0; i < verts.size(); ++i) {
		float a = 2.0f * PI * i / (verts.size()-1);
		vec2 delta(cosf(a), sinf(a));
		verts[i] = ctext.getPos() + delta * ctext.getRadius();
	}
	glLineWidth(2);
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(vec2), &verts[0]);
	glDrawArrays(GL_LINE_LOOP, 0, verts.size());
	glDisableVertexAttribArray(ATTRIB_POSITION);
	glLineWidth(1);

	ctext.draw(render, parent->getTransform(), col);
}

float Button::distance(const vec2& p) {
	return (p - ctext.getPos()).length() - ctext.getRadius();
}

