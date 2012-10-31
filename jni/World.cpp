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

#include "World.h"
#include "Planet.h"
#include "Tree.h"
#include "Genus.h"
#include "opengl.h"
#include "JSONParser.h"
#include "ResourceManager.h"
#include "rand.h"
#include "Sound.h"
#include "utils.h"
#include "Tutorial.h"
#include "Settings.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <stdio.h>

const int MAX_LEVELS = 19;

const float defaultScale = 0.5f, maxScale = 2.0f, minScale = 0.1f;

class LevelParser: public JSONParser {
	enum ObjectType {
		OT_RACES,
		OT_PLANET,
		OT_BLACK_HOLE,
		OT_COLOR,
		OT_POSITION,
		OT_START_POSITION,
		OT_TREES,
		OT_DIR,
		OT_NONE
	};
	std::vector<ObjectType> otype;
	color4	color;
	float	grow, strength, linkFactor;
	vec2	position, startPosition;
	float	radius, scale, rich;

	int		race;
	float	size;
	vec2	dir;
	struct	TreeInfo {
		int		race;
		float	size;
		vec2	dir;
		TreeInfo()	{}
		TreeInfo(int r, float s, const vec2& d): race(r), size(s), dir(d)	{}
	};

	std::vector<TreeInfo>	trees;
	int idx;
	World &world;
public:
	LevelParser(World &a_world): idx(0), world(a_world), startPosition(0.0f), scale(defaultScale) {}
	~LevelParser() {
		world.setScale(scale);
		world.move(startPosition);
	}

	virtual bool objectStart(const std::string &name) {
		switch(otype.back()) {
			case OT_RACES:
				color = color4(1,1,1);
				grow = 1.0f;
				strength = 1.0f;
				linkFactor = 1.0f;
				return true;
			case OT_PLANET:
			case OT_BLACK_HOLE:
				position = vec2(0,0);
				radius = 0.5f;
				rich = 1.0f;
				trees.clear();
				return true;
			case OT_TREES:
				race = 0;
				size = 0;
				dir = randVec2();
				return true;
			default:
				idx = 0;
				return true;
		}
		return false;
	}
	virtual bool objectEnd(const std::string &name) {	
		if(otype.empty()) 
			return false;

		switch(otype.back()) {
			case OT_RACES:
				new Genus(color, grow, 1.0f / strength, linkFactor, 4.0f,
							0.2f, 0.85f, 0.3f, PI/6.0f, 0.3f,
							0.02f, 1.5f, 0.2f, PI/6.0f, 0.3f);
				return true;
			case OT_PLANET:
				{
					Planet *p = new Planet(position, radius, rich);
					world.addPlanet(p);
					if(!trees.empty()) {
						float sizeSum = 0;
						for(std::vector<TreeInfo>::iterator it = trees.begin(); it!=trees.end(); ++it) 
							sizeSum += it->size;
						if(sizeSum > 1.0) {
							float factor = 1.0f / sizeSum;
							for(std::vector<TreeInfo>::iterator it = trees.begin(); it!=trees.end(); ++it) 
								it->size *= factor;
						}
						for(std::vector<TreeInfo>::iterator it = trees.begin(); it!=trees.end(); ++it) 
							if(it->race < (int)genuses.size()) 
								Tree::build(genuses[it->race], p, it->size*p->getMaxLength(), it->dir);
					}
				}
				return true;
			case OT_BLACK_HOLE:
				{
					BlackHole *b = new BlackHole(position, radius);
				}
				return true;
			case OT_TREES:
				trees.push_back(TreeInfo(race, size, dir));
				return true;

		}
		
		return false;	
	}
	virtual bool arrayStart(const std::string &name) {
		if(name=="races") {
			otype.push_back(OT_RACES);
			return true;
		} else if(name=="planets") {
			otype.push_back(OT_PLANET);
			return true;
		} else if(name=="blackholes") {
			otype.push_back(OT_BLACK_HOLE);
			return true;
		} else if(name=="position") {
			otype.push_back(otype.empty() ? OT_START_POSITION : OT_POSITION);
			idx = 0;
			return true;
		} else if(name=="color") {
			otype.push_back(OT_COLOR);
			idx = 0;
			return true;
		} else if(name=="trees") {
			otype.push_back(OT_TREES);
			return true;
		} else if(name=="dir") {
			otype.push_back(OT_DIR);
			idx = 0;
			return true;
		}
		return false;
	}
	virtual bool arrayEnd(const std::string &name) {
		otype.pop_back();
		return true;	
	}
	virtual	bool variable(const std::string &name, Variant value) {
		float v = value.type == JST_Int ? value.intValue : value.floatValue; 

		if(otype.empty()) {
			if(name == "scale") { 
				scale = v; 
				return true;
			}
			return false;
		}
			
		switch(otype.back()) {
			case OT_RACES:
				if(name == "grow") { 
					grow = v;
					return true;
				}
				if(name == "strength") { 
					strength = v;
					return true;
				}
				if(name == "linkGrow") { 
					linkFactor = v;
					return true;
				}
				return false;
			case OT_COLOR:
				if(idx>3)
					return false;
				color[idx] = v;
				idx++;
				return true;
			case OT_DIR:
				if(idx>1)
					return false;
				dir[idx] = v;
				idx++;
				return true;
			case OT_PLANET:
			case OT_BLACK_HOLE:
				if(name == "radius") { 
					radius = v; 
					return true;
				}
				if(name == "rich") { 
					rich = v; 
					return true;
				}
				return false;
			case OT_POSITION:
				if(idx>2)
					return false;
				position[idx] = v;
				idx++;
				return true;
			case OT_START_POSITION:
				if(idx>2)
					return false;
				startPosition[idx] = v;
				idx++;
				return true;
			case OT_TREES:
				if(name == "race") { 
					race = value.intValue; 
					return true;
				} else if(name == "size") { 
					size = v; 
					return true;
				}
				return false;
		}
		return false;	
	}
};

