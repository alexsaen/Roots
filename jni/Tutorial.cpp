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

#include "Tutorial.h"
#include "Planet.h"
#include "World.h"
#include "Tree.h"
#include "FormatText.h"
#include <vector>

const int timeout = 200;

class FText: public FormatText {
public:
	FText(const char *txt): FormatText(txt, Render::instance().getSimpleFont(), 0.15f, color4(1,1,1,1), color4(1,1,0,1), -0.25f, -0.625f, Render::instance().getAspect() + 0.15f, TA_Right) {}
}; 

struct FlashText {
	std::string	txt;
	vec2	pos;
	float	width, factor;
	FlashText() {}
	FlashText(const vec2& p, const char* t): txt(t), pos(p), width(0), factor(0) {}
};

static std::vector<FlashText> flashText;

Tutorial::Tutorial(World *w): state(ST_START), toState(ST_ZOOM), world(w), source(0), dest(0), timer(timeout) {
	if(planets.size() >= 2) {
		source = planets[0];
		dest = planets[1];
	}
	arrowLen = 0;
	fade = 0;
	minScale = maxScale = 1.0f;
	flashText.clear();
	sndPapapam.attach( Sound::instance().getBuffer(SND_PAPAPAM) );
}
	
void Tutorial::switchState(State s) {
	toState = s;
	timer = timeout;
}

void Tutorial::update() {
	if(timer>0) { 
		timer--;
		if(timer>0)
			return;
		state = toState; 
		arrowLen = 0;
		fade = 0;
		if(state == ST_ZOOM)
			minScale = maxScale = world->getScale(); 
	}

	if(toState != state) 
		return;

	switch(state) {
		case ST_START:
			break;
		case ST_ZOOM: 
			minScale = std::min(minScale, world->getScale());
			maxScale = std::max(maxScale, world->getScale());
			if(minScale < maxScale) 
				switchState(ST_LINK);
			break;
		case ST_LINK:
			if(checkLink(true, ST_UNLINK)) {
				flashText.push_back( FlashText((source->getPos()+dest->getPos()) * 0.5f, "GOOD!") );
				sndPapapam.play();
			}
			break;
		case ST_UNLINK:
			if(checkLink(false, ST_NOTES)) {
				flashText.push_back( FlashText((source->getPos()+dest->getPos()) * 0.5f, "EXCELENT!") );
				sndPapapam.play();
			}
			break;
		case ST_NOTES:
		case ST_FINISH:
			break;
	}
}

void Tutorial::onTouch() {
	if(toState != state) 
		return;
	switch(state) {
		case ST_NOTES: 
			switchState(ST_FINISH);
			break;
		case ST_ZOOM: 
			switchState(ST_LINK);
			break;
		default:
			break;
	}
}

bool Tutorial::checkLink(bool linked, State st) {
	Tree *tree = source->getTree(world->getCurrentRace());
	if(tree) {
		if(tree->isLinked(dest, linked) == linked) {
			switchState(st);
			return true;
		}
	}
	return false;
}


void Tutorial::draw() {
	Render &render = Render::instance();

	if(timer == 0.0f) {
		switch(state) {
			case ST_START:
			case ST_ZOOM:
				break;
			case ST_LINK:
				drawArrow(source, dest, color4(0.5f, 1.0f, 0.5f, 0.5f));
				break;
			case ST_UNLINK:
				drawArrow(dest, source, color4(1.0f, 0.5f, 0.5f, 0.5f));
				break;
			case ST_NOTES:
			case ST_FINISH:
				break;
		}
	}

	int idx = state - 1;
	float alpha = 1.0f; 
	if(timer > 0) {
		const float fadeTime = timeout*0.25f; 		
		if(timer > timeout/2) {
			alpha = std::max(0.0f, (timer - timeout + fadeTime) / fadeTime);
		}  else {
			idx = toState - 1;
			alpha = std::max(0.0f, 1.0f - timer / fadeTime);
		}
	}

	if(idx >= 0) {
		static FText txts[ST_STATE_COUNT-1] = { 
			FText("<em>[Pinch]</em> to zoom in and out.<br><em>[Tap]</em> to continue."),
			FText("<em>[Drag]</em> from one planet to another to plant a new tree."),
			FText("<em>[Drag]</em> backward to cut the link."),
			FText("Notice the tree grows faster when it is linked from others.<br><em>[Tap]</em> to continue."),
			FText("All alien trees have to be eliminated to complete the level."),
		};

		mat4 transform = world->getTransform();
		Render::instance().beginFont( transform );
		txts[idx].draw(render, alpha);
	}
}


void Tutorial::drawFlashText() {
	if(!flashText.empty()) { 
		const float maxFlashFactor = 3.0f;
		const color4 color(1,0.25f,0.0f,1);

		mat4 transform = world->getTransform();
		Render &render = Render::instance();
		
		render.beginFont( transform );

		for(size_t i=0; i < flashText.size(); ) {
			FlashText &f = flashText[i];
			f.factor += 0.05f;
			if(f.factor > maxFlashFactor) {
				f = flashText.back();
				flashText.pop_back();
				continue;
			}
			float fontSize = f.factor * 0.1f; 
			float alpha = f.factor <= 1.0f ? f.factor : 1.0f - (f.factor - 1.0f)/(maxFlashFactor - 1.0f);
			Font& font = render.getFont();
			if(f.width <= 0.0f) 
				f.width = font.width(f.txt.c_str());
			render.setColor( color4(color[0], color[1], color[2], color[3]*alpha) );
			font.draw(f.pos.x - f.width*fontSize*0.5f, f.pos.y - fontSize*0.3f, fontSize, f.txt.c_str());
			++i;
		}
	}
}

void Tutorial::drawArrow(Planet *from, Planet *to, const color4& c) {
	Render &render = Render::instance();
	if(arrowLen < 1.0f) {
		arrowLen = std::min(arrowLen + 0.01f, 1.0f);
		if(fade < 1.0f)
			fade = std::min(fade + 0.02f, 1.0f);
	} else {
		if(fade > 0.0f)
			fade = std::max(fade - 0.02f, 0.0f);
		else
			arrowLen = 0;
	}
	vec2 p1 = from->getPos();
	vec2 dir = to->getPos() - p1;
	float r1 = from->getRadius();
	float d = dir.normalize() - r1 - to->getRadius();
	render.drawArrow(p1 + dir * r1, p1 + dir * (r1 + d * arrowLen), 0.1f / world->getScale(), color4(c[0], c[1], c[2], c[3]*fade));
}

bool Tutorial::finished() {
	return state == ST_FINISH;
}

