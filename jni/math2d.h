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

#ifndef MATH3D_H
#define MATH3D_H

#include <math.h>
#include <algorithm>

#define EPSILON 1e-6f
#define F_INFINITY 1e+6f

inline bool is_NAN(float x) {
	return (*(unsigned int*)&x & 0x7f800000) == 0x7f800000;
}

inline  float absf(float v) { 
	unsigned int result = *(unsigned int*) &v;
	result &= 0x7fffffff;
    return *(float*)&result;	
}

inline  float signf(float v) {		// return 1.0 if v>=0, return -1 if v<0  
	unsigned int result = *(unsigned int*) &v;
	result = (result & 0x80000000) | 0x3f800000;
    return *(float*)&result;	
}

inline float fast_sqrtf(float x) {	// 5% calculation error
	unsigned int result = *(unsigned int*)&x;
	result = ( ((result - 0x3f800000) >> 1) + 0x3f800000 ) & 0x7fffffff;
	return *(float*)&result;
}

inline unsigned clp2(unsigned x) {	// round up to 2^n 
	x--;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return x + 1;
}

inline unsigned flp2(unsigned x) {	// round down to 2^n 
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return x - (x >> 1);
}

const	float PI=3.14159265359f;
const	float PI_2=3.14159265359f/2.0f;
const	float PI_4=3.14159265359f/4.0f;
inline  float deg2rad(float a)			{ return PI/180.0f*a; }
inline  float rad2deg(float a)			{ return 180.0f/PI*a; }

struct vec2;
struct mat3;
struct mat2;

// vec2                                                                      

struct vec2 {
	
	inline vec2()                                   {}
	inline vec2(const float x, const float y) : x(x), y(y)      {}
	inline vec2(const float *v): x(v[0]), y(v[1])   {}
	inline vec2(const float v): x(v), y(v)          {}
	inline vec2(const vec2 &v): x(v.x), y(v.y)      {}
	
	inline int operator==(const vec2 &v) { return (absf(x - v.x) < EPSILON && absf(y - v.y) < EPSILON); }
	inline int operator!=(const vec2 &v) { return !(*this == v); }
	
	inline const vec2 operator*(float f) const { return vec2(x * f,y * f); }
	inline const vec2 operator/(float f) const { return vec2(x / f,y / f); }
	inline const vec2 operator+(const vec2 &v) const { return vec2(x + v.x,y + v.y); }
	inline const vec2 operator-() const { return vec2(-x,-y); }
	inline const vec2 operator-(const vec2 &v) const { return vec2(x - v.x,y - v.y); }
	
	inline vec2 &operator*=(float f) { return *this = *this * f; }
	inline vec2 &operator/=(float f) { return *this = *this / f; }
	inline vec2 &operator+=(const vec2 &v) { return *this = *this + v; }
	inline vec2 &operator-=(const vec2 &v) { return *this = *this - v; }
	
	inline float operator*(const vec2 &v) const { return x * v.x + y * v.y; }
	
	inline operator float*() { return (float*)&x; }
	inline operator const float*() const { return (float*)&x; }
	
	inline float &operator[](int i) { return ((float*)&x)[i]; }
	inline const float operator[](int i) const { return ((float*)&x)[i]; }
	
	inline float length() const			{ return sqrtf(x * x + y * y); }
	inline float fast_length() const    { return fast_sqrtf(x * x + y * y); }
	inline float length2() const		{ return x * x + y * y; }

	inline float normalize() {
		float inv,length = sqrtf(x * x + y * y);
		inv = 1.0f / length;
		x *= inv;
		y *= inv;
		return length;
	}

	inline float fast_normalize() {
		float length = fast_sqrtf(x * x + y * y);
		float inv = 1.0f / length;
		x *= inv;
		y *= inv;
		return length;
	}

	inline float safe_normalize() {
		float inv,length = sqrtf(x * x + y * y);
		if(length < EPSILON*10) return length;  
		inv = 1.0f / length;
		x *= inv;
		y *= inv;
		return length;
	}
	
