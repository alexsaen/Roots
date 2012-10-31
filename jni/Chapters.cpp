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

#include "Chapters.h"
#include "World.h"
#include "Render.h"

ChapterSShot::ChapterSShot(int aid, const color4& c, const char *txt, const color4& tc): Chapter(aid), text(txt), color(c), textColor(tc), menuButton(0), fadeStep(0.02f) {
}

void ChapterSShot::reshape(int w, int h) {
	Chapter::reshape(w, h);
	if(main->getCurrent() == id) {
		World * world = (World*)main->getChapter(CID_GAME);
		if(world)
			world->pause();
		main->setCurrent(CID_GAME, false);
	}
}

void ChapterSShot::abortGame() {
	World * world = (World*)main->getChapter(CID_GAME);
	if(world)
		world->abort();
	main->setCurrent(CID_MAINMENU);
}

void ChapterSShot::onClick(Control *c) {
	if(c == menuButton)
		abortGame();
}

void ChapterSShot::keyDown(int kid) {
	if(kid == BACK_KEY_ID)
		abortGame();
}

void ChapterSShot::onShow() {
	color.a = 0;
}

void ChapterSShot::draw() {
	Render &render = Render::instance();
	render.drawChapteSShotBegin(text.c_str(), color4(textColor.r, textColor.g, textColor.b, color.a));
	Chapter::draw();
	render.drawChapteSShotEnd(color);
}

void ChapterSShot::update() {
	if(color.a < 1.0f) 
		color.a = std::min(color.a + fadeStep, 1.0f);
}

ChapterSShot2::ChapterSShot2(int aid, const color4& c, const char *txt, const color4& tc): ChapterSShot(aid, c, txt, tc), okButton(0) {
	add( menuButton = new Button(vec2(-0.8f, -0.7f), 0.18f, " Main \n Menu ", color4(0.3f,0.05f,0.05f,1), color4(0.75f,0.5f,0.1f,1), this) );
}


ChapterPause::ChapterPause(): ChapterSShot2(CID_PAUSE, color4(0.7f,0.7f,0.7f,0), "PAUSE", color4(1,0.25f,0.0f,1)) {
	add( okButton = new Button(vec2(0.8f, -0.63f), 0.25f, "Continue", color4(0.3f,0.05f,0.05f,1), color4(0.75f,0.5f,0.1f,1), this) );
	fadeStep = 0.04f;
}

void ChapterPause::onShow() {
	ChapterSShot::onShow();
	state = ST_FADE_IN;
}

void ChapterPause::onClick(Control *c) {
	if(c == okButton)
		state = ST_FADE_OUT;
	else 
		ChapterSShot::onClick(c);
}

void ChapterPause::keyDown(int kid) {
	if(kid == BACK_KEY_ID)
		state = ST_FADE_OUT;
}

void ChapterPause::update() {
	switch(state) {
		case ST_FADE_IN:
			ChapterSShot::update();
			break;
		case ST_FADE_OUT:
			if(color.a > 0.0f) 
				color.a = std::max(color.a - fadeStep, 0.0f);
			else
				main->setCurrent(CID_GAME, false);
			break;
	}
}

ChapterLevelCompleted::ChapterLevelCompleted(): ChapterSShot2(CID_LEVEL_COMPLETED, color4(0.5f,0.25f,0.1f,0), "LEVEL COMPLETED", color4(1,0.25f,0.0f,1)) {
	add( okButton = new Button(vec2(0.8f, -0.63f), 0.25f, "Next\nLevel", color4(0.3f,0.05f,0.05f,1), color4(0.75f,0.5f,0.1f,1), this) );
}

void ChapterLevelCompleted::onClick(Control *c) {
	if(c == okButton)
		main->setCurrent(CID_GAME);
	else 
		ChapterSShot::onClick(c);
}

ChapterLevelFailed::ChapterLevelFailed(): ChapterSShot2(CID_LEVEL_FAILED, color4(0.75f,0,0.1f,0), "LEVEL FAILED", color4(1,0.1f,0.1f,1)) {
	add( okButton = new Button(vec2(0.8f, -0.63f), 0.25f, "Retry", color4(0.3f,0.05f,0.05f,1), color4(0.75f,0.1f,0.1f,1), this) );
}

void ChapterLevelFailed::onClick(Control *c) {
	if(c == okButton)
		main->setCurrent(CID_GAME);
	else 
		ChapterSShot::onClick(c);
}

ChapterGameOver::ChapterGameOver(): ChapterSShot(CID_GAME_OVER, color4(0.25f,0.1f,0.7f,0), "GAME FINISHED!", color4(1,0.25f,0.0f,1)) {
	add( menuButton = new Button(vec2(0, -0.7f), 0.18f, " OK ", color4(0.3f,0.05f,0.05f,1), color4(0.75f,0.5f,0.1f,1), this) );
}



