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

#include "ChapterAbout.h"
#include "Render.h"
#include "Font.h"

static const char *title = "ABOUT";

ChapterAbout::ChapterAbout(): Chapter(CID_ABOUT), 
	ctext1(vec2(-0.6f, 0.15f), 0.5f, "Design\n Programming \n\nAlexey     \n     Saenko"),
	ctext2(vec2(0.6f, -0.15f), 0.5f, "Music\nSound FX\n\nEvgeny      \n    Emelyanov") {
	add( okButton = new Button(vec2(0, -0.725f), 0.175f, " OK ", color4(0.3f,0.05f,0.05f,1), color4(0.75f,0.5f,0.1f,1), this) );
}

void ChapterAbout::onClick(Control *c) {
	main->setCurrent(CID_MAINMENU);
}

void ChapterAbout::keyDown(int kid) {
	if(kid == BACK_KEY_ID)
		main->setCurrent(CID_MAINMENU);
}

void ChapterAbout::draw() {
	Render &render = Render::instance();
	render.drawBegin();

	render.beginBooble(viewMat);
	render.drawCircle(viewMat, ctext1.getPos(), ctext1.getRadius());
	render.drawCircle(viewMat, ctext2.getPos(), ctext2.getRadius());

	const color4 textColor(0.9f,0.25f,0.1f, 1);
	ctext1.draw(render, viewMat, textColor);
	ctext2.draw(render, viewMat, textColor);

	render.beginFont(viewMat);
	Font &font = render.getFont();
	const float titleFontSize = 0.21f;
	render.setColor(color4(1,0.25f,0.0f, 1));
	float w = font.width(title);
	font.draw(-w*titleFontSize*0.5f, 1.0f - titleFontSize, titleFontSize, title);

	Chapter::draw();

	render.drawEnd();
}
