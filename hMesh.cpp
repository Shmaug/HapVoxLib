#include "hMesh.h"
#include <cmath>
#include <algorithm>

using namespace hvl;

SurfaceNode::SurfaceNode(){
	index = -1;
	numConnections = 0;
	connections[0];
}
SurfaceNode::SurfaceNode(int in, int* con, int nc){
	index = in;
	numConnections = nc;

	for (int i = 0; i < 32; i++)
		if (i < 32)
			connections[i] = con[i];
		else
			connections[i] = -1;
}

corner::corner(){
	pos = osg::Vec3();
	density = 0;
}
corner::corner(float d, osg::Vec3 p){
	pos = p;
	density = d;
}

Mesh::Mesh(){
}

Mesh::Mesh(osg::Vec3 pos, int w, int h, int d){
	position = pos;
	width = w;
	height = h;
	depth = d;
	densities = std::vector<float>();
}

Mesh::~Mesh(){

}

float Mesh::getDensity(float x, float y, float z, bool local){
	if (x < 0 || y < 0 || z < 0 || x >= width || y >= height || z >= depth)
		return 0;
	float d = 1.f - (x*x + y*y + z*z) / 25.f;
	return d;
	/*if (local)
		return densities[(int)x][(int)y][(int)z];
	else
		return densities[(int)(x - position.x())][(int)(y - position.y())][(int)(z - position.z())];*/
}

// returns whether x is above density threshold
bool Mesh::isSolid(float x){
	return x > 0;
}

// find position between c1 and c2 where density is 0
osg::Vec3 Mesh::cLerp(corner c1, corner c2){
	float mu = -c1.density / (c2.density - c1.density);
	if (isnan(mu) || !isfinite(mu))
		mu = 0;
	if (mu > 1) mu = 1;
	if (mu < 0) mu = 0;
	return c1.pos + ((c2.pos - c1.pos) * mu);
}

void Mesh::buildVoxel(float x, float y, float z){
	float s = 1;
	corner corners[] = {
		corner(getDensity(x, y, z),				osg::Vec3(x, y, z)),			// 0
		corner(getDensity(x, y + s, z),			osg::Vec3(x, y + s, z)),		// 1
		corner(getDensity(x + s, y + s, z),		osg::Vec3(x + s, y + s, z)),	// 2
		corner(getDensity(x + s, y, z),			osg::Vec3(x + s, y, z)),		// 3

		corner(getDensity(x, y, z + s),			osg::Vec3(x, y, z + s)),		// 4
		corner(getDensity(x, y + s, z + s),		osg::Vec3(x, y + s, z + s)),	// 5
		corner(getDensity(x + s, y + s, z + s), osg::Vec3(x + s, y + s, z + s)),// 6
		corner(getDensity(x + s, y, z + s),		osg::Vec3(x + s, y, z + s))		// 7
	};


	// create index
	int index = 0;
	if (isSolid(corners[0].density)) index |= 1;
	if (isSolid(corners[1].density)) index |= 2;
	if (isSolid(corners[2].density)) index |= 4;
	if (isSolid(corners[3].density)) index |= 8;
	if (isSolid(corners[4].density)) index |= 16;
	if (isSolid(corners[5].density)) index |= 32;
	if (isSolid(corners[6].density)) index |= 64;
	if (isSolid(corners[7].density)) index |= 128;

	if (index == 0 || index > 128)
		return; // voxel completely inside/outside, no verticies

	// check each bit in index and place verticies from edge lookup table
	osg::Vec3 vts[12];
	if ((Edges[index] & 1) > 0)
		vts[0] = cLerp(corners[0], corners[1]);
	if ((Edges[index] & 2) > 0)
		vts[1] = cLerp(corners[1], corners[2]);
	if ((Edges[index] & 4) > 0)
		vts[2] = cLerp(corners[2], corners[3]);
	if ((Edges[index] & 8) > 0)
		vts[3] = cLerp(corners[3], corners[0]);

	if ((Edges[index] & 16) > 0)
		vts[4] = cLerp(corners[4], corners[5]);
	if ((Edges[index] & 32) > 0)
		vts[5] = cLerp(corners[5], corners[6]);
	if ((Edges[index] & 64) > 0)
		vts[6] = cLerp(corners[6], corners[7]);
	if ((Edges[index] & 128) > 0)
		vts[7] = cLerp(corners[7], corners[4]);

	if ((Edges[index] & 256) > 0)
		vts[8] = cLerp(corners[0], corners[4]);
	if ((Edges[index] & 512) > 0)
		vts[9] = cLerp(corners[1], corners[5]);
	if ((Edges[index] & 1024) > 0)
		vts[10] = cLerp(corners[2], corners[6]);
	if ((Edges[index] & 2048) > 0)
		vts[11] = cLerp(corners[3], corners[7]);

	// create verticies from triangle table
	for (int i = 0; Triangles[index][i] != -1; i += 3){
		osg::Vec3 v1 = vts[Triangles[index][i]];
		osg::Vec3 v2 = vts[Triangles[index][i + 1]];
		osg::Vec3 v3 = vts[Triangles[index][i + 2]];
		osg::Vec3 norm = (v1 - v3) ^ (v1 - v2);

		meshVerts->push_back(v1);
		meshVerts->push_back(v3);
		meshVerts->push_back(v2);

		meshNormals->push_back(norm);
		meshColors->push_back(osg::Vec4(1, 0, 0, 1));
	}
}

