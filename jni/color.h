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

#ifndef COLOR_H
#define COLOR_H

#include "math2d.h"

struct color4 {
	inline color4()																			{}
	inline color4(float ar, float ag, float ab, float aa=1.0f) : r(ar), g(ag), b(ab), a(aa)	{}
	inline color4(const float *v) : r(v[0]), g(v[1]), b(v[2]), a(v[3])						{}
	inline color4(const color4 &v) : r(v.r), g(v.g), b(v.b), a(v.a)							{}

	inline int operator==(const color4 &v) { return (absf(r - v.r) < EPSILON && absf(g - v.g) < EPSILON && absf(b - v.b) < EPSILON && absf(a - v.a) < EPSILON); }
	inline int operator!=(const color4 &v) { return !(*this == v); }

	inline const color4 operator*(float f) const { return color4(r * f, g * f, b * f, a * f); }
	inline const color4 operator/(float f) const { return color4(r / f, g / f, b / f, a / f); }
	inline const color4 operator+(const color4 &v) const { return color4(r + v.r, g + v.g, b + v.b, a + v.a); }
	inline const color4 operator-() const { return color4(-r, -g, -b, -a); }
	inline const color4 operator-(const color4 &v) const { return color4(r - v.r, g - v.g, b - v.b, a - v.a); }

	inline color4 &operator*=(float f) { return *this = *this * f; }
	inline color4 &operator/=(float f) { return *this = *this / f; }
	inline color4 &operator+=(const color4 &v) { return *this = *this + v; }
	inline color4 &operator-=(const color4 &v) { return *this = *this - v; }

	inline float operator*(const color4 &v) const { return r * v.r + g * v.g + b * v.b + a * v.a; }

	inline operator float*() { return v; }
	inline operator const float*() const { return v; }

	inline float &operator[](int i) { return v[i]; }
	inline const float operator[](int i) const { return v[i]; }

	union {
		struct {
			float r, g, b, a;
		};
		float v[4];
	};
};


#endif
