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

#ifndef SHADER_H
#define SHADER_H

#include "RenderResource.h"
#include "math2d.h"
#include "color.h"

class Shader: public RenderResource {
friend class ShaderProgram;
	unsigned int	shaderType;
	unsigned int	shaderID;
public:
					Shader(unsigned int ShaderType);
	virtual	void	release();
			void	load(const char *fileNameShader);
			void	init(const char *shaderSrc);

};

class VertexShader: public Shader {
public:
	VertexShader();
};

class FragmentShader: public Shader {
public:
	FragmentShader();
};

enum STD_UNIFORM {
	SU_TRANSFORM,
	SU_INDEX,
	SU_COLOR,
	SU_TIME,
	SU_TEX0,
	SU_TEX1,
	SU_TEX2,
	SU_FINISH
};

class ShaderProgram: public RenderResource {
public:
	unsigned int	programID;
	Shader *vertexShader, *fragmentShader;

	int	uniformIDs[SU_FINISH];
	void	getUniformIDs();
public: 
					ShaderProgram();
	virtual	void	release();
			void	use();
	static	void	unuse();

			void	uniform(const char *uid, int value);
			void	uniform(const char *uid, float value);
			void	uniform(const char *uid, const vec2 &value);
			void	uniform(const char *uid, const color4 &value);
			void	uniform(const char *uid, const mat4 &value);

			void	uniform(STD_UNIFORM id, int value);
			void	uniform(STD_UNIFORM id, float value);
			void	uniform(STD_UNIFORM id, const vec2 &value);
			void	uniform(STD_UNIFORM id, const color4 &value);
			void	uniform(STD_UNIFORM id, const mat4 &value);

			void	bindAttrib(int id, const char *name);

			void	attach(VertexShader *vShader, FragmentShader *fShader);
			void	link();

};

#endif
