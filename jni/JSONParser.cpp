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

#include "JSONParser.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

JSONParser::Object::Object(const char *n, int t): name(n), type(t)	{ } 

JSONParser::JSONParser() {}

char JSONParser::getChar() {
	char c=*is++;
	pos++;
	if(c=='\n') {
		lineno++;
		pos=0;
	}
	return c;
}

void JSONParser::putBack() {
	is--;
	pos--;
}

void JSONParser::eatSpaces() {
    for(;;) {
        char c=getChar();
		if(c==0) return;
        if(!isspace(c)) {
            putBack();
            break;
        }
    } 
}

inline bool JSONParser::isDelimiter(char c) {
	return !(isalpha(c) || isdigit(c) || strchr("_-.",c));	
}

const char* JSONParser::getName() {
	buffer[0]=getChar();
	char finishChar=0;
	if(buffer[0]=='"' || buffer[0]=='\'') {
		finishChar=buffer[0];
		buffer[0]=getChar();
	}
	if(!isalpha(buffer[0])) {
		error="The name must be begin with an alpha.";
		return 0;
	}
	for(int sz=1; sz<sizeof(buffer); sz++) {
		buffer[sz]=getChar();
		if(buffer[sz]==finishChar) {
			if(!buffer[sz]) return 0;
			buffer[sz]=0;
			return buffer;
		} else if(!finishChar && isDelimiter(buffer[sz])) {
			putBack();
			buffer[sz]=0;
			return buffer;
		}
	}
	error="Buffer overflow.";
	return 0;
}

const char* JSONParser::getToken() {
	for(int sz=0; sz<sizeof(buffer2); ++sz) {
		buffer2[sz]=getChar();
		if(!buffer2[sz]) return 0;
		if(isDelimiter(buffer2[sz])) {
			putBack();
			buffer2[sz]=0;
			return buffer2;
		}
	}
	error="Buffer overflow.";
	return 0;
}

const char* JSONParser::getQuotes(char c) {
	for(int sz=0; sz<sizeof(buffer2); ++sz) {
		buffer2[sz]=getChar();
		if(!buffer2[sz]) return 0;
		if(buffer2[sz]==c) {
			buffer2[sz]=0;
			return buffer2;
		}
	}
	error="Buffer overflow.";
	return 0;
}

bool JSONParser::getObject() {
	for(;;) {
		eatSpaces();
		const char *name=getName();
		if(!name) return false;
		eatSpaces();
		if(getChar()!=':') {
			error="Missing ':'.";
			return false;
		}
		if(!getValue()) return false;
		eatSpaces();
		switch(getChar()) {
			case ',':
				continue;
			case '}':
				if(!stack.empty()) {
					switch(stack.back().type) {
						case JST_Object:
							if(!objectEnd(stack.back().name)) 
return false;
						case JST_Root:
							break;
						default:
							return false;
					}
					stack.pop_back();
					return true;
				} 
			default:
				return false;

		}
	}
}

bool JSONParser::getValue() {
	char c;
	eatSpaces();
	switch(c=getChar()) {
		case '\"':
		case '\'':
			if(!getQuotes(c)) return false;
			return variable(buffer, Variant(buffer2));
		case '{':
			stack.push_back(Object(buffer, JST_Object));
			if(!objectStart(buffer)) return false;
			return getObject();
		case '[':
			stack.push_back(Object(buffer, JST_Array));
			if(!arrayStart(buffer)) return false;
			if(getChar()!=']') {
				buffer[0]=0;
				putBack();
				for(;;) {
					if(!getValue())	return false;
					eatSpaces();
					switch(getChar()) {
						case ',':					
							continue;
						case ']':
							if(stack.empty() || stack.back().type!=JST_Array) return false;
							goto arrayFinish;
					}
				}
			} 
			arrayFinish:
			if(!arrayEnd(stack.back().name)) return false;
			stack.pop_back();
			return true;
		default:
			putBack();
			if(!getToken()) return false;
			if(strcmp(buffer2, "true")==0) {
				return variable(buffer, Variant(true));
			} else if(strcmp(buffer2, "false")==0) {
				return variable(buffer, Variant(false));
			} else if(strchr(buffer2, '.')) {
				float val=(float)atof(buffer2);
				return variable(buffer, val);
			} else {
				int val=atoi(buffer2);
				return variable(buffer, val);
			}
	}
}

bool JSONParser::parse(const char* src) {
	is=src;
	error="";
	lineno=1;
	eatSpaces();
	char c=getChar();
	if(stack.empty() && c!='{') {
		error="Missing \'{\'.";
		return false;
	} 
	stack.push_back(Object("", JST_Root));
	return getObject();
}
