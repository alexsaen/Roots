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

#include "Chapter.h"
#include "World.h"
#include "MainMenu.h"
#include "Chapters.h"
#include "ChapterAbout.h"
#include "Settings.h"

static const float fadeStep = 0.05f;
static const float defaultCursorSize = 0.25f * 160;		//  1/4 inch, 160dpi - default logical density

Main::Main(const char *resourceFile): windowWidth(0), windowHeight(0), mouseX(0), mouseY(0), suspended(false), curChapter(0), toChapter(0), density(1.0f), chapterFade(0), finish(false) {
	cursorSize = 0;		// for PC mouse

	ResourceManager::init(resourceFile);
	Settings::instance();
	Sound::instance();
    MusicPlayer &mp = MusicPlayer::instance();
	mp.addToPlayList("assets/music/theme1.ogg");
	mp.addToPlayList("assets/music/theme2.ogg");
	mp.changeMusic();

	add(new World());
	add(new MainMenu());
	add(new ChapterPause());
	add(new ChapterLevelCompleted());
	add(new ChapterLevelFailed());
	add(new ChapterAbout());
	add(new ChapterGameOver());
	setCurrent(CID_MAINMENU, true);
}

Main::~Main() {
	for(std::vector<Chapter*>::iterator c=chapters.begin(); c!=chapters.end(); ++c)
		delete (*c);
	Render::destroy();
	Sound::destroy();
	MusicPlayer::destroy();
	ResourceManager::destroy();
}

void Main::add(Chapter *c) {
	chapters.push_back(c);
	c->main = this;
}

int	Main::getCurrent()	{	
	return curChapter ? curChapter->id : -1;	
}

Chapter* Main::getChapter(int id) {
	for(std::vector<Chapter*>::iterator c=chapters.begin(); c!=chapters.end(); ++c)
		if((*c)->id == id) 
			return *c;
	return 0;
}

bool Main::setCurrent(int id, bool fade) {
	Chapter *c = getChapter(id);
	if(!c)
		return false;
	setCurrent(c, fade);
	return true;
}

void Main::setCurrent(Chapter *c, bool fade) {
//	if(c == curChapter)
//		return;

	if(!fade) {
		setCurChapter(c);
		toChapter = 0;
	} else {
		if(!curChapter) {
			setCurChapter(c);
			chapterFade = 1.0f;
		} else {
			toChapter = c;
			chapterFade = std::max(0.0f, chapterFade);
		}
	}
}

void Main::setCurChapter(Chapter *c) {
	if(curChapter)
		curChapter->onHide();
	curChapter = c;
	if(curChapter) {
#if USE_MOUSE
		curChapter->touchMove(0, mouseX, mouseY);
#endif
		curChapter->onShow();
	}
}

void Main::reshape(int w, int h, float density) {	// when screen density == 160dpi logical density =1.0
	windowWidth = w;
	windowHeight = h;
	if(density > 0) {
		float diag = sqrtf((float)w*w + h*h);
		cursorSize = defaultCursorSize * density / diag * 3.4f;
	} else
		cursorSize = 0.2f;
	for(std::vector<Chapter*>::iterator c=chapters.begin(); c!=chapters.end(); ++c)
		(*c)->reshape(w, h);
}

void Main::touchBegan(int id, int x, int y) {
	if(suspended)
		return;
	mouseX = x; mouseY = y;
	if(curChapter)
		curChapter->touchBegan(id, x, y);
}

void Main::touchMove(int id, int x, int y) {
	if(suspended)
		return;
	mouseX = x; mouseY = y;
	if(curChapter)
		curChapter->touchMove(id, x, y);
}
	
void Main::touchEnded(int id) {
	if(suspended)
		return;
	if(curChapter)
		curChapter->touchEnded(id);
}

void Main::keyDown(int kid) {
	if(suspended)
		return;
	if(curChapter)
		curChapter->keyDown(kid);
}

void Main::draw() {
	if(suspended)
		return;

	if(toChapter) {
		chapterFade += fadeStep;
		if(chapterFade >= 1.0f) {
			chapterFade = 1.0f;
			setCurChapter(toChapter);
			if(!finish) 
				toChapter = 0;
		}
	} else if(chapterFade > 0) {
		chapterFade -= fadeStep;
	}	

	if(curChapter) {
		curChapter->update();
		curChapter->draw();
		if(chapterFade > 0) {
			Render &render = Render::instance();
			render.fade(chapterFade);
		}
	}
}

void Main::getScreenSize(int &width, int &height) {
	width = windowWidth;
	height = windowHeight;
}

void Main::exit()  {
	if(finish)
		return;
	finish = true;
	setCurrent(CID_MAINMENU);
}			

bool Main::finished()	{	
	return	finish && chapterFade>=1.0f;	
}

void Main::release() {
	Render::release();
}

void Main::suspend() {
	suspended = true;
	MusicPlayer::instance().suspend();
}

void Main::resume() {
	MusicPlayer::instance().resume();
	suspended = false;
}

Chapter::Chapter(int aid): id(aid), main(0), viewMat(1.0f), width(0), height(0), aspect(1), curControl(0) {
}

void Chapter::add(Control *c) {
	controls.push_back(c);
	c->parent = this;
}

Chapter::~Chapter() {
	for(std::vector<Control*>::iterator c = controls.begin(); c!=controls.end(); ++c)
		delete *c;
}

void Chapter::reshape(int w, int h) {
	width = w;
	height = h;
	aspect = float(width)/height;
	viewMat.ortho(-aspect, aspect, -1, 1); 
}

void Chapter::draw() {
	Render &render = Render::instance();
	for(std::vector<Control*>::iterator c = controls.begin(); c!=controls.end(); ++c)
		(*c)->draw(render);
}

Control* Chapter::findControl(const vec2 &pos) {
	float cursorSize = main->getCursorSize();
	Control* selectedControl = 0;
	float minDist = cursorSize;
	for(std::vector<Control*>::iterator c = controls.begin(); c!=controls.end(); ++c) {
		float dist = (*c)->distance(pos);
		if(dist < minDist) { 
			minDist = dist;
			selectedControl = *c;
		}
	}
	return selectedControl;
}

void Chapter::processTouch(int x, int y) {
	Control *c = findControl(touchToLocal(x, y));
	if(curControl && curControl != c)
		curControl->onExit();
	curControl = c;
	if(curControl)
		curControl->onEnter();
}

void Chapter::touchBegan(int id, int x, int y)	{
	if(id!=0)
		return;
	processTouch(x, y);
}

vec2 Chapter::touchToLocal(int x, int y) {
	return vec2( (float(x)/width * 2.0f - 1.0f) * aspect, (1.0f - float(y)/height) * 2.0f - 1.0f);
}

void Chapter::touchMove(int id, int x, int y) {
	if(id!=0)
		return;
	processTouch(x, y);
}

void Chapter::touchEnded(int id) {
	if(id!=0)
		return;
	if(!curControl)
		return;
	curControl->onClick();
#if !USE_MOUSE
	curControl->onExit();
#endif
	curControl = 0;
}

void Chapter::onHide() {
	if(curControl) {
		curControl->onExit();
		curControl = 0;
	}
}



