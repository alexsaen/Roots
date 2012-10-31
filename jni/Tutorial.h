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

#ifndef TUTORIAL_H
#define TUTORIAL_H

#include "Sound.h"

class World;
class Planet;
struct color4;

class Tutorial {
	enum State {
		ST_START,
		ST_ZOOM,
		ST_LINK,
		ST_UNLINK,
		ST_NOTES,
		ST_FINISH,
		ST_STATE_COUNT
	};
	State	state, toState;
	World*	world;
	Planet	*source, *dest;
	int		timer;

	float	arrowLen, minScale, maxScale;
	float	fade;
	SoundSource	sndPapapam;

	void	switchState(State s);
	bool	checkLink(bool linked, State st);
	void	drawArrow(Planet *from, Planet *to, const color4& c);
public:
			Tutorial(World *w);
	void	update();
	void	onTouch();
	void	draw();
	void	drawFlashText();
	bool	finished();

};


#endif
