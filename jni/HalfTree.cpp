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

#include "HalfTree.h"
#include "opengl.h"
#include "rand.h"
#include "color.h"
#include "Texture.h"
#include "Render.h"

HalfTree::Node::~Node() {
	if(parent) {
		if(parent->left==this) 
			parent->left=0;
		else
			parent->right=0;
	}
}

HalfTree::HalfTree(const vec2 &p, const vec2 &d, float l, float lFactor, float lFactorDiv, float aFactor, float aFactorDiv, Deformer *def): 
				bounds(p, p), deformer(def), root(0), current(0), count(0), deep(0), length(0), iterator(0), maxIterator(0), 
				invStatus(IS_ALL),
				lengthFactor(lFactor), lengthFactorDiv(lFactorDiv), angleFactor(aFactor), angleFactorDiv(aFactorDiv) 
{
	root = new Node(p, d, l, 0);
}

inline HalfTree::Node *HalfTree::makeNode(const vec2 &pos, const vec2 &dir, float length, Node *parent) {
	if(deformer) {
		vec2 p = pos + dir * length;
		if(deformer->deform(p)) {
			p -= pos;
			length = p.fast_normalize();
			return new Node(pos, p, length, parent);
		}
	}
	return new Node(pos, dir, length, parent);
}

HalfTree::Node *HalfTree::makeLeft(HalfTree::Node *n) {
	n->left = makeNode(n->end(), mat2::get_rotate( angleFactor * randf(1, angleFactorDiv)) * n->dir, n->length * lengthFactor * randf(1, lengthFactorDiv), n);
	bounds.add( n->left->end() );
	return n->left;
}

HalfTree::Node *HalfTree::makeRight(HalfTree::Node *n) {
	n->right = makeNode(n->end(), mat2::get_rotate(-angleFactor * randf(1, angleFactorDiv)) * n->dir, n->length * lengthFactor * randf(1, lengthFactorDiv), n);
	bounds.add( n->right->end() );
	return n->right;
}

void HalfTree::addBounds(HalfTree::Node *n) {
	bounds.add(n->end());
	if(n->left)
		addBounds(n->left);
	if(n->right)
		addBounds(n->right);
}

void HalfTree::recalcBounds() {
	bounds = rect(root->pos, root->pos);
	addBounds(root);
}

float HalfTree::getLength() {
	return current ? length + current->curLength : length;
}

void HalfTree::invalidate(InvalidateStatus invSt) {
	invStatus = std::max(invStatus, invSt);
}

void HalfTree::stepUp(float v) {
	if(!current) {
		leftBranch();
		count++;
		invalidate(IS_ALL);
	}

	current->curLength += v;
	if(current->curLength >= current->length) {
		current->curLength = current->length;
		length += current->length;
		nextBranch();
		count++;
		invalidate(IS_ALL);
	} else
		invalidate(IS_LAST_POINT);
}

void HalfTree::stepDown(float v) {
	if(count==0) 
		return;

	current->curLength -= v;
	if(current->curLength <= 0) {
		count--;
		if(count==0) {
			current = 0;
			length = 0;
			return;
		}
		delete current;
		recalcBounds();
		prevBranch();
		length -= current->length;
		invalidate(IS_ALL);
	} else
		invalidate(IS_LAST_POINT);

}

void HalfTree::leftBranch() {
	iterator = 0;
	current = root;
	int deep = 0;
	while(current->left) {
		deep++;
		current = current->left;
	}
	if(current->curLength >= current->length) {
		deep++;
		current = makeLeft(current);
	}
	maxIterator = (1 << deep);
}

void HalfTree::nextBranch() {
	iterator++;
	if(iterator >= maxIterator) {
		maxIterator <<= 1;
		deep++;
		iterator = 0;
	}
	current = root;
	int it = iterator;
	for(int fit = maxIterator; !(fit & 1); fit >>= 1) {
		if(it & 1) {
			if(current->right)
				current = current->right;
			else 
				current = makeRight(current);
		} else {
			if(current->left)
				current = current->left;
			else 
				current = makeLeft(current);
		}
		it >>= 1;
	}
}

void HalfTree::prevBranch() {
	if(iterator > 0) 
		iterator--;
	else {
		maxIterator >>= 1; 
		deep--;
		if(maxIterator <= 0) {
			iterator = 0;
			current = 0;
			deep=0;
			return;
		}
		iterator = maxIterator - 1;
	} 
	current = root;
	int it = iterator;
	for(int fit = maxIterator; !(fit & 1); fit >>= 1) {
		if(it & 1) 
			current = current->right;
		else 
			current = current->left;
		it >>= 1;
	}
}

