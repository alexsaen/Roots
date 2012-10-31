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

#include "Texture.h"
#include "opengl.h"
#include "ResourceManager.h"
#include <string>

Texture::Texture(): tid(0) {}

void Texture::release() {
	if(tid) {
		glDeleteTextures(1, &tid);
		tid = 0;
	}
}

bool Texture::loadFromPNG(const char *filename) {
	const char *texPath = "assets/textures/";
	std::string path(texPath);
	path += filename;

	int width, height;
	unsigned char* png = ResourceManager::instance()->readPNG(path.c_str(), width, height);
	if(!png)
		return false;

	init(png, width, height, 3);
	delete png;
	return true;
}

void Texture::init(unsigned char *buf, int width, int height, int depth) {
	if(!tid)
		glGenTextures(1, &tid);

	glBindTexture(GL_TEXTURE_2D, tid);

	GLenum format = depth == 3 ? GL_RGB : GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, buf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Texture::bind() {
	if(tid)
		glBindTexture(GL_TEXTURE_2D, tid);
}

void Texture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