	union {
		struct {
			float x,y;
		};
		float v[2];
	};
};

//  mat3                                                                      

struct mat3 {
	mat3() {}
	
	mat3(const float v) {
		mat[0] = v; mat[3] = 0; mat[6] = 0;
		mat[1] = 0; mat[4] = v; mat[7] = 0;
		mat[2] = 0; mat[5] = 0; mat[8] = v;
	}
	mat3(const float *m) {
		mat[0] = m[0]; mat[3] = m[3]; mat[6] = m[6];
		mat[1] = m[1]; mat[4] = m[4]; mat[7] = m[7];
		mat[2] = m[2]; mat[5] = m[5]; mat[8] = m[8];
	}
	mat3(const mat3 &m) {
		mat[0] = m[0]; mat[3] = m[3]; mat[6] = m[6];
		mat[1] = m[1]; mat[4] = m[4]; mat[7] = m[7];
		mat[2] = m[2]; mat[5] = m[5]; mat[8] = m[8];
	}

	vec2 operator*(const vec2 &v) const {
		vec2 ret;
		ret[0] = mat[0] * v[0] + mat[3] * v[1] + mat[6];
		ret[1] = mat[1] * v[0] + mat[4] * v[1] + mat[7];
		return ret;
	}

	mat3 operator*(float f) const {
		mat3 ret;
		ret[0] = mat[0] * f; ret[3] = mat[3] * f; ret[6] = mat[6] * f;
		ret[1] = mat[1] * f; ret[4] = mat[4] * f; ret[7] = mat[7] * f;
		ret[2] = mat[2] * f; ret[5] = mat[5] * f; ret[8] = mat[8] * f;
		return ret;
	}

	mat3 operator*(const mat3 &m) const {
		mat3 ret;
		ret[0] = mat[0] * m[0] + mat[3] * m[1] + mat[6] * m[2];
		ret[1] = mat[1] * m[0] + mat[4] * m[1] + mat[7] * m[2];
		ret[2] = mat[2] * m[0] + mat[5] * m[1] + mat[8] * m[2];
		ret[3] = mat[0] * m[3] + mat[3] * m[4] + mat[6] * m[5];
		ret[4] = mat[1] * m[3] + mat[4] * m[4] + mat[7] * m[5];
		ret[5] = mat[2] * m[3] + mat[5] * m[4] + mat[8] * m[5];
		ret[6] = mat[0] * m[6] + mat[3] * m[7] + mat[6] * m[8];
		ret[7] = mat[1] * m[6] + mat[4] * m[7] + mat[7] * m[8];
		ret[8] = mat[2] * m[6] + mat[5] * m[7] + mat[8] * m[8];
		return ret;
	}

	mat3 operator+(const mat3 &m) const {
		mat3 ret;
		ret[0] = mat[0] + m[0]; ret[3] = mat[3] + m[3]; ret[6] = mat[6] + m[6];
		ret[1] = mat[1] + m[1]; ret[4] = mat[4] + m[4]; ret[7] = mat[7] + m[7];
		ret[2] = mat[2] + m[2]; ret[5] = mat[5] + m[5]; ret[8] = mat[8] + m[8];
		return ret;
	}

	mat3 operator-(const mat3 &m) const {
		mat3 ret;
		ret[0] = mat[0] - m[0]; ret[3] = mat[3] - m[3]; ret[6] = mat[6] - m[6];
		ret[1] = mat[1] - m[1]; ret[4] = mat[4] - m[4]; ret[7] = mat[7] - m[7];
		ret[2] = mat[2] - m[2]; ret[5] = mat[5] - m[5]; ret[8] = mat[8] - m[8];
		return ret;
	}
	
	mat3 &operator*=(float f) { return *this = *this * f; }
	mat3 &operator*=(const mat3 &m) { return *this = *this * m; }
	mat3 &operator+=(const mat3 &m) { return *this = *this + m; }
	mat3 &operator-=(const mat3 &m) { return *this = *this - m; }
	
