#include "Mesh.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include "simplexnoise.h"

using namespace hvl;

SurfaceNode::SurfaceNode(){
	index = -1;
	connections = new std::vector<int>();
}
SurfaceNode::SurfaceNode(int i){
	index = i;
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
	densityMap = new float[w*h*d];
}

Mesh::~Mesh(){
	
}

void Mesh::setDensity(int x, int y, int z, float d) {
	densityMap[(x + width * (y + depth * z))] = d;
}

float Mesh::getDensity(float x, float y, float z, bool local){
	if (!local) {
		x -= position.x();
		y -= position.y();
		z -= position.z();
		x *= scale;
		y *= scale;
		z *= scale;
	}
	if (x <= 0 || y <= 0 || z <= 0 || x >= width - 1 || y >= height - 1 || z >= depth - 1)
		return -1;
	switch (shape){
		case MESH_SHAPE_SPHERE:
			return 1.f - (osg::Vec3(x, y, z) - osg::Vec3(width / 2, height / 2, depth / 2)).length() / size;
		case MESH_SHAPE_BOX:{
			int s = size / 2;
			if (x <= width / 2 - s || y <= height / 2 - s || z <= depth / 2 - s ||
				x >= width / 2 + s || y >= height / 2 + s || z >= depth / 2 + s)
				return -1;
			return 1;
		}
		case MESH_SHAPE_NOISE:{
			return raw_noise_3d(x * size, y * size, z * size);
		}
		case MESH_SHAPE_DENSITY:{
			if (x >= 0 && y >= 0 && z >= 0 && x < width && y < height && z < depth)
				return densityMap[((int)x + width * ((int)y + depth * (int)z))];
			return -1;
		}
		default:
			return -1;
	}
}

// returns whether x is above density threshold
bool Mesh::isSolid(float x){
	return x > 0;
}

// find position between c1 and c2 where density is 0
osg::Vec3 Mesh::cLerp(corner c1, corner c2){
	float mu = osg::clampBetween<float>(
		-c1.density / (c2.density - c1.density),
		0, 1);;
	if (isnan(mu) || !isfinite(mu))
		mu = 0;
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
	unsigned char index = 0;
	if (isSolid(corners[0].density)) index |= 1;
	if (isSolid(corners[1].density)) index |= 2;
	if (isSolid(corners[2].density)) index |= 4;
	if (isSolid(corners[3].density)) index |= 8;
	if (isSolid(corners[4].density)) index |= 16;
	if (isSolid(corners[5].density)) index |= 32;
	if (isSolid(corners[6].density)) index |= 64;
	if (isSolid(corners[7].density)) index |= 128;

	if (index == 0 || index >= 255)
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
		osg::Vec3 pts[] = { vts[Triangles[index][i]], vts[Triangles[index][i + 1]], vts[Triangles[index][i + 2]] };
		osg::Vec3 norm = (pts[0] - pts[1]) ^ (pts[0] - pts[2]);
		norm.normalize();
		
		// check vertex array for duplicates
		for (int v = 0; v < 3; v++) {
			bool add = true;
			for (int v2 = meshVerts->size() - 1; v2 >= 0; v2--) {
				if ((meshVerts->at(v2) - pts[v]).length2() < .01f) {
					meshIndicies->push_back(v2);
					meshNormals->at(v2) += norm;
					add = false;
					break;
				}
			}
			if (add) {
				meshVerts->push_back(pts[v]);
				meshNormals->push_back(norm);
				meshColors->push_back(osg::Vec4(1, 0, 0, 1));
				meshIndicies->push_back(meshVerts->size()-1);
			}
		}
	}
}

