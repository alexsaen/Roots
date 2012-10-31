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

#include "AI.h"
#include "World.h"
#include "Genus.h"
#include "Planet.h"
#include "Tree.h"
#include "Link.h"
#include "platform.h"

static const float normalMindStep = 0.5f;

Mind::Move Mind::searchBestMove(int depthStart) {
	const unsigned int maxSearchingTime = 2000;
	unsigned int ticks = platform::getTicks();
	int depth = depthStart;

	std::vector<Mind::Move> moves;
	calcMoves(playerIdx, moves);

	int mc = moves.size();
	int bestMove = -1;
	float alpha = -F_INFINITY;

	for(unsigned i=0; i<moves.size(); ++i) {
		Move m = moves[i];
		makeMove(playerIdx, m); 

		for(size_t p=0; p < genuses.size(); ++p) {
			if(genuses[p]->trees.empty())
				continue;
			if(p != playerIdx) {
				float eval = -alphaBeta(p, depth-1, -F_INFINITY, -alpha);
				if(m.type != MT_NOTING) {
					MTreeData &dt = treeData(playerIdx, m.from);
					eval += absf(eval) * (1.0f - dt.doNothingFactor);
				}
				if(eval > alpha) {
					alpha = eval;
					bestMove = i;
				}
			}
		}

		undoMove();

		if(state == ST_ABORT)
			return Move(MT_NOTING);

		if(depth == depthStart) {
			unsigned int dt = platform::getTicks() - ticks;
			if(dt>maxSearchingTime)
				depth = 1;
		}
	}
	return bestMove == -1 ? Move(MT_NOTING) : moves[bestMove];
}

float Mind::alphaBeta(int pIdx, int depth, float alpha, float beta) {
	if(state == ST_ABORT)
		return -F_INFINITY;

	if(isLooser(pIdx)) 
		return -F_INFINITY;

	if(depth == 0) 
		return evaluate(pIdx);

	std::vector<Mind::Move> moves;
	calcMoves(pIdx, moves);

	float score = -F_INFINITY;

	for(unsigned i = 0; i < moves.size(); ++i) {
		makeMove(pIdx, moves[i]); 

		for(size_t p = 0; p < genuses.size(); ++p) {
			if(p != pIdx && !isLooser(p)) {
				float eval = -alphaBeta(p, depth-1, -beta, -alpha);
				if(eval > score) {
					score = eval;
					if(score > alpha) {
						alpha = score;		
						if(alpha >= beta) {
							undoMove();
							return alpha;
						}
					}
				}
			}
		}
		undoMove();
		if(state == ST_ABORT)
			return score;
	}

	return score;
}

inline Mind::MTree& Mind::tree(int race, int planet) {
	return trees[ stack * stackSize + planet*genuses.size() + race ];
} 

inline Mind::MTreeData& Mind::treeData(int race, int planet) {
	return treesData[ planet*genuses.size() + race ];
} 

void Mind::initPosition() {
	stack = 0; 
	trees.clear();
	trees.resize(stackSize);
	links.clear();
	linksIdx.clear();
	linksIdx.push_back(0);

	for(unsigned ip=0; ip<planets.size(); ++ip) {
		Planet *p = planets[ip];
		for(std::vector<Tree*>::iterator it = p->trees.begin(); it != p->trees.end(); ++it) { 
			MTree& t = tree((*it)->genus->index, ip);
			MTreeData& dt = treeData((*it)->genus->index, ip);
			dt.canLink = true;
			t.length = (*it)->getLength();
			t.treeLength = (*it)->getTreeLength();
			for(std::vector<Link*>::iterator l = (*it)->links.begin(); l != (*it)->links.end(); ++l) {
				switch((*l)->state) {
					case Link::LS_NORMAL:
						t.links++;
						links.push_back(MLink((*it)->genus->index, (*l)->parent->planet->index, (*l)->target->index, (*l)->length, (*l)->age>200));
						break;
					case Link::LS_GROWING:
						{
							t.links++;
							dt.canLink = false;
							links.push_back(MLink((*it)->genus->index, (*l)->parent->planet->index, (*l)->target->index, (*l)->dist, false));
							float delta = (*l)->dist > (*l)->length;
							if(delta > 0)	
								t.treeLength = std::max(0.0f, t.treeLength - delta);
						}
						break;
					default:
						break;
				}
			}
		}
	}

}