	operator float*() { return mat; }
	operator const float*() const { return mat; }
	
	float &operator[](int i) { return mat[i]; }
	const float operator[](int i) const { return mat[i]; }
	
	mat3 transpose() const {
		mat3 ret;
		ret[0] = mat[0]; ret[3] = mat[1]; ret[6] = mat[2];
		ret[1] = mat[3]; ret[4] = mat[4]; ret[7] = mat[5];
		ret[2] = mat[6]; ret[5] = mat[7]; ret[8] = mat[8];
		return ret;
	}

	float det() const {
		float det;
		det = mat[0] * mat[4] * mat[8];
		det += mat[3] * mat[7] * mat[2];
		det += mat[6] * mat[1] * mat[5];
		det -= mat[6] * mat[4] * mat[2];
		det -= mat[3] * mat[1] * mat[8];
		det -= mat[0] * mat[7] * mat[5];
		return det;
	}

	mat3 inverse() const {
		mat3 ret;
		float idet = 1.0f / det();
		ret[0] =  (mat[4] * mat[8] - mat[7] * mat[5]) * idet;
		ret[1] = -(mat[1] * mat[8] - mat[7] * mat[2]) * idet;
		ret[2] =  (mat[1] * mat[5] - mat[4] * mat[2]) * idet;
		ret[3] = -(mat[3] * mat[8] - mat[6] * mat[5]) * idet;
		ret[4] =  (mat[0] * mat[8] - mat[6] * mat[2]) * idet;
		ret[5] = -(mat[0] * mat[5] - mat[3] * mat[2]) * idet;
		ret[6] =  (mat[3] * mat[7] - mat[6] * mat[4]) * idet;
		ret[7] = -(mat[0] * mat[7] - mat[6] * mat[1]) * idet;
		ret[8] =  (mat[0] * mat[4] - mat[3] * mat[1]) * idet;
		return ret;
	}
	
	void zero() {
		mat[0] = 0; mat[3] = 0; mat[6] = 0;
		mat[1] = 0; mat[4] = 0; mat[7] = 0;
		mat[2] = 0; mat[5] = 0; mat[8] = 0;
	}

	void identity() {
		mat[0] = 1; mat[3] = 0; mat[6] = 0;
		mat[1] = 0; mat[4] = 1; mat[7] = 0;
		mat[2] = 0; mat[5] = 0; mat[8] = 1;
	}

	void rotate(float angle) {
		float c = cosf(angle);
		float s = sinf(angle);
		mat[0] = c; mat[3] = -s; mat[6] = 0;
		mat[1] = s; mat[4] = c;  mat[7] = 0;
		mat[2] = 0; mat[5] = 0;  mat[8] = 1;
	}

	void scale(const vec2 &v) {
		mat[0] = v.x; mat[3] = 0;   mat[6] = 0;
		mat[1] = 0;   mat[4] = v.y; mat[7] = 0;
		mat[2] = 0;   mat[5] = 0;   mat[8] = 1;
	}

	void scale(float x,float y) {
		scale(vec2(x,y));
	}

	void translate(const vec2 &v) {
		mat[0] = 1; mat[3] = 0; mat[6] = v.x; 
		mat[1] = 0; mat[4] = 1; mat[7] = v.y; 
		mat[2] = 0; mat[5] = 0; mat[8] = 1; 
	}

	void translate(float x, float y) {
		translate(vec2(x,y));
	}

    static mat3  get_translate(const vec2 &v) {     
        mat3 ret;
        ret.translate(v);
        return ret;
	}

    static mat3  get_translate(float x, float y) {
        mat3 ret;
        ret.translate(vec2(x,y));
        return ret;
	}

	static mat3 get_rotate(float angle) {
        mat3 ret;
        ret.rotate(angle);
        return ret;
	}

	static mat3 get_scale(const vec2 &v) {
        mat3 ret;
		ret.scale(v);
        return ret;
	}

	static mat3 get_scale(float x, float y) {
        mat3 ret;
		ret.scale(vec2(x,y));
        return ret;
	}

