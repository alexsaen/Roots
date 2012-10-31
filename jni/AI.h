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

#ifndef AI_H
#define AI_H

#include <vector>
#include <pthread.h>

class Planet;

class Mind {
	int		playerIdx, depth, stack, stackSize;
	unsigned int ticks;
	float	alphaBeta(int pIdx, int depth, float alpha, float beta);

	struct MTreeData {
		bool  canLink;
		float doNothingFactor;
		float growing, accumulator;
		MTreeData(): canLink(true), doNothingFactor(1.0f), growing(0), accumulator(0) {}
	};

	struct MTree {
		float length, treeLength;
		int	  links;
		MTree(): length(-1), treeLength(-1), links(0)	{}
	};

	struct MLink {
		int	race;
		int from, to;
		float length;
		bool canUnlink;
		MLink()																	{}
		MLink(int r, int f, int t, float l, bool cul): race(r), from(f), to(t), length(l), canUnlink(cul)	{}
	};

	std::vector<MTree>	trees;
	std::vector<MTreeData>	treesData;
	std::vector<MLink>	links;
	std::vector<int>	linksIdx;
	MTree&	tree(int race, int planet);
	MTreeData&	treeData(int race, int planet);
	enum MoveType {
		MT_NOTING,
		MT_LINK,
		MT_UNLINK
	};

	struct Move {
		MoveType	type;
		int from, to;
		union {
			float length;
			int	  idx;
		};
		Move()																			{}
		Move(MoveType t): type(t), from(-1), to(-1), length(0)							{}
		Move(MoveType t, int f, int tt, float l): type(t), from(f), to(tt), length(l)	{}
		Move(MoveType t, int i, int f, int tt): type(t), idx(i), from(f), to(tt)		{}
	};

	enum State {
		ST_STOP,
		ST_SEARCHING,
		ST_READY,
		ST_ABORT
	};

	Move			bestMove;
	State			state;

	void	clear();
	void	initPosition();
	void	addLink(int pidx, int from, int to);
	void	calcMoves(int pIdx, std::vector<Move> &moves);
	void	dublicateStack();
	void	proceedMove(float step);
	void	makeMove(int pIdx, const Move &m);
	void	undoMove();
	float	evaluate(int pidx);
	bool	isLooser(int pidx);

	bool	haveLink(int pIdx, int from, int to);
	Move	searchBestMove(int depth);
public:
			Mind(int pidx, int d);
			~Mind();
	bool	update();
	bool	threadUpdate();
	void	abort();
};

class AI {
	std::vector<Mind*> minds;
	bool	threadUpdate();
	static	void*	threadFunc(void* arg);

	pthread_t		thread;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	bool			finish;
public:
			AI();
			~AI();
	void	clear();
	void	add(Mind *m);
	void	update();
	void	abort();
	void	suspend();
	void	resume();
};

#endif

