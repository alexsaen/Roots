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

#include "Shader.h"
#include "opengl.h"
#include "ResourceManager.h"
#include <fstream>
#include <string>

Shader::Shader(unsigned int shType): shaderType(shType), shaderID(0) {}

void Shader::release() {
	if(shaderID) {
		glDeleteShader(shaderID);
		shaderID = 0;
	}
}

void Shader::init(const char *shaderSrc) {
	char log[1024];
	GLsizei logsize;
	if(!shaderID)
		shaderID = glCreateShader(shaderType);
	glShaderSource(shaderID, 1, &shaderSrc, 0);
	glCompileShader(shaderID);
	glGetShaderInfoLog(shaderID, sizeof(log), &logsize, log);
	const char *l=log;
}

void Shader::load(const char *fileNameShader) {
	std::string path("assets/shaders/");
	path += fileNameShader;
	int size;
	const char *data = (const char*)ResourceManager::instance()->loadFile(path.c_str(), size);
	if(data) {
		init(data);
		delete data;
	}
}

VertexShader::VertexShader(): Shader(GL_VERTEX_SHADER)			{}
FragmentShader::FragmentShader(): Shader(GL_FRAGMENT_SHADER)	{}

ShaderProgram::ShaderProgram(): programID(0), vertexShader(0), fragmentShader(0) {
}

void ShaderProgram::release() {
	if(programID) {
		glDeleteProgram(programID);
		programID = 0;
		vertexShader = 0;
		fragmentShader = 0;
	}
}

void ShaderProgram::attach(VertexShader *vShader, FragmentShader *fShader) {
	if(!programID) 
		programID = glCreateProgram();
	if(!vertexShader) {
		vertexShader = vShader;
		glAttachShader(programID, vertexShader->shaderID);
	}
	if(!fragmentShader) {
		fragmentShader = fShader;
		glAttachShader(programID, fragmentShader->shaderID);
	}
}

void ShaderProgram::link() {
	glLinkProgram(programID);

	char log[1024];
	GLsizei logsize;
	glGetProgramInfoLog(programID, sizeof(log), &logsize, log);
	const char *l=log;
	getUniformIDs();
}

void ShaderProgram::use() {
	glUseProgram(programID);
}

void ShaderProgram::unuse() {
	glUseProgram(0);
}

void ShaderProgram::bindAttrib(int id, const char *name) {
	glBindAttribLocation(programID, id, name);
}

void ShaderProgram::getUniformIDs() {
	static const char *strUniform[SU_FINISH] = { 
		"transform", "index", "color", "time", "tex0", "tex1", "tex2" };
	for(int i=0; i<SU_FINISH; ++i) 
		uniformIDs[i] = glGetUniformLocation(programID, strUniform[i]);
}

void ShaderProgram::uniform(const char *uid, int value) {
	int id = glGetUniformLocation(programID, uid);
	glUniform1i(id, value);
}

void ShaderProgram::uniform(const char *uid, const vec2 &value) {
	int id = glGetUniformLocation(programID, uid);
	glUniform2fv(id, 1, value);
}

void ShaderProgram::uniform(const char *uid, const float value) {
	int id = glGetUniformLocation(programID, uid);
	glUniform1f(id, value);
}

void ShaderProgram::uniform(const char *uid, const color4 &value) {
	int id = glGetUniformLocation(programID, uid);
	glUniform4fv(id, 1, value);
}

void ShaderProgram::uniform(const char *uid, const mat4 &value) {
	int id = glGetUniformLocation(programID, uid);
	glUniformMatrix4fv(id, 1, GL_FALSE, value);
}

////////// std uniform

void ShaderProgram::uniform(STD_UNIFORM id, int value) {
	if(uniformIDs[id] < 0)
		return;
	glUniform1i(uniformIDs[id], value);
}

void ShaderProgram::uniform(STD_UNIFORM id, const vec2 &value) {
	if(uniformIDs[id] < 0)
		return;
	glUniform2fv(uniformIDs[id], 1, value);
}

void ShaderProgram::uniform(STD_UNIFORM id, const float value) {
	if(uniformIDs[id] < 0)
		return;
	glUniform1f(uniformIDs[id], value);
}

void ShaderProgram::uniform(STD_UNIFORM id, const color4 &value) {
	if(uniformIDs[id] < 0)
		return;
	glUniform4fv(uniformIDs[id], 1, value);
}

void ShaderProgram::uniform(STD_UNIFORM id, const mat4 &value) {
	if(uniformIDs[id] < 0)
		return;
	glUniformMatrix4fv(uniformIDs[id], 1, GL_FALSE, value);
}

