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

#ifndef CHAPTERS_H
#define CHAPTERS_H

#include "Chapter.h"
#include "Button.h"
#include "color.h"

class ChapterSShot: public Chapter, public ClickEvent {
protected:
	std::string	text;
	color4	color, textColor;
	Control			*menuButton;
	float			fadeStep;
	virtual	void	onClick(Control *c);
	virtual	void	keyDown(int kid);
			void	abortGame();
	virtual void	onShow();
	virtual void	draw();
	virtual void	update();
	virtual	void	reshape(int w, int h);
public:
					ChapterSShot(int aid, const color4& c, const char *txt, const color4& tc);
};

class ChapterSShot2: public ChapterSShot {
protected:
	Control			*okButton;
public:
					ChapterSShot2(int aid, const color4& c, const char *txt, const color4& tc);
};


class ChapterPause: public ChapterSShot2 {
	enum State {
		ST_FADE_IN,
		ST_FADE_OUT
	};
	State	state;

	virtual	void	onClick(Control *c);
	virtual	void	keyDown(int kid);
	virtual void	onShow();
	virtual void	update();
public:
					ChapterPause();
};

class ChapterLevelCompleted: public ChapterSShot2 {
	virtual	void	onClick(Control *c);
public:
					ChapterLevelCompleted();
};

class ChapterLevelFailed: public ChapterSShot2 {
	virtual	void	onClick(Control *c);
public:
					ChapterLevelFailed();
};

class ChapterGameOver: public ChapterSShot {
public:
	ChapterGameOver();
};

#endif
