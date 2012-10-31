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

#include "Sound.h"
#include "platform.h"
#include <AL/alc.h>
#include <string>

static size_t readOgg(void *ptr, size_t size, size_t nmemb, void *datasource) {
	ResourceManager::file* f = (ResourceManager::file*)datasource;
	return f->read(ptr, size * nmemb);
}

static int seekOgg(void *datasource, ogg_int64_t offset, int whence) {
	return -1;
}

static long tellOgg(void *datasource) {
	return -1;
}

static int closeOgg(void *datasource) {
	ResourceManager::file* f = (ResourceManager::file*)datasource;
	f->close();
	return 0;
}

void OGGStream::error(int code) {
    const char *error = "Unknown Ogg error.";
    switch(code) {
        case OV_EREAD:
            error = "Read from media.";
			break;
        case OV_ENOTVORBIS:
            error = "Not Vorbis data.";
			break;
        case OV_EVERSION:
            error = "Vorbis version mismatch.";
			break;
        case OV_EBADHEADER:
            error = "Invalid Vorbis header.";
			break;
        case OV_EFAULT:
            error = "Internal logic fault (bug or heap/stack corruption.";
			break;
        default:
			break;
    }
}

bool OGGStream::open(const char *filename) {
	oggFile = ResourceManager::instance()->open(filename);
    if(!oggFile)
        return false;

	ov_callbacks cb;
	cb.close_func = closeOgg;
	cb.read_func = readOgg;
	cb.seek_func = seekOgg;
	cb.tell_func = tellOgg;

	if (ov_open_callbacks(&oggFile, &oggStream, 0, -1, cb) < 0) {
        oggFile.close();
        return false;
    }
	vorbisInfo = ov_info(&oggStream, -1);
	return true;
}

int	OGGStream::get(void *buffer, int bufferSize) {
    int  size = 0;

	while(size < bufferSize) {
		int section;
        int result = ov_read(&oggStream, (char*)buffer + size, bufferSize - size, 0, 2, 1, &section);
        if(result > 0)
            size += result;
        else
            if(result < 0) {
                error(result);
				return -1;
			} else
                break;
    }

	return size;
}

void OGGStream::close() {
 	if(!oggFile) 
		return;
	ov_clear(&oggStream);	
}

void OGGStream::rewind() {
	ov_pcm_seek(&oggStream, 0);
	oggFile.rewind();
}

SoundStream::SoundStream(int bs, bool l): bufferSize(bs), source(0), loop(l)	{
	buffers[0] = buffers[1] = 0; 
}

SoundStream::~SoundStream()	{
	release();
}

void SoundStream::stop() {
	if(!source)
		return;
	alSourceStop(source);
	int queued=0;
    alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
    while(queued--) {
        ALuint buffer;
        alSourceUnqueueBuffers(source, 1, &buffer);
    }
	oggStream.close();
}

void SoundStream::release() {
	if(!source)
		return;
	stop();

	alDeleteSources(1, &source);
    alDeleteBuffers(2, buffers);
	source = 0;
}

void SoundStream::volume(float v) {
	alSourcef(source, AL_GAIN, v);
}

bool SoundStream::play(const char *path) {
	if(!source)	{
		alGenBuffers(2, buffers);
		alGenSources(1, &source);

		alSource3f(source, AL_POSITION, 0.0, 0.0, 0.0);
		alSource3f(source, AL_VELOCITY, 0.0, 0.0, 0.0);
		alSource3f(source, AL_DIRECTION, 0.0, 0.0, 0.0);
		alSourcef (source, AL_ROLLOFF_FACTOR, 0.0);
		alSourcei (source, AL_SOURCE_RELATIVE, AL_TRUE);
	} else 
		stop();	

	if(!oggStream.open(path))
		return false;

	int cnt = 0;
    if(getData(buffers[0]))
		cnt++;
	else
        return false;

    if(getData(buffers[1]))
        cnt++;
    
    alSourceQueueBuffers(source, cnt, buffers);
    alSourcePlay(source);
    
    return true;
}