#if 0
void Plant::drawNode(Plant::Node *n, const color4 &c, bool drawTerm) {
	glVertex2fv( n->pos );
	vec2 finish = n->pos + n->dir * n->curLength;
	glVertex2fv( finish );
	if(n->left)
		drawNode(n->left, c, drawTerm);
	else if(drawTerm) {
		glEnd();
		glColor3f(1, 0, 0);
		glBegin(GL_POINTS);
		glVertex2fv( finish );
		glEnd();
		glColor4fv(c);
		glBegin(GL_LINES);
	} 
	if(n->right)
		drawNode(n->right, c, drawTerm);
}

void Plant::drawDraft(const color4 &c, bool drawTerm) {

	if(!root)
		return;

	glColor4fv(c);
	glBegin(GL_LINES);
	drawNode(root, c, drawTerm);
	glEnd();
}
#endif

void HalfTree::buildDrawBuffer() {
	bool setAllData = invStatus == IS_REBUILD;

	invStatus = IS_VALID;
	vertCount = 0;
	if(!root)
		return;

	float thickness = (deep + 1 + (float(iterator)/maxIterator) ) * 0.003f;
	vec2 finish = root->pos + root->dir * root->curLength;	// root->left->pos
	vec2 n = vec2(-root->dir.y, root->dir.x);

	vec2 vn = n;// * (finish - root->pos).fast_length();

	n *= thickness;

	size_t newVertSize = count * 3 + 1;
	if(verts.size() < newVertSize) { 
		verts.resize( std::max( verts.empty() ? 250 : verts.size()+verts.size()/2, newVertSize) );
		setAllData = true;
	}

	verts[0] = TreeVert(root->pos-n, vn, 0);
	verts[1] = TreeVert(root->pos+n, vn, 0);
	verts[2] = TreeVert(finish-n, vn, 1);
	verts[3] = TreeVert(finish+n, vn, 1);
	vertCount = 4;

	if(maxIterator<2) {
		vertSample = TreeVert(n, vn, 1);
	} else {
		float level = 1;
		int mit = 2;
		for(int i = 0; (i <= iterator && mit == maxIterator) || mit < maxIterator;) {
			current = root;
			int it = i;
			bool r;
			int idx = 0;
			for(int fit = mit; !(fit & 1); fit >>= 1) {
				r = it & 1;
				if(r) 
					current = current->right;
				else
					current = current->left;
				it >>= 1;
				idx += fit >> 2;
			}

			int mask = (mit >> 1) - 1;
			idx = ((i & mask)+1+idx)*3-1;	// index of parent
			if(r) 
				buildDrawBufferRight(current, idx, level, &verts[0]);
			else
				buildDrawBufferLeft(current, idx, level, &verts[0]);

			i++;
			if(i >= mit) {
				mit <<= 1;
				i = 0;
				level++;
			}
		}

	}
	if(setAllData) 
		vertsVBO.setData(verts.size() * sizeof(TreeVert), GL_DYNAMIC_DRAW, &verts[0]);
	else
		vertsVBO.setSubData(0, vertCount * sizeof(TreeVert), &verts[0]);
}
			
void HalfTree::updateDrawBufferLastPoint() {
	if(!current) {
		invStatus = IS_ALL;
		return;
	}
	vec2 finish = current->pos + current->dir * current->curLength;	
	TreeVert verts[2] = {	TreeVert(finish - vertSample.v, vertSample.n, vertSample.lev),
							TreeVert(finish + vertSample.v, vertSample.n, vertSample.lev)	};  
	vertsVBO.setSubData((vertCount-2)*sizeof(TreeVert), sizeof(verts), verts);
	invStatus = IS_VALID;
}

void HalfTree::buildDrawBufferLeft(Node *nd, int idx, float level, TreeVert* verts) {
	vec2 p = nd->pos + nd->dir * (nd->dir * (verts[idx+1].v - nd->pos));
	vec2 n = (verts[idx+1].v - p) * 0.75f;
	vec2 pn = p - n;

	vec2 finish = nd->pos + nd->dir * nd->curLength;	// root->left->pos
	vec2 vn = n;// * (finish - nd->pos).fast_length();
	vn.fast_normalize();

	verts[vertCount] = TreeVert(pn, verts[idx+1].n, level);	vertCount++;
	verts[vertCount] = TreeVert(finish - n, vn, level + 1);	vertCount++;
	verts[vertCount] = TreeVert(finish + n, vn, level + 1);	vertCount++;
	vertSample = TreeVert(n, vn, level + 1);
}

void HalfTree::buildDrawBufferRight(Node *nd, int idx, float level, TreeVert* verts) {
	vec2 p = nd->pos + nd->dir * (nd->dir * (verts[idx].v - nd->pos));
	vec2 n = p - verts[idx].v;
	vec2 pn = p + n;

	vec2 finish = nd->pos + nd->dir * nd->curLength;	// root->left->pos
	vec2 vn = n;// * (finish - nd->pos).fast_length();
	vn.fast_normalize();

	verts[vertCount] = TreeVert(pn, verts[idx].n, level);	vertCount++;
	verts[vertCount] = TreeVert(finish - n, vn, level + 1);	vertCount++;
	verts[vertCount] = TreeVert(finish + n, vn, level + 1);	vertCount++;
	vertSample = TreeVert(n, vn, level + 1);
}

