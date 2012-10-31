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

#include "VBO.h"
#include <algorithm>
#include <vector>

VBO::VBO(int a_target): id(0), target(a_target), size(0) {
}

void VBO::release() {
	if(id) {
		glDeleteBuffers(1, &id);
		id = 0;
	}
}

void VBO::bind() {
	if(!id)
		glGenBuffers(1, &id);
    glBindBuffer(target, id);
}

void VBO::setData(unsigned sz, GLenum usage, const void *ptr) {
	size = sz;
    glBufferData(target, size, ptr, usage);
}

void VBO::setSubData(unsigned offs, unsigned size, const void *ptr) {
    glBufferSubData(target, offs, size, ptr);
}