bool SoundStream::playing() {
    ALenum state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING);
}

bool SoundStream::update() {
    int processed;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

    while(processed--) {
        ALuint buffer;
        alSourceUnqueueBuffers(source, 1, &buffer);
        check();

		if(!getData(buffer)) 
			return false;

		alSourceQueueBuffers(source, 1, &buffer);
		check();
    }

	if(!playing())
		alSourcePlay(source);

    return true;
}

bool SoundStream::getData(ALuint buffer) {
	const vorbis_info *info = oggStream.getInfo();
	if(!info)
		return false;

	static std::vector<char> pcm;
	pcm.resize(bufferSize);
	int size = oggStream.get(&pcm[0], pcm.size());

	if(loop && size < (int)pcm.size()) {
			oggStream.rewind();
			int s = oggStream.get(&pcm[size], pcm.size()-size);
			if(s <= 0)													// recovery after ugly rewind 
				size += oggStream.get(&pcm[size], pcm.size()-size);
	}

	alBufferData(buffer, info->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, &pcm[0], size, info->rate);
    check();
    
	if(size < (int)pcm.size())
        return false;

	return true;
}

void SoundStream::check() {
	int error = alGetError();
	if(error != AL_NO_ERROR) {
//		throw string("OpenAL error was raised.");
		return;
	}
}

MusicPlayer::MusicPlayer(): sstream(0x4000, false), state(ST_STOP), volume(0) {
#if USE_PTHREAD
	finish = suspended = false; 
	pthread_mutex_init(&mutex, 0);
	pthread_create(&thread, 0, threadFunc, this);
#endif
	sstream.volume(volume);
}

MusicPlayer::~MusicPlayer()	{
#if USE_PTHREAD
	if(!suspended)
		pthread_mutex_lock(&mutex);
	finish = true;
	pthread_mutex_unlock(&mutex);
	pthread_join(thread, 0);
	pthread_mutex_destroy(&mutex);
#endif
}

void MusicPlayer::suspend() {
	suspended = true;
	pthread_mutex_lock(&mutex);
}

void MusicPlayer::resume() {
	pthread_mutex_unlock(&mutex);
	suspended = false;
}

static MusicPlayer *player = 0;
MusicPlayer& MusicPlayer::instance() {
	if(!player)
		player = new MusicPlayer();
	return *player;
}

void MusicPlayer::destroy() {
	if(player) {
		delete player;
		player = 0;
	}
}

void MusicPlayer::play(const char *filename) {
	switch(state) {
		case ST_STOP:
			volume = 0;
			sstream.volume(volume);
			sstream.play(filename);
			state = ST_FADE_IN;
			break;
		case ST_FADE_IN:
		case ST_FADE_OUT:
		case ST_PLAYING:
			nextFilename = filename;
			state = ST_FADE_OUT;
			break;
		default:
			break;
	}	
}

static const float volumeStep = 0.001f;	// 1 second

void MusicPlayer::update(unsigned timeDelta) {
	switch(state) {
		case ST_PLAYING:
			if(!sstream.update())
				changeMusic();
			break;
		case ST_STOP:
			break;
		case ST_FADE_IN:
			volume = volume + volumeStep * timeDelta;
			if(volume >= 1.0f) {
				volume = 1.0f;
				state = ST_PLAYING;
			}
			sstream.volume(volume);
			sstream.update();
			break;
		case ST_FADE_OUT:
			volume = volume - volumeStep * timeDelta;
			if(volume <= 0) {
				volume = 0;
				sstream.volume(volume);
				if(!nextFilename.empty()) {
					sstream.play(nextFilename.c_str());
					nextFilename.clear();
					state = ST_FADE_IN;
				} else 
					state = ST_STOP;
			} else {
				sstream.volume(volume);
				sstream.update();
			}
			break;
		default:
			break;
	}	
}

#if USE_PTHREAD

void* MusicPlayer::threadFunc(void* arg) {
	unsigned tick = platform::getTicks();
	for(;;) {
		unsigned t =  platform::getTicks();
		int dt = std::min(t - tick, 300u);
		tick = t;
		MusicPlayer *mp = (MusicPlayer*) arg;
		if(mp->updateThread(dt))
			break;
		platform::sleep(100);
	}
	return 0;
}