void Mind::dublicateStack() {
	stack++;
	trees.resize(stackSize*(stack+1));
	memcpy(&trees[stackSize*stack], &trees[stackSize*(stack-1)], stackSize*sizeof(MTree));

	int idx = linksIdx.back();
	int newIdx = links.size(); 
	int s = newIdx - idx;
	if(s > 0) {
		links.resize(newIdx + s);
		memcpy(&links[newIdx], &links[idx], s * sizeof(MLink) );
	}
	linksIdx.push_back(newIdx);
}

void Mind::undoMove() {
	stack--;
	trees.resize(stackSize*(stack+1));
	links.resize(linksIdx.back());
	linksIdx.pop_back();
}

void Mind::proceedMove(float step) {
	for(unsigned r=0; r<genuses.size(); ++r) {
		for(unsigned p=0; p<planets.size(); ++p) {
			MTreeData &d = treeData(r, p);
			d.growing = d.accumulator = 0;
			MTree &t = tree(r, p);
			if(t.treeLength >= 0) {
				d.growing = step * ( 1.0f + log(t.treeLength*0.8f + 1.0f) ) * genuses[r]->growingFactor * planets[p]->rich; //( 1.0f + log((float)coma.getLength() + 1.0f) ) * race->growingFactor;
				if(t.links > 0) {
					float maxLength = planets[p]->maxLength;
//					if(t.length + d.growing / t.links > maxLength) {	// monopoly case
//						d.accumulator = maxLength - t.length;
//						d.growing -= d.accumulator;
//						d.growing /= t.links;
//					} else {
						d.growing /= t.links + 1;
						d.accumulator = d.growing;
//					}
				} else {
					d.accumulator = d.growing;
					d.growing = 0;
				}
			}
		}
	}

	for(size_t i = linksIdx.back(); i<links.size(); ++i) {
		MLink &l = links[i];
		MTreeData &d1 = treeData(l.race, l.from);
		MTreeData &d2 = treeData(l.race, l.to);
		d2.accumulator += d1.growing;
	}

	for(unsigned p=0; p<planets.size(); ++p) {
		for(;;) {
			float sum = 0;
			int cnt = 0;
			for(unsigned r=0; r<genuses.size(); ++r) {
				MTree &t = tree(r, p);
				MTreeData &d = treeData(r, p);
				if(t.length>=0) {
					t.length += d.accumulator;
					d.accumulator = 0;
					sum += t.length;
					cnt++;
				}
			}
			if(cnt == 0)
				break;
			float overLength = sum - planets[p]->maxLength;
			if(overLength <= EPSILON)
				break;
			float sumWeakness = 0;
			for(unsigned r=0; r<genuses.size(); ++r) {
				MTree &t = tree(r, p);
				if(t.length>=0) 
					sumWeakness += genuses[r]->weakness;
			}
			for(unsigned r=0; r<genuses.size(); ++r) {
				MTree &t = tree(r, p);
				if(t.length>=0) 
					t.length -= overLength * genuses[r]->weakness / sumWeakness;
			}
		}
	}

	for(size_t i = stack * stackSize; i<trees.size(); ++i) {
		MTree &t = trees[i];
		t.treeLength = t.length;
	}

	for(size_t i = linksIdx.back(); i<links.size(); ) { // drop links from dead trees
		MLink &l = links[i];
		MTree &t = tree(l.race, l.from);
		if(t.length<0) {
			l = links.back();
			links.pop_back();
		} else {
			if(t.treeLength>0)
				t.treeLength = std::max(0.0f, t.treeLength - l.length);
			++i;
		}
	}

} 

