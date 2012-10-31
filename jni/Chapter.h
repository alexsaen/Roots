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

#ifndef CHAPTER_H
#define CHAPTER_H

#include <vector>
#include "math2d.h"

#ifdef WIN32
	#define USE_MOUSE 1
	#define BACK_KEY_ID 0
#elif defined(ANDROID)
	#define USE_MOUSE 0
	#define BACK_KEY_ID 4
#endif

class Chapter;
class Render;

class Main {
	int		windowWidth, windowHeight;
	int		mouseX, mouseY;
	bool	suspended;

	float	cursorSize, density;
	std::vector<Chapter*>	chapters;
	Chapter	*curChapter, *toChapter;
	bool	finish;
	float	chapterFade;
	void	add(Chapter *c);
	void	setCurChapter(Chapter *c);
public:
			Main(const char *resourceFile);
			~Main();
	void	getScreenSize(int &windowWidth, int &windowHeight);
	float	getCursorSize()						{	return cursorSize;		}
	Chapter*	getChapter(int id);
	bool	setCurrent(int id, bool fade=true);
	void	setCurrent(Chapter *c, bool fade=true);
	int		getCurrent();
	void	reshape(int w, int h, float density);
	void	release();
	void	suspend();
	void	resume();
	bool	isSuspended()			{	return suspended;	}
	void	touchBegan(int id, int x, int y);
	void	touchMove(int id, int x, int y);
	void	touchEnded(int id);
	void	keyDown(int kid);
	void	draw();
	void	exit();			
	bool	finished();
};

class Chapter;

class Control {
friend class Chapter;
protected:
	Chapter *parent;
	bool	hover;
public:
					Control(): parent(0), hover(false)	{}
	virtual			~Control()				{}
	virtual	float	distance(const vec2& p)	{	return F_INFINITY;	}
	virtual	void	draw(Render &dender)	{}
	virtual	void	onEnter()				{	hover = true;	}
	virtual	void	onExit()				{	hover = false;	}
	virtual	void	onClick()				{}
};

class Chapter {
friend class Main;
protected:
	int		id;
	Main	*main;
	mat4	viewMat;
	int		width, height;
	float	aspect;
	Control*	curControl;
	std::vector<Control*> controls;

	void		add(Control *c);
	vec2		touchToLocal(int x, int y);
	void		processTouch(int x, int y);
	Control*	findControl(const vec2 &pos);
public:
					Chapter(int aid);
	virtual			~Chapter();
	virtual	void	reshape(int w, int h);
	virtual	void	touchBegan(int id, int x, int y);
	virtual	void	touchMove(int id, int x, int y);
	virtual	void	touchEnded(int id);
	virtual	void	keyDown(int kid)					{}
	virtual void	onShow()							{}
	virtual void	onHide();
	virtual void	update()							{}

	const	mat4&	getTransform()						{	return viewMat;	}	
	virtual void	draw();
};

enum ChapterID {
	CID_MAINMENU,
	CID_GAME,
	CID_PAUSE,
	CID_LEVEL_COMPLETED,
	CID_LEVEL_FAILED,
	CID_ABOUT,
	CID_GAME_OVER,
	CID_FINISH
};


#endif
