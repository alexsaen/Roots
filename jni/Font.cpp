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

#include "Font.h"
#include "ResourceManager.h"
#include "Render.h"
#include "opengl.h"
#include "utf8/unchecked.h"
#include <algorithm>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

class FontMaker {
	bool 	initialized;
	int 	fontSize, texWidth, texHeight, posX, posY, sweepY;
    FT_Library 	library;
    FT_Face 	face;
    const FT_Byte *filedata;
    std::vector<char>	buffer;
public:
	FontMaker(const char *filename): initialized(false), library(0), face(0), filedata(0) {
	    if(FT_Init_FreeType( &library ))
	        return;

		int fsize;
		filedata = (FT_Byte*)ResourceManager::instance()->loadFile(filename, fsize);
		if(!filedata)
			return;

	    if(FT_New_Memory_Face(library, filedata, fsize, 0, &face))
	    	return;

	    initialized=true;
	}

	~FontMaker() {
		if(face)
			FT_Done_Face(face);
		if(library)
			FT_Done_FreeType(library);
		if(filedata)
			 delete filedata;
	}

	void 	*makeTexture(int fontSize, int &texWidth, int &texHeight, Font *font, const int *ranges);
	bool 	drawGlyph(unsigned int c, Font *font);
	void 	clear();

};

void FontMaker::clear() {
	buffer.clear();
	posX = posY = sweepY = 0;
	texHeight = texWidth = 0;
}

void *FontMaker::makeTexture(int fsize, int &tWidth, int &tHeight, Font *font, const int *ranges) {
	if(!initialized)
		return 0;

	clear();

	fontSize = fsize;

	if(FT_Set_Char_Size( face, fontSize << 6, fontSize << 6, 0, 0))
		return 0;

	texWidth = clp2(fontSize*16);
	texHeight = texWidth >> 3;

	buffer.resize(texWidth * texHeight);
	memset(&buffer[0], 0, buffer.size());

	int cnt = 0;

	for(; ranges[0]; ranges+=2) {
		int start=ranges[0], end=ranges[1];

		font->charsRanges.push_back( Font::CharsRange(start, end, font->textures.size()) );
		Font::CharsRange &cr = font->charsRanges.back();

		int c = 0;

		for(int i=start; i<=end; ++i) {
			if(drawGlyph(i, font)) {
				cr.charsIdx[i-start] = font->chars.size() - 1;
				c++;
			}
		}

		if(c == 0)						// empty range
			font->charsRanges.pop_back();
		cnt += c;
	}

	if(cnt==0)
		return 0;

	tWidth = texWidth;
	tHeight = texHeight;

	return &buffer[0];
}

bool FontMaker::drawGlyph(unsigned int ch, Font *font) {
	unsigned chIdx = FT_Get_Char_Index( face, ch );
	if(chIdx==0)
		return false;

	if(FT_Load_Glyph( face, chIdx, FT_LOAD_DEFAULT ))
		return false;

	FT_Glyph glyph;
	if(FT_Get_Glyph( face->glyph, &glyph ))
		return false;

	if( glyph->format != FT_GLYPH_FORMAT_BITMAP ) {
		if(FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1 ))
			return false;
	}

	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

	FT_Bitmap& bitmap=bitmap_glyph->bitmap;

	if(posX + bitmap.width + 2 > texWidth) {
		posY = sweepY;
		posX = 0;
	}

	if(posY + bitmap.rows + 2 > texHeight) {	// texture growing up
		texHeight <<= 1;
		buffer.resize(texWidth * texHeight);
		int s = buffer.size() >> 1;
		memset(&buffer[s], 0, s);

		for(std::vector<Font::CharsRange>::reverse_iterator cr = font->charsRanges.rbegin(); cr != font->charsRanges.rend(); ++cr) {
			if(cr->textureIdx != font->textures.size())
				break;
			for(std::vector<int>::iterator i = cr->charsIdx.begin(); i != cr->charsIdx.end(); ++i) {
				if(*i < 0)
					continue;
				Font::FontChar &c = font->chars[*i];
				c.tp.y *= 0.5f;
				c.ts.y *= 0.5f;
			}
		}
	}

	int idx = (posY+1) * texWidth + posX + 1;	// copy glyph bitmap to texture
	for(int x = 0; x < bitmap.width; ++x )
		for(int y = 0; y < bitmap.rows; ++y )
			buffer[idx + y*texWidth+x] = bitmap.buffer[ y*bitmap.width+x ];

	float s = 1.0f / (fontSize+2);
	float tsx = 1.0f / texWidth, tsy = 1.0f / texHeight;
	int gw = bitmap.width + 2, gh = bitmap.rows + 2;
	font->chars.push_back( Font::FontChar( (bitmap_glyph->left - 2) * s, (bitmap_glyph->top - gh + 1) * s, gw * s,   gh * s,
								     (posX-0.5f) * tsx,            (posY-0.5f) * tsy,                gw * tsx, gh * tsy,
								     (face->glyph->advance.x >> 6) * s) );

	posX += bitmap.width + 2;
	sweepY = std::max(sweepY, posY + bitmap.rows + 2);

	FT_Done_Glyph( glyph );
	return true;
}

