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

#ifndef FONT_H
#define FONT_H

#include "RenderResource.h"
#include "math2d.h"
#include <vector>

class FontMaker;
struct FontDrawer;

class Font: public RenderResource {
friend class FontMaker;
friend struct FontDrawer;
friend struct FontDrawer2D;
friend class FontDrawer3D;

	struct FontChar {
		vec2		p, s, tp, ts;
		float		width;
		FontChar()																	{}
		FontChar(float x, float y, float w, float h, float tx, float ty, float tw, float th, float charWidth):
			p(x, y), s(w, h), tp(tx, ty), ts(tw, th), width(charWidth) 				{}
	};

protected:
	unsigned int	size;

	struct CharsRange {
		unsigned int start, end;
		unsigned int textureIdx;
		std::vector<int> charsIdx;
		CharsRange()	{}
		CharsRange(unsigned int s, unsigned int e, unsigned int t);
	};

	std::vector<unsigned int> 	textures;
	std::vector<FontChar>		chars;
	std::vector<CharsRange>		charsRanges;
	
			bool 	initTexture(FontMaker &maker, const int *range);
			void 	draw(FontDrawer *fd, const char *str);

public:

					Font();
	virtual	void	release();

			bool 	init(const char *filename, int fontSize);
			void	draw(float x, float y, float size, const char *str);
			float	width(const char *str);
};


#endif