    static mat3 get_identity()  { return mat3(1.0f); }
	
    union {
	    float mat[9];
        float m[3][3];
    };
};

//  mat3                                                                      

struct mat2 {
	mat2() {}
	
	mat2(const float v) {
		mat[0] = v; mat[1] = 0; 
		mat[2] = 0; mat[3] = v;   
	}

	mat2(const float *m) {
		mat[0] = m[0]; mat[1] = m[1]; mat[2] = m[2]; mat[3] = m[3];
	}

	mat2(const mat2 &m) {
		mat[0] = m[0]; 	mat[1] = m[1]; 
		mat[2] = m[2]; 	mat[3] = m[3]; 
	}

	vec2 operator*(const vec2 &v) const {
		vec2 ret;
		ret[0] = mat[0] * v[0] + mat[2] * v[1];
		ret[1] = mat[1] * v[0] + mat[3] * v[1];
		return ret;
	}

	mat2 operator*(float f) const {
		mat2 ret;
		ret[0] = mat[0] * f; ret[1] = mat[1] * f; 
		ret[2] = mat[2] * f; ret[3] = mat[3] * f; 
		return ret;
	}

	mat2 operator*(const mat2 &m) const {
		mat2 ret;
		ret[0] = mat[0] * m[0] + mat[2] * m[1];
		ret[1] = mat[1] * m[0] + mat[3] * m[1];
		ret[2] = mat[0] * m[2] + mat[2] * m[3];
		ret[3] = mat[1] * m[2] + mat[3] * m[3];
		return ret;
	}

	mat2 operator+(const mat2 &m) const {
		mat2 ret;
		ret[0] = mat[0] + m[0];	ret[1] = mat[1] + m[1]; 
		ret[2] = mat[2] + m[2];	ret[3] = mat[3] + m[3];
		return ret;
	}

	mat2 operator-(const mat2 &m) const {
		mat2 ret;
		ret[0] = mat[0] - m[0]; ret[1] = mat[1] - m[1];
		ret[2] = mat[2] - m[2];	ret[3] = mat[3] - m[3]; 
		return ret;
	}
	
	mat2 &operator*=(float f) { return *this = *this * f; }
	mat2 &operator*=(const mat2 &m) { return *this = *this * m; }
	mat2 &operator+=(const mat2 &m) { return *this = *this + m; }
	mat2 &operator-=(const mat2 &m) { return *this = *this - m; }
	
	operator float*() { return mat; }
	operator const float*() const { return mat; }
	
	float &operator[](int i) { return mat[i]; }
	const float operator[](int i) const { return mat[i]; }
	
	mat2 transpose() const {
		mat2 ret;
		ret[0] = mat[0];	
		ret[1] = mat[2]; 
		ret[2] = mat[1]; 
		ret[3] = mat[3];
		return ret;
	}

	float det() const {
		return mat[0] * mat[3] - mat[1] * mat[2];
	}

	mat2 inverse() const {
		mat2 ret;
		float idet = 1.0f / det();
		ret[0] =  mat[3] * idet;
		ret[1] = -mat[1] * idet;
		ret[2] = -mat[2] * idet;
		ret[3] =  mat[0] * idet;
		return ret;
	}
	
	void zero() {
		mat[0] = mat[1] = mat[2] = mat[3] = 0.0f;
	}

	void identity() {
		mat[0] = 1; mat[1] = 0; 
		mat[2] = 0; mat[3] = 1; 
	}

	void rotate(float angle) {
		float c = cosf(angle);
		float s = sinf(angle);
		mat[0] = c; mat[2] = -s;
		mat[1] = s; mat[3] = c; 
	}

	void scale(const vec2 &v) {
		mat[0] = v.x; mat[1] = 0; 
		mat[2] = 0;   mat[3] = v.y; 
	}

	void scale(float x,float y) {
		scale(vec2(x,y));
	}

	static mat2 get_rotate(float angle) {
        mat2 ret;
        ret.rotate(angle);
        return ret;
	}