bool Mesh::generate(){
	if (width == 0 || height == 0 || depth == 0)
		return false;

	meshNormals = new osg::Vec3Array();
	meshColors = new osg::Vec4Array();
	meshVerts = new osg::Vec3Array();
	meshDefVerts = new osg::Vec3Array();
	meshOrigVerts = new osg::Vec3Array();
	meshIndicies = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

	meshNormals->setBinding(osg::Array::BIND_PER_PRIMITIVE_SET);
	meshColors->setBinding(osg::Array::BIND_PER_PRIMITIVE_SET);
	
	numVert = 0;
	numInd = 0;
	
	printf("building voxels\n");
	// build mesh
	int i = 0;
	for (int x = 0; x < width; x++){
		for (int y = 0; y < height; y++){
			for (int z = 0; z < depth; z++){
				buildVoxel(x, y, z);// verts, norms, inds);
				i++;
			}
		}
		//printf("building voxels %d%% \n", (int)((i/(float)(width*height*depth)*100)));
	}
	printf("%d triangles\n", meshVerts->size());
	printf("registering with osg\n");
	
	meshGeometry = new osg::Geometry();
	meshGeometry->setUseDisplayList(false);
	meshGeometry->setVertexArray(meshVerts);
	meshGeometry->setColorArray(meshColors);
	meshGeometry->setNormalArray(meshNormals);
	//meshGeometry->addPrimitiveSet(meshIndicies);

	meshGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	meshGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	meshGeode = new osg::Geode();
	meshGeode->addDrawable(meshGeometry);
	meshGeode->dirtyBound();

	meshSceneObject = new cvr::SceneObject("Mesh", false, false, false, true, true);
	cvr::PluginHelper::registerSceneObject(meshSceneObject, "Mesh");
	meshSceneObject->addChild(meshGeode);
	meshSceneObject->attachToScene();

	osg::Matrixd transform = osg::Matrixd();
	transform.makeTranslate(position);

	meshSceneObject->setTransform(transform);

	printf("%f, %f, %f\n", position[0], position[1], position[2]);
	/*
	// build surface graph
	nodes[numVert];
	for (int i = 0; i < numVert; i++){
		nodes[i].numConnections = 0;
		nodes[i].connections[32];
		for (int k = 0; k < 32; k++)
			nodes[i].connections[k] = -1;

		// go through each triangle and add nodes
		for (int j = 0; j < numInd; j += 3){
			int i1 = meshIndicies->at(j);
			int i2 = meshIndicies->at(j + 1);
			int i3 = meshIndicies->at(j + 2);

			if (i1 == i || i2 == i || i3 == i){
				// this triangle contains the current vertex
				if (i1 != i && !std::find(std::begin(nodes[i].connections), std::end(nodes[i].connections), i1)){
					// index isn't self and isn't already in connections
					nodes[i].connections[nodes[i].numConnections] = i1;
					nodes[i].numConnections++;
				}
				if (i2 != i && !std::find(std::begin(nodes[i].connections), std::end(nodes[i].connections), i2)){
					// index isn't self and isn't already in connections
					nodes[i].connections[nodes[i].numConnections] = i2;
					nodes[i].numConnections++;
				}
				if (i3 != i && !std::find(std::begin(nodes[i].connections), std::end(nodes[i].connections), i3)){
					// index isn't self and isn't already in connections
					nodes[i].connections[nodes[i].numConnections] = i3;
					nodes[i].numConnections++;
				}
			}
		}
	}*/
	built = true;
	return true;
}