World::World(): Chapter(CID_GAME), boundsMin(F_INFINITY, F_INFINITY), boundsMax(-F_INFINITY, -F_INFINITY), pos(0, 0), scale(defaultScale), touchEvent(TE_NONE), currentRace(0), sourcePlanet(0), 
	render(Render::instance()), state(ST_GAMEPLAY), currentLevel(-1), tutorial(0)
{
	touched[0] = touched[1] = false;

	sndStartLevel.attach( Sound::instance().getBuffer(SND_START) );
	sndLevelComplete.attach( Sound::instance().getBuffer(SND_LEVEL_COMPLETE) );
	sndGameOver.attach( Sound::instance().getBuffer(SND_GAME_OVER) );
}

World::~World() {
	ai.abort();
}

void World::reshape(int w, int h) {
	Chapter::reshape(w, h);
	render.reshape(w, h);
	for(std::vector<Planet*>::iterator p = planets.begin(); p != planets.end(); ++p)
		(*p)->invalidate();
}

void World::addPlanet(Planet *p) {
	boundsMin.x = std::min(boundsMin.x, p->getPos().x - p->getRadius());
	boundsMin.y = std::min(boundsMin.y, p->getPos().y - p->getRadius());
	boundsMax.x = std::max(boundsMax.x, p->getPos().x + p->getRadius());
	boundsMax.y = std::max(boundsMax.y, p->getPos().y + p->getRadius());
}

void World::update() {
	if(state != ST_GAMEPLAY)
		return;

	checkFinishGame();

	for(std::vector<Planet*>::iterator p = planets.begin(); p!=planets.end(); ++p) 
		(*p)->growUp();
	for(std::vector<Planet*>::iterator p = planets.begin(); p!=planets.end(); ++p) 
		(*p)->step();

	ai.update();
	if(titleColor.a>0)
		titleColor.a -= 0.005f;

	if(tutorial)
		tutorial->update();
}

vec2 World::pointToPos(int x, int y) {
	return pos + vec2((2.0f * float(x-width*0.5f) / height),
		   -2.0f * float(y) / height + 1.0f) / scale;
}

void World::draw() {
	switch(state) {
		case ST_GAMEPLAY:
			render.draw(*this);
			drawAction();
			if(tutorial)
				tutorial->draw();
//			if(currentRace)
//				render.drawCurrentRace(currentRace->getColor());
			break;
		case ST_DEFEAT:
			render.makeScreenshot(*this);
			main->setCurrent(CID_LEVEL_FAILED, false);
			selectedLevel = currentLevel;
			break;
		case ST_WIN:
			render.makeScreenshot(*this);
			if(currentLevel < MAX_LEVELS) {
				main->setCurrent(CID_LEVEL_COMPLETED, false);
				selectedLevel = currentLevel + 1;
			} else 
				main->setCurrent(CID_GAME_OVER, false);
			break;
		case ST_PAUSE:
			render.makeScreenshot(*this);
			main->setCurrent(CID_PAUSE, false);
			break;
	}
}

void World::pause() {
	state = ST_PAUSE;
}

void World::drawAction() {
	if(!sourcePlanet)
		return;

	vec2 ps = pointToPos(touchCurrentX[0], touchCurrentY[0]);

	if((sourcePlanet->getPos() - ps).fast_length() < sourcePlanet->getRadius())
		return;

	switch(touchEvent) {
		case TE_TRY:
			render.drawArrow(sourcePlanet->getPos(), ps, 0.05f / scale, color4(0.7f, 0.7f, 0.7f, 1));
			break;
		case TE_CAN_LINK:
			render.drawArrow(sourcePlanet->getPos(), ps, 0.1f / scale, color4(0.0f, 1.0f, 0.0f, 1));
			break;
		case TE_CAN_UNLINK:
			render.drawUnlink(sourcePlanet->getPos(), ps, 0.075f / scale);
			break;		
	}  

}

