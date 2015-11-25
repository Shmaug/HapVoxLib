#ifndef HVEC3
#define HVEC3

namespace hvl{
	class hVec3{
	public:
		float X;
		float Y;
		float Z;

		hVec3();
		hVec3(float num);
		hVec3(float x, float y, float z);

		float Length();
		void Normalize();
		hVec3 Normalized();

		static float Distance(hVec3 a, hVec3 b);
		static hVec3 Lerp(hVec3 a, hVec3 b, float t);
		static hVec3 Cross(hVec3 a, hVec3 b);

		hVec3 operator +(hVec3 b);
		hVec3 operator -(hVec3 b);
		hVec3 operator *(hVec3 b);
		hVec3 operator /(hVec3 b);
		hVec3 operator *(float b);
		hVec3 operator /(float b);

		void operator +=(hVec3 b);
		void operator -=(hVec3 b);
		void operator *=(hVec3 b);
		void operator /=(hVec3 b);
		void operator *=(float b);
		void operator /=(float b);

		float operator [](int i);
	};
}
#endif