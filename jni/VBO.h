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

#ifndef VBO_H
#define VBO_H

#include "opengl.h"
#include "RenderResource.h"

#include <vector>

class VBO: public RenderResource {
	unsigned int id, target, size;
public:
					VBO(int target);
	virtual	void	release();
			void	bind();
			void	unbind()					{	glBindBuffer(target, 0);					}	
	static	void	unbindVertex()				{	glBindBuffer(GL_ARRAY_BUFFER, 0);			}
	static	void	unbindIndex()				{	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	}

			void	setData(unsigned size, GLenum usage = GL_STATIC_DRAW, const void *ptr = 0);
			void	setSubData(unsigned offs, unsigned size, const void *ptr);
};

class VBOVertex: public VBO {
public:
	VBOVertex(): VBO(GL_ARRAY_BUFFER)	{}
};

class VBOIndex: public VBO {
public:
	VBOIndex(): VBO(GL_ELEMENT_ARRAY_BUFFER)	{}
};

#endif
