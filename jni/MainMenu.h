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

#ifndef MAINMENU_H
#define MAINMENU_H

#include "Chapter.h"
#include "math2d.h"
#include "color.h"
#include "Button.h"

class MainMenu: public Chapter, ClickEvent {
	int			touchID, selectID, openLevels;
	Control		*exitButton, *aboutButton;
	const color4&	getItemColor(int id);
	int		findButton(const vec2 &pos);
	void	pressButton(int bid);

	virtual	void	onClick(Control *c);
public:
					MainMenu();
	virtual			~MainMenu()							{}
	virtual	void	touchBegan(int id, int x, int y);
	virtual	void	touchMove(int id, int x, int y);
	virtual	void	touchEnded(int id);
	virtual	void	keyDown(int kid);
	virtual void	onShow();
	virtual void	draw();
	virtual void	update();
};


#endif