void World::move(const vec2 &p) {
	pos.x = std::min(std::max(p.x, boundsMin.x), boundsMax.x);
	pos.y = std::min(std::max(p.y, boundsMin.y), boundsMax.y);
}

Planet*	World::getPlanet(const vec2 &point) {
	float cs = main->getCursorSize()*defaultScale/scale;		// half of cursor size
	Planet *planet = 0;
	float minDistance = F_INFINITY;
	for(std::vector<Planet*>::iterator p = planets.begin(); p != planets.end(); ++p) {
		float d = (*p)->touchDistance(point, cs);
		if(d == 0.0f)
			return *p;
		if(d < 0.0f)
			continue;
		if(d < minDistance) {
			planet = *p;
			minDistance = d;
		}
	}
	return planet;
}

bool World::attack(Genus *r, Planet *from, Planet *to) {
	Tree *t = to->getTree(r);
	if(t) {
		if(t->unlink(from))
			return false;
	}

	t = from->getTree(r);
	if(!t)
		return false;
	return t->link(to);
}

void World::clear() {
	ai.abort();
	currentRace = 0;
	touchEvent = TE_NONE;
	while(!planetObjects.empty()) {
		delete planetObjects.back();
		planetObjects.pop_back();
	}
	blackHoles.clear();
	planets.clear();

	while(!genuses.empty()) {
		delete genuses.back();
		genuses.pop_back();
	}

	boundsMin = vec2(F_INFINITY, F_INFINITY);
	boundsMax = vec2(-F_INFINITY, -F_INFINITY);
	scale = defaultScale;

	if(tutorial) {
		delete tutorial;
		tutorial = 0;
	}
	sourcePlanet = 0;
}

bool World::loadLevel(const char *filename) {

	std::string path("assets/levels/");
	path += filename;
	int size;
	const char *data = (const char*)ResourceManager::instance()->loadFile(path.c_str(), size);
	if(!data)
		return false;

	clear();
	LevelParser lp(*this);
	if( lp.parse(data) ) {
		calcPlanetGraph();

		ai.clear();
		for(size_t i=1; i<genuses.size(); ++i)
			ai.add(new Mind(i, 3));
	}
	delete data;


	state = ST_GAMEPLAY;
	return true;
}

void World::touchBegan(int id, int x, int y) {
	if(id>1)
		return;

	touchCurrentX[id] = touchStartX[id] = x;
	touchCurrentY[id] = touchStartY[id] = y;
	touched[id] = true;

	switch(id) {
		case 0:
			sourcePlanet = getPlanet( pointToPos(x, y) );
			if(sourcePlanet && sourcePlanet->canSelect(currentRace) ) {
				touchEvent = TE_TRY;
			} else {
				touchEvent = TE_PAN;
				panStart = pos;
			}
			break;
		case 1:		//  for right mouse button PC only
			touchEvent = TE_PAN;
			panStart = pos;
			break;
	}
}

void World::touchMove(int id, int x, int y) {
	if(id>1)
		return;

 	touchCurrentX[id] = x;
	touchCurrentY[id] = y;

#if !USE_MOUSE
	if(id == 1) {
		touched[1] = true;
		touchStartX[1] = touchCurrentX[1];
		touchStartY[1] = touchCurrentY[1];

		touchEvent = TE_ZOOM;
		touchStartX[0] = touchCurrentX[0];
		touchStartY[0] = touchCurrentY[0];
		panStart = pos;
		scaleStart = scale;
		return;
	}
#endif

	switch(touchEvent) {
		case TE_ZOOM: 
			setScale(scaleStart * (vec2((float)touchCurrentX[0] - touchCurrentX[1], (float)touchCurrentY[0] - touchCurrentY[1]).length() /
				vec2((float)touchStartX[0] - touchStartX[1], (float)touchStartY[0] - touchStartY[1]).length() ) );
			move(panStart + pointToPos((touchStartX[0] + touchStartX[1]) / 2, (touchStartY[0] + touchStartY[1]) / 2)
				-	pointToPos((touchCurrentX[0] + touchCurrentX[1]) / 2, (touchCurrentY[0] + touchCurrentY[1]) / 2) );
			break;
		case TE_PAN:
			move(panStart + pointToPos(touchStartX[id], touchStartY[id]) - pointToPos(touchCurrentX[id], touchCurrentY[id]));
			break;
		case TE_TRY:
		case TE_CAN_LINK:
		case TE_CAN_UNLINK:
			if(id>0)
				break;
			{
			Planet* targetPlanet = getPlanet( pointToPos(x, y) );
			if(!targetPlanet || targetPlanet == sourcePlanet) {
				touchEvent = TE_TRY;
				break;
			}
			Tree *targetTree = targetPlanet->getTree(currentRace);
			if(targetTree) {
				if(targetTree->isLinked(sourcePlanet)) {
					touchEvent = TE_CAN_UNLINK;
					break;
				}
			}
			Tree *sourceTree = sourcePlanet->getTree(currentRace);
			if(sourceTree) {
				if(sourceTree->isLinked(targetPlanet)) {
					touchEvent = TE_TRY;
					break;
				}
				touchEvent = TE_CAN_LINK;
			}
			}
			break;
	}

}

