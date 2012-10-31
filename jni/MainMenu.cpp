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

#include "MainMenu.h"
#include "Render.h"
#include "utils.h"
#include "World.h"
#include "Sound.h"
#include "Button.h"
#include "Settings.h"
#include <vector>

const float wingFactor = 0.4f;
const float startLength = 0.44f;
const float lengthFactor = 0.95f;

const char *gameTitle = "R.O.O.T.S";

static color4 itemColor[] = { color4(0.3f,0.05f,0.05f,1), color4(1.0f,0.5f,0.0f,1), color4(1.0f,0.75f, 0.25f,1) };

struct CircleButton {
	int		id;
	vec2	pos;
	float	r;
	CircleButton()															{}
	CircleButton(int aid, const vec2 &p, float ar): id(aid), pos(p), r(ar)	{}
};

static std::vector<CircleButton> buttons; 

void drawCircleButton(Render &render, const vec2 &pos, const vec2 &d, float length, int id) {
	static std::vector<vec2> verts;
	verts.clear();
	verts.push_back( pos );
	verts.push_back( pos+d*length );
	float r = sqrtf(length)*0.3f;
	vec2 p=pos+d*(length+r);
	int vertCount = 20;
	for(int i = 0; i < vertCount; ++i) {
		float a = 2.0f * PI * i / (vertCount-1);
		vec2 delta(cosf(a), sinf(a));
		if(i>1)
			verts.push_back( verts.back() );
		verts.push_back( p+delta*r );
	}
	glLineWidth(2);
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(vec2), &verts[0]);
	glDrawArrays(GL_LINES, 0, verts.size());
	glDisableVertexAttribArray(ATTRIB_POSITION);
	glLineWidth(1);

	buttons.push_back(CircleButton(id, p, r));
}

MainMenu::MainMenu(): Chapter(CID_MAINMENU) {
	add( exitButton = new Button(vec2(1.025f, -0.8f), 0.15f, "Exit", color4(0.3f,0.05f,0.05f,1), color4(0.75f,0.1f,0.1f,1), this) );
	add( aboutButton = new Button(vec2(1.125f, -0.49f), 0.125f, "About", color4(0.3f,0.05f,0.05f,1), color4(0.75f,0.5f,0.1f,1), this) );
	openLevels = Settings::instance().getOpenLevels();
}

const color4& MainMenu::getItemColor(int id) {
	if(id > openLevels)
		return itemColor[0];
	if(id == touchID || id == selectID)
		return itemColor[2];
	return itemColor[1];
}

void MainMenu::draw() {
	Render &render = Render::instance();
	render.drawBegin();

	render.setStdShader(viewMat);

	static float a=0.0f;
	a += 0.01f;
	float deviator=sinf(a);

///////////////////////////////////////////////////////
	buttons.clear();
	static std::vector<vec2> verts;
	verts.clear();
	vec2 dir(-0.5, 1); dir.normalize();

	vec2 pos(-0.35f, -1);
	float length = startLength;
	vec2 pn;
	for(int i=0; i<21; ++i) {
		vec2 n(-dir.y, dir.x);
		
		if(i==0) {
			verts.push_back(pos);
			verts.push_back(pos+n*(length*0.13f));
		}
		vec2 p(pos + dir*length);
		verts.push_back(p);
		verts.push_back(p+n*(length*0.1f));
		
		length*= (i == 0) ? 0.65f : lengthFactor;
		dir -= n*(wingFactor+deviator*0.005f);
		dir.normalize();

		if(i>0) {
			vec2 nn(n+pn);
			nn.normalize();
			int id = i - 1;
			
			render.setColor( getItemColor(id) );
			drawCircleButton(render, pos, nn, length*0.75f, id);
		}

		pos = p;
		pn = n;
	}

///////////////////////////////////////////

	render.setColor(color4(1,0.5f,0,1));
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(vec2), &verts[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, verts.size());
	glDisableVertexAttribArray(ATTRIB_POSITION);

	render.beginFont(viewMat);
	Font &font = render.getFont();
	for(size_t i=0; i<buttons.size(); ++i) {
		CircleButton &b = buttons[i];
		render.setColor( getItemColor(b.id) );
		std::string s = to_string(b.id+1);
		float w = font.width(s.c_str());
		float fs = b.r*1.3f;
		font.draw(b.pos.x - w*fs*0.5f, b.pos.y-fs*0.3f, fs, s.c_str());
	}

	float titleFontSize = 0.25f;
	render.setColor(color4(1,0.25f,0.0f, 1));
	float w = font.width(gameTitle);
	font.draw(-w*titleFontSize*0.5f, 1.0f - titleFontSize, titleFontSize, gameTitle);

	Chapter::draw();

	render.drawEnd();
}

void MainMenu::update()	{
}

int MainMenu::findButton(const vec2 &pos) {
	float cursorSize = main->getCursorSize();
	int selectedId = -1;
	float minDist = cursorSize;
	for(int i=0; i<std::min((int)buttons.size(), openLevels+1); ++i) {
		CircleButton &b = buttons[i];
		float dist = (pos-b.pos).length() - b.r;
		if(dist < minDist) { 
			minDist = dist;
			selectedId = b.id;
		}
	}

	if(selectedId > openLevels)
		selectedId = -1;

	if(selectedId != -1 && curControl) {
		if(minDist > curControl->distance(pos) )
			selectedId = -1;
	} 

	return selectedId;
}

void MainMenu::touchBegan(int id, int x, int y)	{
	if(id!=0)
		return;
	Chapter::touchBegan(id, x, y);
	touchID = findButton(touchToLocal(x, y));
}

void MainMenu::touchMove(int id, int x, int y) {
	if(id!=0)
		return;
	Chapter::touchMove(id, x, y);

	selectID = findButton(touchToLocal(x, y));
	if(selectID != touchID)
		touchID = -1;
}

void MainMenu::touchEnded(int id) {
	if(id!=0)
		return;
	if(touchID<0) {
		Chapter::touchEnded(id);
		return;
	}
	pressButton(touchID);
	touchID = -1;
}

void MainMenu::pressButton(int bid) {
	World *world = (World*)main->getChapter(CID_GAME);
	world->startLevel(bid);
}

void MainMenu::onShow() {
	MusicPlayer::instance().changeMusic();
	touchID = selectID = -1;
	openLevels = Settings::instance().getOpenLevels();
}

void MainMenu::onClick(Control *c) {
	if(c==exitButton)
		main->exit();
	else if(c==aboutButton)
		main->setCurrent(CID_ABOUT);
		
}

void MainMenu::keyDown(int kid) {
	if(kid == BACK_KEY_ID)
		main->exit();
}