	static mat2 get_scale(const vec2 &v) {
        mat2 ret;
		ret.scale(v);
        return ret;
	}

	static mat2 get_scale(float x, float y) {
        mat2 ret;
		ret.scale(vec2(x,y));
        return ret;
	}

    static mat2 get_identity()  { 
		return mat2(1.0f); 
	}
	
    union {
	    float mat[4];
        float m[2][2];
    };
};

struct mat4 {
	
	mat4() {}

    mat4(const float v) {
		mat[0] = v;   mat[4] = 0.0; mat[8] = 0.0;  mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = v;   mat[9] = 0.0;  mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = v;   mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = v;
	}

	mat4(const float *m) {
		mat[0] = m[0]; mat[4] = m[4]; mat[8] = m[8]; mat[12] = m[12];
		mat[1] = m[1]; mat[5] = m[5]; mat[9] = m[9]; mat[13] = m[13];
		mat[2] = m[2]; mat[6] = m[6]; mat[10] = m[10]; mat[14] = m[14];
		mat[3] = m[3]; mat[7] = m[7]; mat[11] = m[11]; mat[15] = m[15];
	}

	mat4(const mat4 &m) {
		mat[0] = m[0]; mat[4] = m[4]; mat[8] = m[8]; mat[12] = m[12];
		mat[1] = m[1]; mat[5] = m[5]; mat[9] = m[9]; mat[13] = m[13];
		mat[2] = m[2]; mat[6] = m[6]; mat[10] = m[10]; mat[14] = m[14];
		mat[3] = m[3]; mat[7] = m[7]; mat[11] = m[11]; mat[15] = m[15];
	}
	
    mat4(float m0, float m1, float m2,  float m3,
         float m4, float m5, float m6,  float m7,
         float m8, float m9, float m10, float m11,
         float m12,float m13,float m14, float m15) {
        mat[0]=m0;   mat[1]=m1;   mat[2]=m2;   mat[3]=m3;
        mat[4]=m4;   mat[5]=m5;   mat[6]=m6;   mat[7]=m7;
        mat[8]=m8;   mat[9]=m9;   mat[10]=m10; mat[11]=m11;
        mat[12]=m12; mat[13]=m13; mat[14]=m14; mat[15]=m15;
    }   

	mat4 operator*(float f) const {
		mat4 ret;
		ret[0] = mat[0] * f; ret[4] = mat[4] * f; ret[8] = mat[8] * f; ret[12] = mat[12] * f;
		ret[1] = mat[1] * f; ret[5] = mat[5] * f; ret[9] = mat[9] * f; ret[13] = mat[13] * f;
		ret[2] = mat[2] * f; ret[6] = mat[6] * f; ret[10] = mat[10] * f; ret[14] = mat[14] * f;
		ret[3] = mat[3] * f; ret[7] = mat[7] * f; ret[11] = mat[11] * f; ret[15] = mat[15] * f;
		return ret;
	}

	mat4 operator*(const mat4 &m) const {
		mat4 ret;
		ret[0] = mat[0] * m[0] + mat[4] * m[1] + mat[8] * m[2] + mat[12] * m[3];
		ret[1] = mat[1] * m[0] + mat[5] * m[1] + mat[9] * m[2] + mat[13] * m[3];
		ret[2] = mat[2] * m[0] + mat[6] * m[1] + mat[10] * m[2] + mat[14] * m[3];
		ret[3] = mat[3] * m[0] + mat[7] * m[1] + mat[11] * m[2] + mat[15] * m[3];
		ret[4] = mat[0] * m[4] + mat[4] * m[5] + mat[8] * m[6] + mat[12] * m[7];
		ret[5] = mat[1] * m[4] + mat[5] * m[5] + mat[9] * m[6] + mat[13] * m[7];
		ret[6] = mat[2] * m[4] + mat[6] * m[5] + mat[10] * m[6] + mat[14] * m[7];
		ret[7] = mat[3] * m[4] + mat[7] * m[5] + mat[11] * m[6] + mat[15] * m[7];
		ret[8] = mat[0] * m[8] + mat[4] * m[9] + mat[8] * m[10] + mat[12] * m[11];
		ret[9] = mat[1] * m[8] + mat[5] * m[9] + mat[9] * m[10] + mat[13] * m[11];
		ret[10] = mat[2] * m[8] + mat[6] * m[9] + mat[10] * m[10] + mat[14] * m[11];
		ret[11] = mat[3] * m[8] + mat[7] * m[9] + mat[11] * m[10] + mat[15] * m[11];
		ret[12] = mat[0] * m[12] + mat[4] * m[13] + mat[8] * m[14] + mat[12] * m[15];
		ret[13] = mat[1] * m[12] + mat[5] * m[13] + mat[9] * m[14] + mat[13] * m[15];
		ret[14] = mat[2] * m[12] + mat[6] * m[13] + mat[10] * m[14] + mat[14] * m[15];
		ret[15] = mat[3] * m[12] + mat[7] * m[13] + mat[11] * m[14] + mat[15] * m[15];
		return ret;
	}

