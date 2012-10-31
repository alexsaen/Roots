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

#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>


#ifdef WIN32

#include <SDL.h>

namespace platform {

inline unsigned int getTicks() {
	return SDL_GetTicks();
}

inline void sleep(unsigned int msec) {
	SDL_Delay(msec);
}

#elif defined(ANDROID)

#include <unistd.h>

namespace platform {

inline unsigned int getTicks() {
	return clock() / (CLOCKS_PER_SEC/1000);
}

inline void sleep(unsigned int msec) {
	usleep(msec*1000);
}

#endif


void saveSettings(const std::string &data);
std::string	loadSettings();


};

#endif