void HalfTree::drawBounds(Render *render) {
	render->drawRect(bounds, color4(1,0,0,1));
}

///////////////////////////// Index VBO builder

struct PlantIndexVBOBuilder {
	int	tsize;
	unsigned short *idxs, *idxs2;
	static int indexOffset;

	PlantIndexVBOBuilder(int vertsCount, VBO *vbo): tsize(4), idxs(0), idxs2(0) {
		indexOffset = vertsCount*3;
		int idxSize2 = (vertsCount/3+1)*2;

		std::vector<unsigned short> vidxs(indexOffset + idxSize2);
		idxs = &vidxs[0];
		idxs2 = idxs + indexOffset;

		push_back(idxs, 0);
		push_back(idxs, 1);
		push_back(idxs, 2);
		push_back(idxs, 2);
		push_back(idxs, 1);
		push_back(idxs, 3);

		push_back(idxs2, 0);
		push_back(idxs2, 2);

		int mit = 2, i = 0;
		do {
			int it = i;
			bool r;
			int idx = 0;
			for(int fit = mit; !(fit & 1); fit >>= 1) {
				r = it & 1;
				it >>= 1;
				idx += fit >> 2;
			}

			int mask = (mit >> 1) - 1;
			idx = ((i & mask)+1+idx)*3-1;	// index of parent
			if(r) 
				buildDrawBufferIndexRight(idx);
			else
				buildDrawBufferIndexLeft(idx);

			i++;
			if(i >= mit) {
				mit <<= 1;
				i = 0;
			}

		} while( *(idxs-1) < vertsCount );

		vbo->setData( vidxs.size() * sizeof(unsigned short), GL_STATIC_DRAW, &vidxs[0] );
	}

	inline void push_back(unsigned short* &indx, int value) {
		*indx = (unsigned short)value;
		++indx;
	}

	void buildDrawBufferIndexLeft(int idx) {
		int newIdx = tsize;
		tsize += 3;

		push_back(idxs, idx);
		push_back(idxs, idx+1);
		push_back(idxs, newIdx);
	
		push_back(idxs, newIdx);
		push_back(idxs, idx+1);
		push_back(idxs, newIdx+1);
		push_back(idxs, newIdx+1);
		push_back(idxs, idx+1);
		push_back(idxs, newIdx+2);

		push_back(idxs2, idx+1);
		push_back(idxs2, newIdx+1);
	}

	void buildDrawBufferIndexRight(int idx) {
		int newIdx = tsize;
		tsize += 3;

		push_back(idxs, idx);
		push_back(idxs, idx+1);
		push_back(idxs, newIdx);
	
		push_back(idxs, idx);
		push_back(idxs, newIdx);
		push_back(idxs, newIdx+1);
		push_back(idxs, newIdx+1);
		push_back(idxs, newIdx);
		push_back(idxs, newIdx+2);

		push_back(idxs2, idx);
		push_back(idxs2, newIdx+1);
	}
};

int PlantIndexVBOBuilder::indexOffset = 0;

void HalfTree::buildVBOIndex(VBOIndex &index) {
	index.bind();
	PlantIndexVBOBuilder build(32000, &index);
	index.unbind();
}
////////////////

void HalfTree::draw(Render *render) {
	if(count==0)
		return;
	if(!render->getBounds().intersect(bounds))
		return;

	vertsVBO.bind();

	switch(invStatus) {
		case IS_LAST_POINT:
			updateDrawBufferLastPoint();
			break;
		case IS_ALL:
		case IS_REBUILD:
			buildDrawBuffer();
			break;
		default:
			break;
	}

	render->bindPlantVBOIndex();

	glEnableVertexAttribArray(ATTRIB_POSITION);
	glEnableVertexAttribArray(ATTRIB_NORMAL);
	glEnableVertexAttribArray(ATTRIB_LEVEL);
	glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, sizeof(TreeVert), ((TreeVert*)0)->v);
	glVertexAttribPointer(ATTRIB_NORMAL, 2, GL_FLOAT, false, sizeof(TreeVert), ((TreeVert*)0)->n);
	glVertexAttribPointer(ATTRIB_LEVEL, 1, GL_FLOAT, false, sizeof(TreeVert), &((TreeVert*)0)->lev);

	int cnt = count*9 - 3;
	glDrawElements(GL_TRIANGLES, cnt, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINES, count*2, GL_UNSIGNED_SHORT, ((unsigned short*)0)+PlantIndexVBOBuilder::indexOffset);

	vertsVBO.unbind();
	VBO::unbindIndex();

	glDisableVertexAttribArray(ATTRIB_LEVEL);
	glDisableVertexAttribArray(ATTRIB_NORMAL);
	glDisableVertexAttribArray(ATTRIB_POSITION);
}

