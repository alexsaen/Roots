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

#include "FormatText.h"
#include "Font.h"
#include "Render.h"

struct RichTextParser {
	color4 color1, color2;
	float x, y, w, fontSize, sp;
	FormatText &rt;
	TextAlign align;

	std::string ss, ts;
	float ww, xx;
	color4 color;
	int lidx;

	RichTextParser(FormatText &art, float s, const color4 &c1, const color4 &c2, float ax, float ay, float aw, TextAlign a): color1(c1), color2(c2), x(ax), y(ay), w(aw), fontSize(s), rt(art), align(a), ww(0) {
		sp=rt.font.width(" ")*fontSize;
	}

	void alignLine() {
		if(align == TA_Left)
			return;
		float ls = rt.strings.back().x + rt.font.width(rt.strings.back().s.c_str())*fontSize - rt.strings[lidx].x;
		float delta = w - ls;
		if(align == TA_Center)
			delta *= 0.5f;
		for(int i=lidx; i<(int)rt.strings.size(); ++i)
			rt.strings[i].x += delta;
		lidx = rt.strings.size();
	}

	void pushWord() {
		if(ts.empty())
			return;
		float ws = rt.font.width(ts.c_str())*fontSize;
		if(ww + ws + sp + xx < w) {
			if(!ss.empty())
				ss+=' ';
			ss += ts;
			ww += ws + sp;
		} else {
			if(!ss.empty())
				rt.strings.push_back(FormatText::String(ss, xx, y, fontSize, color));
			alignLine();
			ss=ts;
			ww=ws;
			y-=fontSize;
			xx = x;
		}
		ts.clear();
	}

	void flush() {
		pushWord();
		if(ss.empty())
			return;
		float ws = rt.font.width(ss.c_str())*fontSize;
		rt.strings.push_back(FormatText::String(ss, xx, y, fontSize, color));
		ww = 0;
		xx += ws + sp;
		ss.clear();
	}

	void br() {
		flush();
		alignLine();
		ww = 0;
		y-=fontSize;
		xx = x;
	}

	void parse(std::string str) {
		lidx = 0;
		color = color1;
		xx=x;
		for(unsigned idx=0;;idx++) {
			if(idx>=str.length()) {
				flush();
				alignLine();
				break;
			}

			char ch = str[idx];
			switch(ch) {
				case '<':
					{
					bool tagClosed = false;
					if(str[idx+1] == '/') {
						idx++;
						tagClosed = true;
					}
					idx++;
					std::string tag;
					while(idx<str.length() && str[idx]!='>')
						tag += str[idx++];
					if(tag == "em") {
						flush();
						color = tagClosed ? color1 : color2;
					} else if(tag == "br") {
						br();
					}
					}
					break;
				case ' ':
					pushWord();
					break;
				default:
					ts+=ch;
					break;
			}

		}
	}

};

FormatText::FormatText(std::string str, Font &f, float s, const color4 &color1, const color4 &color2, float x, float y, float w, TextAlign align): font(f) {
	RichTextParser p(*this, s, color1, color2, x, y, w<=0 ? 0xffffff : w, align);
	p.parse(str);
}

void FormatText::draw(Render &render, float alpha) {
	for(std::vector<String>::iterator is=strings.begin(); is!=strings.end(); ++is) {
		render.setColor( color4(is->color[0], is->color[1], is->color[2], is->color[3]*alpha) );
		font.draw(is->x, is->y, is->fontSize, is->s.c_str());
	}
}