void World::touchEnded(int id) {
	switch(state) {
		case ST_GAMEPLAY:
			switch(touchEvent) {
				case TE_CAN_LINK:
				case TE_CAN_UNLINK:
					{
						Planet *targetPlanet = getPlanet( pointToPos(touchCurrentX[0], touchCurrentY[0]) );
						if(targetPlanet) 
							attack(currentRace, sourcePlanet, targetPlanet);
					}
					touchEvent = TE_NONE;
					break;
				case TE_TRY:
				case TE_PAN:
				case TE_ZOOM:
					touchEvent = TE_NONE;
					break;
			}
			touched[0] = touched[1] = false;
			if(tutorial)
				tutorial->onTouch();
			break;
		case ST_WIN:
		case ST_DEFEAT:
			break;
	}
	
}

bool World::checkFinishGame() {
	if(tutorial && !tutorial->finished())
		return false;
	if(!currentRace)
		return false;
	if(currentRace->eliminated()) {
		state = ST_DEFEAT;
		sndGameOver.play();
		return true;
	}
	for(std::vector<Genus*>::iterator r = genuses.begin(); r != genuses.end(); ++r) {
		if(*r == currentRace)
			continue;
		if(!(*r)->eliminated())
			return false;
	}
	MusicPlayer::instance().changeMusic();
	state = ST_WIN;
	sndLevelComplete.play();
	Settings::instance().setOpenLevels(currentLevel+1);
	return true;
}

void World::setCurrentRace(int idx) {
	if(idx < (int)genuses.size())
		currentRace = genuses[idx];
}

void World::calcPlanetGraph() {
	for(std::vector<Planet*>::iterator p1 = planets.begin(); p1 != planets.end()-1; ++p1) {
		for(std::vector<Planet*>::iterator p2 = p1 + 1; p2 != planets.end(); ++p2) {
			float dist = ((*p1)->pos - (*p2)->pos).length();
			if(dist < (*p1)->maxLength)
				(*p1)->addLink(*p2, dist);
			if(dist < (*p2)->maxLength)
				(*p2)->addLink(*p1, dist);
		}
	}
}

void World::keyDown(int kid) {
#ifdef WIN32
	switch(kid) {
		case 4:						// for PC SDL weel up/down
			scaleUp(0.2f);
			break;
		case 5:
			scaleUp(-0.2f);
			break;
		case 27:
			pause();
			break;
		default:
			break;
	}
#elif defined(ANDROID)
	if(kid == BACK_KEY_ID)
		pause();
#endif
}

void World::surrender() {
	currentRace->clear();
}

bool World::loadLevel(int idx) {
	MusicPlayer::instance().changeMusic();
	if(!loadLevel( (std::string("level.") + to_string(idx)).c_str() ) ) 
		return false;
	setCurrentRace(0);
	currentLevel = idx;
	levelTitle = std::string("LEVEL ") + to_string(currentLevel+1);
	titleColor = color4(1,0.25f,0.0f,1);
	if(idx==0)
		tutorial = new Tutorial(this);
	sndStartLevel.play();
	return true;
}

void World::nextLevel() {
	startLevel(currentLevel+1);
}

void World::setScale(float s) {	
	scale = std::min(std::max(s, minScale), maxScale);
}

void World::startLevel(int idx) {
	selectedLevel = idx;
	main->setCurrent(CID_GAME);
	state = ST_GAMEPLAY;
}

void World::onShow() {
	switch(state) {
		case ST_PAUSE:
			state = ST_GAMEPLAY;
			break;
		case ST_GAMEPLAY:
		default:
			if(!loadLevel(selectedLevel))
				main->setCurrent(CID_MAINMENU);
			break;
	}
}

void World::drawFlashText()	{	
	if(tutorial) 
		tutorial->drawFlashText();		
}

void World::abort() {
	ai.abort();
}