bool Mesh::generate() {
	if (width == 0 || height == 0 || depth == 0)
		return false;

	printf("creating node\n");

	meshGeode = new osg::Geode();
	meshGeometry = new osg::Geometry();
	meshGeode->addDrawable(meshGeometry);
	meshGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

	meshGeometry->setUseDisplayList(false);
	meshGeometry->setUseVertexBufferObjects(true);

	meshVerts = new osg::Vec3Array();
	meshNormals = new osg::Vec3Array();
	meshColors = new osg::Vec4Array();
	meshIndicies = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);

	meshGeometry->setVertexArray(meshVerts);
	meshGeometry->setColorArray(meshColors);
	meshGeometry->setNormalArray(meshNormals);
	meshGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	meshGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	printf("building voxels\n");
	// build mesh
	float i = 0;
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			for (int z = 0; z < depth; z++) {
				buildVoxel(x, y, z);
				i++;
			}
		}
		// progress bar
		printf("[");
		float p = (i / (width*height*depth));
		int w = (int)(p * 50);
		for (int c = 0; c < w; c++)
			std::cout << "=";
		std::cout << ">";
		for (int c = 0; c < 50-w; c++)
			std::cout << " ";
		printf("]%d%%\r", (int)(p*100));
	}
	printf("\n");

	meshDefVerts = new osg::Vec3Array(*meshVerts);
	meshOrigVerts = new osg::Vec3Array(*meshVerts);

	printf("normalizing normals\n");
	for (int i = 0; i < meshNormals->size(); i++)
		meshNormals->at(i).normalize();

	printf("registering object\n");
	meshGeometry->addPrimitiveSet(meshIndicies);

	meshSceneObject = new cvr::SceneObject("Mesh", false, false, false, true, false);
	meshSceneObject->setNavigationOn(true);
	meshSceneObject->addChild(meshGeode);

	cvr::PluginHelper::registerSceneObject(meshSceneObject, "Mesh");
	meshSceneObject->attachToScene();
	meshSceneObject->setPosition(position * scale);
	meshSceneObject->setScale(scale);

	printf("building surface graph\n");/*
	// build surface graph
	nodes[meshVerts->size()];
	for (int i = 0; i < meshVerts->size(); i++){
		nodes[i] = SurfaceNode(i);

		// go through each triangle and add nodes
		for (int j = 0; j < meshIndicies->size(); j += 3){
			int i1 = meshIndicies->at(j);
			int i2 = meshIndicies->at(j + 1);
			int i3 = meshIndicies->at(j + 2);

			if (i1 == i || i2 == i || i3 == i){
				// this triangle contains the current vertex
				if (i1 != i && find(nodes[i].connections->begin(), nodes[i].connections->end(), i1) != nodes[i].connections->end()){
					nodes[i].connections->push_back(i1);
				}
				if (i2 != i && std::find(nodes[i].connections->begin(), nodes[i].connections->end(), i2) != nodes[i].connections->end()){
					nodes[i].connections->push_back(i2);
				}
				if (i2 != i && std::find(nodes[i].connections->begin(), nodes[i].connections->end(), i3) != nodes[i].connections->end()) {
					nodes[i].connections->push_back(i3);
				}
			}
		}
	}*/
	built = true;
	return true;
}

void Mesh::recalcNorms(bool* updatelist){
	int vCount = meshVerts->size();
	int iCount = meshIndicies->size();
	// list of verticies that have normals reset
	bool* reset = new bool[vCount];

	for (int i = 0; i < iCount; i += 3){
		if (updatelist[meshIndicies->at(i)] || updatelist[meshIndicies->at(i + 1)] || updatelist[meshIndicies->at(i + 2)]){
			// reset verts if not already reset
			if (!reset[meshIndicies->at(i)]){
				meshNormals->at(i) = osg::Vec3(0, 0, 0);
				reset[meshIndicies->at(i)] = true;
			}
			if (!reset[meshIndicies->at(i + 1)]){
				meshNormals->at(i + 1) = osg::Vec3(0, 0, 0);
				reset[meshIndicies->at(i + 1)] = true;
			}
			if (!reset[meshIndicies->at(i + 2)]){
				meshNormals->at(i + 2) = osg::Vec3(0, 0, 0);
				reset[meshIndicies->at(i + 2)] = true;
			}

			// calculate triangle normal
			osg::Vec3 norm =
				(meshVerts->at(meshIndicies->at(i)) - meshVerts->at(meshIndicies->at(i + 1))) ^ (meshVerts->at(meshIndicies->at(i)) - meshVerts->at(meshIndicies->at(i + 2)));

			// add to each vertex
			meshNormals->at(i) = meshNormals->at(meshIndicies->at(i)) + norm;
			meshNormals->at(i + 1) = meshNormals->at(meshIndicies->at(i)) + norm;
			meshNormals->at(i + 2) = meshNormals->at(meshIndicies->at(i)) + norm;
		}
	}

	// normalize vertex normal
	for (int i = 0; i < vCount; i++){
		osg::Vec3 vert = meshNormals->at(meshIndicies->at(i));
		vert.normalize();
		meshNormals->at(i) = vert;
	}

	delete[] reset;
}

void Mesh::Update(float ds){
	if (!built)
		return;

	bool* def = new bool[meshVerts->size()]; // which verts are deformed

	int deformed = 0;
	for (int i = 0; i < meshVerts->size(); i++){
		if (vertDeforms[i] > 0){
			deformed++;

			if (vertDeforms[i] >= 1)
				meshVerts->at(i) = meshDefVerts->at(i);
			else
				meshVerts->at(i) = meshVerts->at(i) + (meshDefVerts->at(i) - meshVerts->at(i)) * vertDeforms[i];

			vertDeforms[i] -= ds;
			if (vertDeforms[i] <= 0)
				vertDeforms[i] = 0;

			// mark vertex as deformed
			def[i] = true;
		}
		else
			meshVerts->at(i) = meshOrigVerts->at(i);
	}

	// update normals of verticies that were deformed
	recalcNorms(def);
	delete[] def;
}