bool MusicPlayer::updateThread(unsigned dt) {
	pthread_mutex_lock(&mutex);
	bool l_finish = finish;
	pthread_mutex_unlock(&mutex);
	if(l_finish)
		return true;
	update(dt);
	return false;
}

#endif

void  MusicPlayer::changeMusic() {
	if(playList.empty())
		return;
	play(playList[ rand() % playList.size() ].c_str());
}


//////////////////////////////

SoundBuffer::SoundBuffer(): id(0) {}

SoundBuffer::~SoundBuffer() {
	release();
}

void SoundBuffer::release() {
	if(id) {
		alDeleteBuffers(1, &id);
		id = 0;
	}
}

bool SoundBuffer::load(const char *filename) {
	if(!id)
		alGenBuffers(1, &id);
	if(!id)
		return false;
	OGGStream ogg;
	if(!ogg.open(filename))
		return false;
	const vorbis_info *info = ogg.getInfo();
	if(!info)
		return false;

	const size_t loadPartSize = 0x8000;
	static std::vector<char> buffer(loadPartSize*2);

	size_t pos = 0;
	int size = 0;

	for(;;) {
		int	sz = ogg.get(&buffer[pos], loadPartSize);
		size += sz;
		if(sz < loadPartSize)
			break;
		if(buffer.size() < size + loadPartSize)
			buffer.resize(size + loadPartSize);
		pos += loadPartSize;
	}

	alBufferData(id, info->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, &buffer[0], size, info->rate);
	return true;
}

SoundSource::SoundSource(): id(0) {}

SoundSource::SoundSource(SoundBuffer *sb): id(0) {
	attach(sb);
}

SoundSource::~SoundSource() {
	release();
}

void SoundSource::attach(SoundBuffer *sb) {
	if(!id)
		alGenSources(1, &id);
	alSourcei(id, AL_BUFFER, sb->id);
}

void SoundSource::release() {
	if(id) {
		alSourceStop(id);
		alDeleteSources(1, &id);
		id = 0;
	}
}

void SoundSource::play() {
	if(!id)
		return;
	if(playing())
		return;
	alSourcePlay(id);
}

void SoundSource::stop() {
	if(!id)
		return;
	alSourceStop(id);
}
			
bool SoundSource::playing() {
	if(!id)
		return false;
    ALenum state;
    alGetSourcei(id, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING);
}

void SoundSource::position(const vec2 &p) {
	float pos[3] = { p[0], p[1], 0 };
	alSourcefv(id, AL_POSITION, pos);
}

////////////////////////////////////

static ALCdevice* device = 0;
static ALCcontext* context = 0;

static const char* soundPath = "assets/sounds/";
static const char* soundFiles[SOUNDFX_SIZE] = {
	"newRootlet.ogg",
	"outRootlet.ogg",
	"weakTree.ogg",
	"killTree.ogg",
	"startLevel.ogg",
	"levelComplete.ogg",
	"gameOver.ogg",
	"papapam.ogg"
};

Sound::Sound() {
	device = alcOpenDevice(0);
	const ALint context_attribs[] = { ALC_FREQUENCY, 16000, 0 };
	context = alcCreateContext(device, context_attribs);
	alcMakeContextCurrent(context);

	buffers.resize(SOUNDFX_SIZE);
	for(int i = 0; i < SOUNDFX_SIZE; ++i) 
		buffers[i].load( (std::string(soundPath) + soundFiles[i]).c_str() );
}

Sound::~Sound() {
	alcMakeContextCurrent(0);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

static Sound *sound = 0;
Sound& Sound::instance() {
	if(!sound) 
		sound = new Sound();
	return *sound;
}

void Sound::destroy() {
	if(sound) {
		delete sound;
		sound = 0;
	}
}

void Sound::volume(float v) {

}

SoundBuffer	*Sound::getBuffer(SoundType snd) {
	return &buffers[snd];
}