Font::CharsRange::CharsRange(unsigned int s, unsigned int e, unsigned int t): start(s), end(e), textureIdx(t)	{
	charsIdx.resize(end - start + 1);
	std::fill(charsIdx.begin(), charsIdx.end(), -1);
}


Font::Font(): size(0) {}

void Font::release() {
	if(!textures.empty())
		glDeleteTextures(textures.size(), &textures[0]);
	textures.clear();
	chars.clear();
	charsRanges.clear();
}

bool Font::init(const char *filename, int fontSize) {
	size = fontSize;

	FontMaker maker(filename);

	int ranges[] = { 32, 127, 0x400, 0x45f, 0 };

	return initTexture(maker, ranges);
}

bool Font::initTexture(FontMaker &maker, const int *range) {
	int texWidth, texHeight;

	void *data = maker.makeTexture(size, texWidth, texHeight, this, range);

	if(!data)
		return false;

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texWidth, texHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	textures.push_back(texture);
	return true;
}

float Font::width(const char *str) {
	float result = 0;
	for(unsigned int cp=utf8::unchecked::next(str); cp; cp=utf8::unchecked::next(str)) {
		std::vector<CharsRange>::iterator r = charsRanges.begin();
		for(; r != charsRanges.end(); ++r) {
			if(r->start <= cp && cp <=r->end)
				break;
		}
		if(r == charsRanges.end())
			continue;
		int idx = r->charsIdx[cp - r->start];
		if(idx >= 0)
			result += chars[idx].width;
	}
	return result;
}

struct	FontDrawer {
	virtual	void addChar(const Font::FontChar &fc)=0;
	virtual	void flush()=0;
};

void Font::draw(FontDrawer *fd, const char *str) {

	unsigned int bindTexture = 0;

	for(unsigned int cp=utf8::unchecked::next(str); cp; cp=utf8::unchecked::next(str)) {

		std::vector<CharsRange>::iterator r = charsRanges.begin();
		for(; r != charsRanges.end(); ++r) {
			if(r->start <= cp && cp <=r->end)
				break;
		}
		if(r == charsRanges.end())
			continue;

		int idx = r->charsIdx[cp - r->start];
		if(idx >= 0) {

			if(cp != 32 && bindTexture != textures[r->textureIdx]) {  // cp==32 is special case no bind texture
				if(bindTexture != 0)
					fd->flush();
				bindTexture = textures[r->textureIdx];
				glBindTexture(GL_TEXTURE_2D, bindTexture);
			}

			fd->addChar( chars[idx] );
		}
	}

	if(bindTexture != 0)
		fd->flush();
}

struct FontDrawer2D: public FontDrawer {

	struct	Vertex {
		vec2	v;
		vec2	tv;
		Vertex()																								{}
		Vertex(float x, float y, float tx, float ty, float size, const vec2 &offset): v(vec2(x,y)*size + offset), tv(tx, ty)		{}
	};

	std::vector<Vertex> buffer;
	std::vector<unsigned short> indexes;
	vec2 offset;
	float size;

	void start(float x, float y, float s)	{
		offset.x = x;
		offset.y = y;
		size = s;
	}

	virtual	void	addChar(const Font::FontChar &fc) {
		int idx = buffer.size();

		buffer.push_back( Vertex( fc.p.x,        fc.p.y,   	    fc.tp.x,         fc.tp.y+fc.ts.y, size, offset) );
		buffer.push_back( Vertex( fc.p.x,        fc.p.y+fc.s.y, fc.tp.x,         fc.tp.y        , size, offset) );
		buffer.push_back( Vertex( fc.p.x+fc.s.x, fc.p.y,   	    fc.tp.x+fc.ts.x, fc.tp.y+fc.ts.y, size, offset) );
		buffer.push_back( Vertex( fc.p.x+fc.s.x, fc.p.y+fc.s.y, fc.tp.x+fc.ts.x, fc.tp.y,         size, offset) );

		indexes.push_back(idx);
		indexes.push_back(idx+1);
		indexes.push_back(idx+2);
		indexes.push_back(idx+2);
		indexes.push_back(idx+1);
		indexes.push_back(idx+3);

		offset.x += fc.width*size;
	}

	virtual	void flush() {
		if(indexes.empty())
			return;

		glEnableVertexAttribArray(ATTRIB_POSITION);
		glEnableVertexAttribArray(ATTRIB_TEXCOORD);
		glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(Vertex), &buffer[0].v);
		glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, false, sizeof(Vertex), &buffer[0].tv);

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawElements(GL_TRIANGLES, indexes.size(), GL_UNSIGNED_SHORT, &indexes[0]);

		glDisableVertexAttribArray(ATTRIB_POSITION);
		glDisableVertexAttribArray(ATTRIB_TEXCOORD);

		buffer.clear();
		indexes.clear();
	}
};

void Font::draw(float x, float y, float size, const char *str) {
	static FontDrawer2D fd;
	fd.start(x, y, size);
	draw(&fd, str);
}

