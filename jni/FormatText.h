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

#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <vector>
#include <string>
#include "color.h"

class Font;
class Render;

enum TextAlign { TA_Left, TA_Right, TA_Center };

class FormatText {
public:
			FormatText(std::string str, Font &f, float s, const color4 &color1, const color4 &color2, float x, float y, float w=0, TextAlign align = TA_Left);
	void	draw(Render &render, float alpha = 1.0f);

private:
friend struct RichTextParser;

	Font& font;

	struct String {
		float x, y;
		std::string s;
		float	fontSize;
		color4 	color;
		String(std::string as, float ax, float ay, float fs, const color4 &c): x(ax), y(ay), s(as), fontSize(fs), color(c) {}
	};

	std::vector<String>	strings;

};

#endif
