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

#ifndef WORLD_H
#define WORLD_H

#include "Chapter.h"
#include "math2d.h"
#include "color.h"
#include "AI.h"
#include "Render.h"
#include "Sound.h"
#include <vector>

class Planet;
class Genus;
class Tutorial;

class World: public Chapter {
friend class Link;
friend class Mind;
	vec2	boundsMin, boundsMax;
	vec2	pos;
	float	scale;

	enum TouchEvent {
		TE_NONE,
		TE_PAN,
		TE_ZOOM,
		TE_TRY,
		TE_CAN_LINK,
		TE_CAN_UNLINK
	};

	SoundSource	sndStartLevel, sndLevelComplete, sndGameOver;
	Genus*		currentRace;
	TouchEvent	touchEvent;
	int			touchStartX[2], touchStartY[2];
	int			touchCurrentX[2], touchCurrentY[2];
	vec2		panStart;
	float 		scaleStart;
	bool		touched[2];
	Planet*		sourcePlanet;
	AI			ai;
	Render&		render;
	std::string	levelTitle;

	void	calcPlanetGraph();
	void	drawAction();

	enum	State {
		ST_GAMEPLAY,
		ST_WIN,
		ST_DEFEAT,
		ST_PAUSE
	};

	State	state;
	color4	titleColor;
	int		currentLevel;
	int		selectedLevel;
	Tutorial	*tutorial;

			bool	checkFinishGame();

			bool	loadLevel(int idx);
	virtual void	onShow();

public:
					World();
	virtual			~World();

	virtual	void	reshape(int w, int h);
	virtual	void	touchBegan(int id, int x, int y);
	virtual	void	touchMove(int id, int x, int y);
	virtual	void	touchEnded(int id);
	virtual	void	keyDown(int kid);
	virtual	void	update();
	virtual	void	draw();
			void	drawFlashText();

	vec2	pointToPos(int x, int y);
	void	setCurrentRace(int idx);

	void	addPlanet(Planet *p);

	void	move(const vec2 &p);
	vec2	getPos()				{	return pos;			}
	float	getScale()				{	return scale;		}
	void	setScale(float s);
	void	scaleUp(float s)		{	setScale(scale * (1 + s));	}

	Planet*	getPlanet(const vec2 &p);
	bool	attack(Genus *r, Planet *from, Planet *to);

	void	clear();
	bool	loadLevel(const char *filename);

	Genus*	getCurrentRace()		{	return currentRace;	}
	void	surrender();

	void	startLevel(int idx);
	void	nextLevel();

	const char *getLevelTitle()		{	return levelTitle.c_str();	}
	const color4& getTitleColor()	{	return titleColor;	}

	void	pause();
	void	abort();
};


#endif
