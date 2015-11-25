#include <math.h>
#include "hVec3.h"

using namespace hvl;
hVec3::hVec3(){
	X = 0;
	Y = 0;
	Z = 0;
}
hVec3::hVec3(float num){
	X = num;
	Y = num;
	Z = num;
}
hVec3::hVec3(float x, float y, float z){
	X = x;
	Y = y;
	Z = z;
}

float hVec3::Length(){
	return sqrt(pow(X, 2) + pow(Y, 2) + pow(Z, 2));
}
void hVec3::Normalize(){
	float n = Length();
	if (n != 0){
		X /= n;
		Y /= n;
		Z /= n;
	}
}
hVec3 hVec3::Normalized(){
	float n = Length();
	if (n == 0)
		return hVec3();
	return hVec3(X / n, Y / n, Z / n);
}

float hVec3::Distance(hVec3 a, hVec3 b){
	return sqrt(pow(a.X - b.X, 2) + pow(a.Y - b.Y, 2) + pow(a.Z - b.Z, 2));
}
hVec3 hVec3::Lerp(hVec3 a, hVec3 b, float t){
	return a + (b - a) * t;
}
hVec3 hVec3::Cross(hVec3 a, hVec3 b){
	float x, y, z;
	x = a.Y * b.Z - b.Y * a.Z;
	y = (a.X * b.Z - b.X * a.Z) * -1;
	z = a.X * b.Y - b.X * a.Y;
	return hVec3(x, y, z).Normalized();
}

hVec3 hVec3::operator+(hVec3 b){
	return hVec3((*this).X + b.X, (*this).Y + b.Y, (*this).Z + b.Z);
}
hVec3 hVec3::operator-(hVec3 b){
	return hVec3((*this).X - b.X, (*this).Y - b.Y, (*this).Z - b.Z);
}
hVec3 hVec3::operator*(hVec3 b){
	return hVec3((*this).X * b.X, (*this).Y * b.Y, (*this).Z * b.Z);
}
hVec3 hVec3::operator/(hVec3 b){
	return hVec3((*this).X / b.X, (*this).Y / b.Y, (*this).Z / b.Z);
}
hVec3 hVec3::operator*(float b){
	return hVec3((*this).X * b, (*this).Y * b, (*this).Z * b);
}
hVec3 hVec3::operator/(float b){
	return hVec3((*this).X / b, (*this).Y / b, (*this).Z / b);
}

void hVec3::operator+=(hVec3 b){
	(*this).X += b.X;
	(*this).Y += b.Y;
	(*this).Z += b.Z;
}
void hVec3::operator-=(hVec3 b){
	(*this).X -= b.X;
	(*this).Y -= b.Y;
	(*this).Z -= b.Z;
}
void hVec3::operator*=(hVec3 b){
	(*this).X *= b.X;
	(*this).Y *= b.Y;
	(*this).Z *= b.Z;
}
void hVec3::operator/=(hVec3 b){
	(*this).X /= b.X;
	(*this).Y /= b.Y;
	(*this).Z /= b.Z;
}
void hVec3::operator*=(float b){
	(*this).X *= b;
	(*this).Y *= b;
	(*this).Z *= b;
}
void hVec3::operator/=(float b){
	(*this).X /= b;
	(*this).Y /= b;
	(*this).Z /= b;
}


float hVec3::operator[](int i){
	switch (i){
		case 0:
			return X;
		case 1:
			return Y;
		case 2:
			return Z;
		default:
			return 0;
	}
}