	mat4 operator+(const mat4 &m) const {
		mat4 ret;
		ret[0] = mat[0] + m[0]; ret[4] = mat[4] + m[4]; ret[8] = mat[8] + m[8]; ret[12] = mat[12] + m[12];
		ret[1] = mat[1] + m[1]; ret[5] = mat[5] + m[5]; ret[9] = mat[9] + m[9]; ret[13] = mat[13] + m[13];
		ret[2] = mat[2] + m[2]; ret[6] = mat[6] + m[6]; ret[10] = mat[10] + m[10]; ret[14] = mat[14] + m[14];
		ret[3] = mat[3] + m[3]; ret[7] = mat[7] + m[7]; ret[11] = mat[11] + m[11]; ret[15] = mat[15] + m[15];
		return ret;
	}

	mat4 operator-(const mat4 &m) const {
		mat4 ret;
		ret[0] = mat[0] - m[0]; ret[4] = mat[4] - m[4]; ret[8] = mat[8] - m[8]; ret[12] = mat[12] - m[12];
		ret[1] = mat[1] - m[1]; ret[5] = mat[5] - m[5]; ret[9] = mat[9] - m[9]; ret[13] = mat[13] - m[13];
		ret[2] = mat[2] - m[2]; ret[6] = mat[6] - m[6]; ret[10] = mat[10] - m[10]; ret[14] = mat[14] - m[14];
		ret[3] = mat[3] - m[3]; ret[7] = mat[7] - m[7]; ret[11] = mat[11] - m[11]; ret[15] = mat[15] - m[15];
		return ret;
	}
	
	mat4 &operator*=(float f) { return *this = *this * f; }
	mat4 &operator*=(const mat4 &m) { return *this = *this * m; }
	mat4 &operator+=(const mat4 &m) { return *this = *this + m; }
	mat4 &operator-=(const mat4 &m) { return *this = *this - m; }
	
	operator float*() { return mat; }
	operator const float*() const { return mat; }
	
	float &operator[](int i) { return mat[i]; }
	float operator[](int i) const { return mat[i]; }
	
	mat4 rotation() const {
		mat4 ret;
		ret[0] = mat[0]; ret[4] = mat[4]; ret[8] = mat[8]; ret[12] = 0;
		ret[1] = mat[1]; ret[5] = mat[5]; ret[9] = mat[9]; ret[13] = 0;
		ret[2] = mat[2]; ret[6] = mat[6]; ret[10] = mat[10]; ret[14] = 0;
		ret[3] = 0; ret[7] = 0; ret[11] = 0; ret[15] = 1;
		return ret;
	}

	mat4 transpose() const {
		mat4 ret;
		ret[0] = mat[0]; ret[4] = mat[1]; ret[8] = mat[2]; ret[12] = mat[3];
		ret[1] = mat[4]; ret[5] = mat[5]; ret[9] = mat[6]; ret[13] = mat[7];
		ret[2] = mat[8]; ret[6] = mat[9]; ret[10] = mat[10]; ret[14] = mat[11];
		ret[3] = mat[12]; ret[7] = mat[13]; ret[11] = mat[14]; ret[15] = mat[15];
		return ret;
	}

