#include "Mesh.h"
#include <vector>

#ifndef HAPTICS
#define HAPTICS

namespace hvl{
	static std::vector<Mesh> Meshes;
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
		static int init();
	};
}

#endif
