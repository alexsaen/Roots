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

#include "FBO.h"
#include "opengl.h"

FBO::FBO(): width(0), height(0), texID(0), fboID(0) {
}

void FBO::release() {
	if(texID) {
		glDeleteTextures(1, &texID);
		texID = 0;
	}
	if(fboID) {
		glDeleteFramebuffers(1, &fboID);
		fboID = 0;
	}
}

bool FBO::reshape(int w, int h) {
	if(!texID)
		glGenTextures(1, &texID);
	if(!texID)
		return false;

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap, it works only for glCopyTexSubImage2D()
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if(!fboID)
		glGenFramebuffers(1, &fboID);
	if(!fboID)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	width = w;
	height = h;
	return status == GL_FRAMEBUFFER_COMPLETE;
}

void FBO::bind() {
	if(!fboID)
		return;
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	glViewport(0, 0, width, height);
}

void FBO::bindTexture() {
	if(texID)
		glBindTexture(GL_TEXTURE_2D, texID);
}

void FBO::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::unbindTexture() {
	glBindTexture(GL_TEXTURE_2D, 0);
}
