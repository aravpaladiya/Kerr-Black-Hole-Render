#pragma once
#ifndef QUATERNION_H
#define QUATERNION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>

namespace constants {
	const float pi = 3.1415926536f;
}

class Quaternion;
class Vec3;
class Vec4;
class Mat4;
class Vec4 {
public:
	float x;
	float y;
	float z;
	float w;

	float getx();
	float gety();
	float getz();
	float getw();
	Mat4 generateTranslate();

	Vec4();

	Vec4(float xx, float yy, float zz, float ww);
};

class Vec3 {
public:
	float x;
	float y;
	float z;
	float getx();
	float gety();
	float getz();

	Vec3(float xx, float yy, float zz);
	Mat4 generateTranslate();

	Vec3();
};

class Mat4 {
public:
	float elements[4][4];

	Mat4(float m[4][4]);
	Mat4(std::initializer_list<std::initializer_list<float>> list);
	Mat4();
	static const Mat4 identity;
	std::vector<float> to1DArray();
	Mat4 generateProjection();

	Mat4 operator+(const Mat4 m) const;
	Mat4 operator-(const Mat4 m) const;
	Mat4 operator*(const Mat4 m) const;
	Vec4 operator*(Vec4 v) const;
};

class Quaternion {
public:
	float w;
	float x;
	float y;
	float z;

	Quaternion(float ww, float xx, float yy, float zz);

	static Quaternion generateFromAxis(float theta, Vec3 v);
	static Quaternion generateFromAxis(float theta, float x, float y, float z);

	Quaternion conjugate();

	Mat4 toMatrix();
};

#endif