void Mesh::recalcNorms(bool* updatelist){
	// list of verticies that have normals reset
	bool* reset = new bool[numVert];

	for (int i = 0; i < numInd; i += 3){
		if (updatelist[meshIndicies->at(i)] || updatelist[meshIndicies->at(i + 1)] || updatelist[meshIndicies->at(i + 2)]){
			// reset verts if not already reset
			if (!reset[meshIndicies->at(i)]){
				meshNormals->assign(i, osg::Vec3(0, 0, 0));
				reset[meshIndicies->at(i)] = true;
			}
			if (!reset[meshIndicies->at(i + 1)]){
				meshNormals->assign(i + 1, osg::Vec3(0, 0, 0));
				reset[meshIndicies->at(i + 1)] = true;
			}
			if (!reset[meshIndicies->at(i + 2)]){
				meshNormals->assign(i + 2, osg::Vec3(0, 0, 0));
				reset[meshIndicies->at(i + 2)] = true;
			}

			// calculate triangle normal
			osg::Vec3 norm =
				(meshVerts->at(meshIndicies->at(i)) - meshVerts->at(meshIndicies->at(i + 1))) ^ (meshVerts->at(meshIndicies->at(i)) - meshVerts->at(meshIndicies->at(i + 2)));

			// add to each vertex
			meshNormals->assign(i, meshNormals->at(meshIndicies->at(i)) + norm);
			meshNormals->assign(i + 1, meshNormals->at(meshIndicies->at(i)) + norm);
			meshNormals->assign(i + 2, meshNormals->at(meshIndicies->at(i)) + norm);
		}
	}

	// normalize vertex normal
	for (int i = 0; i < numVert; i++){
		osg::Vec3 vert = meshNormals->at(meshIndicies->at(i));
		vert.normalize();
		meshNormals->assign(i, vert);
	}

	delete[] reset;
}

void Mesh::Update(float ds){
	if (!built)
		return;

	bool* def = new bool[numVert];

	int deformed = 0;
	for (int i = 0; i < numVert; i++){
		if (vertDeforms[i] > 0){
			deformed++;

			if (vertDeforms[i] >= 1)
				meshVerts->assign(i, meshDefVerts->at(i));
			else
				meshVerts->assign(i, (meshVerts->at(i) + (meshDefVerts->at(i) - meshVerts->at(i)) * vertDeforms[i]));

			vertDeforms[i] -= ds;
			if (vertDeforms[i] <= 0)
				vertDeforms[i] = 0;

			// mark vertex as deformed
			def[i] = true;
		}
		else
			meshVerts->assign(i, meshOrigVerts->at(i));
	}
	// update normals of verticies that were deformed
	recalcNorms(def);
	delete[] def;
}