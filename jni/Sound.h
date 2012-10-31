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

#ifndef SOUND_H
#define SOUND_H

#define USE_PTHREAD	1

#include "ResourceManager.h"
#include "math2d.h"
#include <vorbis/vorbisfile.h>
#include <vector>
#include <AL/al.h>

#if USE_PTHREAD
#include <pthread.h>
#endif

class OGGStream {
	ResourceManager::file	oggFile;
	vorbis_info*    vorbisInfo;
	OggVorbis_File  oggStream;
	void	error(int code);
public:
						OGGStream()						{}
						~OGGStream()					{	close();			}	
	bool				open(const char *filename);
	int					get(void *buffer,int size);
	void				rewind();
	const vorbis_info*	getInfo()						{	return vorbisInfo;	}	
	void				close();
};

class SoundResource {
public:
				SoundResource()			{}
	virtual		~SoundResource()		{}
};

class SoundStream: public SoundResource {
	OGGStream	oggStream;
	int			bufferSize;
    ALuint		buffers[2];
    ALuint		source;
	bool		loop;
	bool	getData(ALuint buffer);
    void	check();
public:
					SoundStream(int bufferSize = 0x4000, bool loop=false);
	virtual			~SoundStream();
			bool	play(const char *filename);
			void	stop();
			bool	playing();
			bool	update();
			void	volume(float v);
			void	release();
};

class MusicPlayer {
	SoundStream	sstream;
	enum State {
		ST_FADE_IN,
		ST_FADE_OUT,
		ST_PLAYING,
		ST_STOP
	};
	State	state;
	float	volume;
	std::string nextFilename;
#if USE_PTHREAD
	bool	finish, suspended;
	pthread_t	thread;
	pthread_mutex_t	mutex;
	static	void*	threadFunc(void* arg);
	bool	updateThread(unsigned dt);
#endif
	std::vector<std::string>	playList;
			MusicPlayer();
			~MusicPlayer();
public:
	static	MusicPlayer&	instance();
	static	void	destroy();

	void	play(const char *filename);
	void	update(unsigned timeDelta);
	void	addToPlayList(const char *filename)		{	playList.push_back(filename);	}
	void	changeMusic();
	void	suspend();
	void	resume();
};

class SoundBuffer: public SoundResource {
friend class SoundSource;
    ALuint		id;
public:
					SoundBuffer();
	virtual			~SoundBuffer();
			void	release();
			bool	load(const char *filename);
};

class SoundSource: public SoundResource {
    ALuint		id;
public:
					SoundSource();
					SoundSource(SoundBuffer *b);
	virtual			~SoundSource();
			void	attach(SoundBuffer *b);
			void	release();
			void	position(const vec2 &p);
			void	play();
			void	stop();
			bool	playing();
};

////////////////////////////////////////////

enum SoundType {
	SND_NEW_ROOTLET,
	SND_OUT_ROOTLET,
	SND_WEAK_TREE,
	SND_KILL_TREE,
	SND_START,
	SND_LEVEL_COMPLETE,
	SND_GAME_OVER,
	SND_PAPAPAM,
	SOUNDFX_SIZE
};

class Sound {
	std::vector<SoundBuffer>	buffers;
			Sound();
			~Sound();
public:
	static	Sound&	instance();
	static	void	destroy();
	SoundBuffer	*getBuffer(SoundType snd);
	void	volume(float v);
};


#endif