float Mind::evaluate(int pidx) {
	float result = 0;

	for(unsigned r=0; r<genuses.size(); ++r) {
		for(unsigned p=0; p<planets.size(); ++p) {
			MTree &t = tree(r, p);
			if(t.length >= 0)
				result += r==pidx? t.length : -t.length;
		}
	}
	return	result;			
}

bool Mind::isLooser(int pidx) {
	for(unsigned p=0; p<planets.size(); ++p) {
		MTree &t = tree(pidx, p);
		if(t.length > 0)
			return false;
	}
	return true;
}

bool Mind::haveLink(int race, int from, int to) {
	for(size_t i = linksIdx.back(); i<links.size(); ++i) {
		MLink &l = links[i];
		if(l.race == race) {
			if(l.from == from && l.to == to)
				return true;
			if(l.to == from && l.from == to)
				return true;
		} 
	}
	return false;
}

void Mind::calcMoves(int pIdx, std::vector<Mind::Move> &moves) {
	moves.push_back(Move(MT_NOTING));

	for(size_t i = linksIdx.back(); i<links.size(); ++i) {
		MLink &l = links[i];
		if(l.race == pIdx && l.canUnlink)
			moves.push_back(Move(MT_UNLINK, i-linksIdx.back(), l.from, l.to));
	}

	for(unsigned p=0; p<planets.size(); ++p) {
		MTree &t = tree(pIdx, p);
		if(t.length>0) {
			MTreeData &dt = treeData(pIdx, p);
			if(dt.canLink) {
				Planet *planet = planets[p];
				for(size_t i=0; i<planet->links.size(); ++i) {
					Planet::PlanetLink &l = planet->links[i];
					if(l.distance < t.treeLength)
						if(!haveLink(pIdx, p, l.planet->index))	{		// TODO: slowly function
							float d = planet->blackListDistance(genuses[pIdx], l.planet);
							if(d < t.treeLength)
								moves.push_back(Move(MT_LINK, p, l.planet->index, l.distance));
						}
				}
			}
		}
	}
}

void Mind::makeMove(int pIdx, const Move &m) {
	dublicateStack();
	switch(m.type) {
		case MT_NOTING:
			break;
		case MT_UNLINK:
			{
				MLink &l = links[linksIdx.back() + m.idx];
				MTree &t = tree(l.race, l.from);
				t.treeLength += l.length;
				t.links--;
				l = links.back();
				links.pop_back();
			}
			break;
		case MT_LINK: 
			{
				links.push_back(MLink(pIdx, m.from, m.to, m.length, true));
				MTree &t = tree(pIdx, m.from);
				t.treeLength -= m.length;
				t.links++;
				MTree &t2 = tree(pIdx, m.to);
				if(t2.length<0)
					t2.length = t2.treeLength = 0;
			}
			break;
	}
	proceedMove(normalMindStep);	
}

Mind::Mind(int pidx, int d): playerIdx(pidx), depth(d), state(ST_STOP) {
	ticks = platform::getTicks();
	stackSize = genuses.size()*planets.size();
	treesData.resize(stackSize);
}

Mind::~Mind() {}

bool Mind::threadUpdate() {
	if(state == ST_SEARCHING) {
		bestMove = searchBestMove(depth);
		state = ST_READY;
		return true;
	}
	return false;
}

const float nothingFactorMin = 1.0f, nothingFactorNormal=1.0f, nothingFactorMax = 1.1f;
const float nothingFactorStep1 = 0.00001f;
const float nothingFactorStep2 = 0.0000001f;