	mat4 transpose_rotation() const {
		mat4 ret;
		ret[0] = mat[0]; ret[4] = mat[1]; ret[8] = mat[2]; ret[12] = mat[12];
		ret[1] = mat[4]; ret[5] = mat[5]; ret[9] = mat[6]; ret[13] = mat[13];
		ret[2] = mat[8]; ret[6] = mat[9]; ret[10] = mat[10]; ret[14] = mat[14];
		ret[3] = mat[3]; ret[7] = mat[7]; ret[14] = mat[14]; ret[15] = mat[15];
		return ret;
	}
	
	float det() const {
		float det;
		det = mat[0] * mat[5] * mat[10];
		det += mat[4] * mat[9] * mat[2];
		det += mat[8] * mat[1] * mat[6];
		det -= mat[8] * mat[5] * mat[2];
		det -= mat[4] * mat[1] * mat[10];
		det -= mat[0] * mat[9] * mat[6];
		return det;
	}
	
	mat4 inverse() const {
		mat4 ret;
		float idet = 1.0f / det();
		ret[0] =  (mat[5] * mat[10] - mat[9] * mat[6]) * idet;
		ret[1] = -(mat[1] * mat[10] - mat[9] * mat[2]) * idet;
		ret[2] =  (mat[1] * mat[6] - mat[5] * mat[2]) * idet;
		ret[3] = 0.0;
		ret[4] = -(mat[4] * mat[10] - mat[8] * mat[6]) * idet;
		ret[5] =  (mat[0] * mat[10] - mat[8] * mat[2]) * idet;
		ret[6] = -(mat[0] * mat[6] - mat[4] * mat[2]) * idet;
		ret[7] = 0.0;
		ret[8] =  (mat[4] * mat[9] - mat[8] * mat[5]) * idet;
		ret[9] = -(mat[0] * mat[9] - mat[8] * mat[1]) * idet;
		ret[10] =  (mat[0] * mat[5] - mat[4] * mat[1]) * idet;
		ret[11] = 0.0;
		ret[12] = -(mat[12] * ret[0] + mat[13] * ret[4] + mat[14] * ret[8]);
		ret[13] = -(mat[12] * ret[1] + mat[13] * ret[5] + mat[14] * ret[9]);
		ret[14] = -(mat[12] * ret[2] + mat[13] * ret[6] + mat[14] * ret[10]);
		ret[15] = 1.0;
		return ret;
	}
	
