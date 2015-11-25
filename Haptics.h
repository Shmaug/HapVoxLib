#include "hMesh.h"

#ifndef HAPTICS
#define HAPTICS

namespace hvl{
	static Mesh* meshes[32];
	static osg::Vec3 lastPos;
	static osg::Vec3 insertionPoint;
	static int insertionVertex;
	static osg::Vec3 drawPos;

	class Haptics {
	private:
		static void deform(Mesh &mesh, osg::Vec3 point, float p, int vInd, int steps, bool* deformed);

	public:
		static osg::Vec3 checkMesh(Mesh &mesh, osg::Vec3 devPos);
		static void doFrame(osg::Vec3 devPos, osg::Vec3& force);
		static void Update(float deltaSeconds);
		static bool addMesh(Mesh m);
	};
}

#endif
