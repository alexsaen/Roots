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

#ifndef JSONPOARSER_H
#define JSONPOARSER_H

#include <vector>
#include <string>

class JSONParser {
protected:
	enum ObjectType	{	JST_Root, JST_Object, JST_Array	};
	struct Object {
		std::string	name;
		int		type;
		Object()	{}
		Object(const char *n, int t);
	};
	enum ValueType	{	JST_String, JST_Int, JST_Float, JST_Bool };
	struct Variant {
		union {
			const char *stringValue;
			int			intValue;
			float		floatValue;
			bool		boolValue;
		};
		ValueType	type;
		Variant(const char *s): stringValue(s), type(JST_String)	{}
		Variant(int i): intValue(i), type(JST_Int)					{}
		Variant(float f): floatValue(f), type(JST_Float)			{}
		Variant(bool b): boolValue(b), type(JST_Bool)				{}
		float	getFloat() {
			switch(type) {
				case JST_Int:
					return (float)intValue;
				case JST_Float:
					return floatValue;
				default:
					return 0;
			}
		}
	};
	std::vector<Object> stack;
	const char	*is;
	char		buffer[100], buffer2[255];
			char		getChar();
			void		putBack();
			void		eatSpaces();
			bool		isDelimiter(char c);
			const char*	getName();
			const char* getQuotes(char c);
			const char* getToken();
			bool		getValue();
			bool		getObject();
public:
	const char	*error;
	int			lineno, pos;
					JSONParser();
	virtual			~JSONParser()											{}
			bool	parse(const char* src);
	virtual bool	objectStart(const std::string &name)					{	return true;	}
	virtual bool	objectEnd(const std::string &name)						{	return true;	}
	virtual bool	arrayStart(const std::string &name)						{	return true;	}
	virtual bool	arrayEnd(const std::string &name)						{	return true;	}
	virtual	bool	variable(const std::string &name, Variant value)		{	return true;	}
};



#endif