	void zero() {
		mat[0] = 0.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 0.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 0.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 0.0;
	}
	void identity() {
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void rotate_x(float angle) {
		float rad = deg2rad(angle);
		float c = cosf(rad);
		float s = sinf(rad);
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = c; mat[9] = -s; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = s; mat[10] = c; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void rotate_y(float angle) {
		float rad = deg2rad(angle);
		float c = cosf(rad);
		float s = sinf(rad);
		mat[0] = c; mat[4] = 0.0; mat[8] = s; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = -s; mat[6] = 0.0; mat[10] = c; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void rotate_z(float angle) {
		float rad = deg2rad(angle);
		float c = cosf(rad);
		float s = sinf(rad);
		mat[0] = c; mat[4] = -s; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = s; mat[5] = c; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void scale(float x, float y, float z) {
		mat[0] = x; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = y; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = z; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void translate(float x, float y, float z) {
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = x;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = y;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = z;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}
	
	void perspective(float fov,float aspect,float znear,float zfar) {
		float y = tanf(fov * PI / 360.0f);
		float x = y * aspect;
		mat[0] = 1.0f / x; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0f / y; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = -(zfar + znear) / (zfar - znear); mat[14] = -(2.0f * zfar * znear) / (zfar - znear);
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = -1.0; mat[15] = 0.0;
	}

	void ortho(float left, float right, float bottom, float top, float near=-1, float far=1) {
		float rl = right - left;
		float tb = top - bottom;
		float fn = far - near;
		mat[0] = 2.0f / rl;
		mat[5] = 2.0f / tb;
		mat[10] = - 2.0f / fn;
		mat[3] = - (right + left) / rl;
		mat[7] = - (top + bottom) / tb;
		mat[11] = - (far + near) / fn;
		mat[1] = mat[2] = mat[4] = mat[6] = mat[8] = mat[9] = mat[12] = mat[13] = mat[14] = 0;
		mat[15] = 1;
	}

	static mat4 get_ortho(float left, float right, float bottom, float top, float near=-1, float far=1) {
        mat4 ret;
        ret.ortho(left, right, bottom, top, near, far);
        return ret;
	}

    static mat4  get_translate(float x,float y,float z) {
        mat4 ret;
        ret.translate(x, y, z);
        return ret;
	}

	static mat4 get_rotate_x(float angle) {
        mat4 ret;
        ret.rotate_x(angle);
        return ret;
	}

	static mat4 get_rotate_y(float angle) {
        mat4 ret;
        ret.rotate_y(angle);
        return ret;
	}

	static mat4 get_rotate_z(float angle) {
        mat4 ret;
        ret.rotate_z(angle);
        return ret;
	}

	static mat4 get_scale(float x, float y, float z) {
        mat4 ret;
		ret.scale(x, y, z);
        return ret;
	}

    static mat4 get_identity()  { return mat4(1.0f); }

    union {
	    float mat[16];
        float m[4][4];
//        struct {	
//            vec3 rot_x;
//			float	m03;
//			vec3 rot_y;
//			float	m13;
//			vec3 rot_z;
//			float	m23;
//			union {	
//              struct	{	vec3 pos;	 };
//				struct	{	float x,y,z; };
//			};
//			float	m33;
//		};
    };
};


inline vec2 reflex(const vec2 &vel, const vec2 &normal) {	// vel must be normalized!
	return  normal*(2.0f*(-vel*normal)) + vel;
}

float atan2(float y, float x); //Principal arc tangent of y/x, in the interval [-pi,+pi] radians.

inline bool linesIntersection(const vec2 &tp1,const vec2 &tp2,const vec2 &sc1,const vec2 &sc2, vec2 &result) {
	float z  = (tp2.y-tp1.y)*(sc1.x-sc2.x)-(sc1.y-sc2.y)*(tp2.x-tp1.x);
	if(absf(z) < EPSILON)
		return false;
	float a = ( (tp2.y-tp1.y)*(sc1.x-tp1.x)-(sc1.y-tp1.y)*(tp2.x-tp1.x) ) / z;
	float b = ( (sc1.y-tp1.y)*(sc1.x-sc2.x)-(sc1.y-sc2.y)*(sc1.x-tp1.x) ) / z;

	if( (0 <= a) && (a <= 1) && (0 <= b) && (b <= 1) ) {
		result = tp1 + (tp2 - tp1) * b;
		return true;
	}
	return false;
}

struct rect {
	vec2	lb, rt;
	rect(): lb(F_INFINITY, F_INFINITY), rt(-F_INFINITY, -F_INFINITY)	{}
	rect(const vec2 &alb, const vec2 & art): lb(alb), rt(art)			{}
	void	add(const vec2& p)						{	lb.x = std::min(p.x, lb.x); lb.y = std::min(p.y, lb.y);	rt.x = std::max(p.x, rt.x); rt.y = std::max(p.y, rt.y); }
	bool	inside(const vec2& p) const				{	return p.x >= lb.x && p.x <= rt.x && p.y >= lb.y && p.y <= rt.y; }
	bool	intersect(const vec2& p, float r) const {	return p.x >= lb.x - r && p.x <= rt.x + r && p.y >= lb.y - r && p.y <= rt.y + r; }
	bool	intersect(const rect& r) const			{	return r.rt.x >= lb.x && r.lb.x <= rt.x && r.rt.y >= lb.y && r.lb.y <= rt.y; }
};

#endif 

