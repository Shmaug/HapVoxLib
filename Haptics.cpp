#include "hVec3.h"
#include "hMesh.h"
#include "Haptics.h"
#include <math.h>

using namespace hvl;

void Haptics::deform(Mesh &mesh, osg::Vec3 point, float p, int vInd, int steps, bool* deformed){
	// stop deforming if number of steps outward is more then penetration
	if (steps > p + 1)
		return;

	// stop deforming if we've already deformed this vert
	if (deformed[vInd])
		return;

	deformed[vInd] = true;

	osg::Vec3 pos = point - mesh.position;

	float dist = (mesh.meshOrigVerts->at(vInd), insertionPoint - mesh.position).length(); // current verts to insertion point
	if (dist > p) return; // distance > penetration
	dist = 1 - fminf(fmaxf(dist / p, 0), 1);
	mesh.vertDeforms[vInd] = dist;
	mesh.meshDefVerts->assign(vInd, point - mesh.position);

	for (int i = 0; i < mesh.nodes[vInd].numConnections; i++)
		deform(mesh, point, p, mesh.nodes[vInd].connections[i], steps + 1, deformed);
}

osg::Vec3 Haptics::checkMesh(Mesh &mesh, osg::Vec3 devicePos){
	osg::Vec3 force = osg::Vec3();

	float density = mesh.getDensity(devicePos.x(), devicePos.y(), devicePos.z(), false);
	float lastDensity = mesh.getDensity(lastPos.x(), lastPos.y(), lastPos.z(), false);

	if (mesh.isSolid(density)){
		// device is inside object
		if (!mesh.isSolid(lastDensity)){
			// device just entered object
			// set insertionvertex to closest vertex
			insertionPoint = lastPos;
			float minDist = -1;
			for (int i = 0; i < mesh.numVert; i++){
				float d = (mesh.meshVerts->at(i) - (insertionPoint - mesh.position)).length();
				if (d < minDist || minDist == -1){
					insertionVertex = i;
					minDist = d;
				}
			}
		}

		float push = 1;
		osg::Vec3 out = (insertionPoint - devicePos);
		out.normalize();
		float penetration = (insertionPoint - devicePos).length();

		// factor in densities between insertion point and device pos
		for (float f = 0; f < 1; f += 1 / penetration){
			if (f > 0){
				osg::Vec3 p = insertionPoint + (devicePos - insertionPoint) * f;
				float d = mesh.getDensity(p.x(), p.y(), p.z(), false);

				d = fminf(fmaxf(d, 0.1f), 1); // clamp 0-1
				/* use logistic function to get a value to add to the overall push based on density at point
				   low densities push less than higher densities
				           .5
					------------------
					 1 + e^(-12(d-.6))
				*/
				d = .5f / (1 + powf(2.71828f, -15 * (d - .6f)));
				push += d;
			}
		}

		force = out * penetration * push;

		// max out penetration at 4
		if (penetration > 4){
			penetration = 4;
			devicePos = insertionPoint - (out * penetration);
		}

		bool* deformed = new bool[mesh.numVert];

		deform(mesh, devicePos, penetration, insertionVertex, 0, deformed);

		delete[] deformed;
	}

	return force;
}

void Haptics::doFrame(osg::Vec3 devicePos, osg::Vec3& force){
	for (int i = 0; i < 32; i++)
		if (meshes[i] != 0 && (*meshes[i]).built)
			force += checkMesh(*meshes[i], devicePos);
	lastPos = devicePos;
}

bool Haptics::addMesh(Mesh m){
	bool e = false;
	for (int i = 0; i < 32; i++){
		if (meshes[i] == 0){
			meshes[i] = &m;
			e = true;
		}
	}
	return e;
}

void Haptics::Update(float dT){

}