bool Mind::update() {
	unsigned int t = platform::getTicks();
	unsigned int dtime = t - ticks;
	ticks = t;

	switch(state) {
		case ST_STOP:
			initPosition();
			state = ST_SEARCHING;
			break;
		case ST_READY:
			state = ST_STOP;
			switch(bestMove.type) {
				case MT_NOTING:
					for(size_t i=0; i<treesData.size(); ++i) {
						MTree &t = trees[i];
						if(t.length>0) {
							MTreeData &dt = treesData[i]; 
							if(dt.doNothingFactor > nothingFactorMin) {
								if(dt.doNothingFactor > nothingFactorNormal)
									dt.doNothingFactor = std::max(nothingFactorNormal, dt.doNothingFactor - nothingFactorStep1*dtime);
								else
									dt.doNothingFactor = std::max(nothingFactorMin, dt.doNothingFactor - nothingFactorStep2*dtime);
							}
						}
					}
					return true;
				case MT_UNLINK:
					{
						MTreeData &dt = treeData(playerIdx, bestMove.from); 
						dt.doNothingFactor = nothingFactorMax;
						Genus *r = genuses[playerIdx];
						Planet *from = planets[bestMove.from];
						Tree *t = from->getTree(r);
						if(!t)
							return false;
						return t->unlink(planets[bestMove.to]);
					}
					break;
				case MT_LINK:
					{
						MTreeData &dt = treeData(playerIdx, bestMove.from); 
						dt.doNothingFactor = nothingFactorMax;
						Genus *r = genuses[playerIdx];
						Planet *from = planets[bestMove.from];
						Tree *t = from->getTree(r);
						if(!t)
							return false;
						return t->link(planets[bestMove.to]);
					}
					break;
			}
			break;
		case ST_SEARCHING:
			break;
	}
	return false;
}

void Mind::abort() {
	state = ST_ABORT;
}

AI::AI(): finish(false) {
	pthread_mutex_init(&mutex, 0);
	pthread_cond_init(&cond, 0);
	pthread_create(&thread, 0, threadFunc, this);
}

AI::~AI() {
	pthread_mutex_lock(&mutex);
	finish = true;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);

	pthread_join(thread, 0);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
}

void AI::clear() {
	pthread_mutex_lock(&mutex);
	for(std::vector<Mind*>::iterator m = minds.begin(); m != minds.end(); ++m)
		delete *m;
	minds.clear();
	pthread_mutex_unlock(&mutex);
}

void AI::add(Mind *m) {
	pthread_mutex_lock(&mutex);
	minds.push_back(m);
	pthread_mutex_unlock(&mutex);
}

void AI::abort() {
	for(std::vector<Mind*>::iterator m = minds.begin(); m != minds.end(); ++m)
		(*m)->abort();
	pthread_mutex_lock(&mutex);
	pthread_mutex_unlock(&mutex);
}

void AI::update() {
	if(pthread_mutex_trylock(&mutex))
		return;							// mutex already locked

	for(std::vector<Mind*>::iterator m = minds.begin(); m != minds.end(); ++m)
		(*m)->update();

	pthread_cond_signal(&cond);	
	pthread_mutex_unlock(&mutex);
}

bool AI::threadUpdate() {
	int rc = pthread_mutex_lock(&mutex);
	if(finish) {
		rc = pthread_mutex_unlock(&mutex);
		return true;
	}
	rc = pthread_cond_wait(&cond, &mutex);
	if(rc) {
//		LOGE("Thread condwait failed, rc=%d", rc);
		pthread_mutex_unlock(&mutex);
		exit(1);
	}
	for(std::vector<Mind*>::iterator m = minds.begin(); m != minds.end(); ++m)
		(*m)->threadUpdate();
	rc = pthread_mutex_unlock(&mutex);
	return false;
}

void* AI::threadFunc(void* arg) {
	for(;;) {
		AI *ai = (AI*) arg;
		if(ai->threadUpdate())
			break;
	}
	return 0;
}

void AI::suspend() {
	for(std::vector<Mind*>::iterator m = minds.begin(); m != minds.end(); ++m)
		(*m)->abort();
	pthread_mutex_lock(&mutex);
}

void AI::resume() {
	pthread_mutex_unlock(&mutex);
}
