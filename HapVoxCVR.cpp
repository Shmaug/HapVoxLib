#include "Haptics.h"
#include <Windows.h>
#include "HapVoxCVR.h"

#include <iostream>
#include <cstring>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace hvl;

CVRPLUGIN(HapVoxCVR)

HapVoxCVR::HapVoxCVR(){

}
HapVoxCVR::~HapVoxCVR(){
	hdStopScheduler();
	hdUnschedule(updatehandle);
	hdDisableDevice(hvl::hHD);
}

HDCallbackCode HDCALLBACK HapVoxCallback(void *data){
	
	hdBeginFrame(hvl::hHD);
		
	double dpos[] = { 0, 0, 0 };
	hdGetDoublev(HD_CURRENT_POSITION, dpos);

	osg::Vec3 hpos = osg::Vec3(dpos[0], dpos[1], dpos[2]);
	osg::Vec3 force = osg::Vec3();
	Haptics::doFrame(hpos, force);

	double dforce[] = { force.x(), force.y(), force.z() };

	hdSetDoublev(HD_CURRENT_FORCE, dforce);
	hdEndFrame(hvl::hHD);
	
	return HD_CALLBACK_CONTINUE;
}

bool HapVoxCVR::init(){
	printf("HapVoxLib Initializing\n");
	/*
	HDErrorInfo error;
	hvl::hHD = hdInitDevice(HD_DEFAULT_DEVICE);
	if (HD_DEVICE_ERROR(error = hdGetError()))
	{
		printf("Failed to initialize haptic device\n");
		return false;
	}
	printf("Haptic device initialized\n");

	updatehandle = hdScheduleAsynchronous(HapVoxCallback, this, HD_MAX_SCHEDULER_PRIORITY);
	hdEnable(HD_FORCE_OUTPUT);
	hdStartScheduler();
	
	printf("Generating mesh\n");
	Mesh* m = new Mesh(osg::Vec3(0, 0, -10), 10, 10, 10);
	m->generate();
	*/
	//printf("adding mesh\n");
	//Haptics::addMesh(m);
	osg::Group* root = new osg::Group();
	osg::Geometry* geom = new osg::Geometry();
	osg::Geode* geode = new osg::Geode();
	
	geom->setUseDisplayList(false);
	geode->addDrawable(geom);
	root->addChild(geode);

	osg::Vec3Array* vArray = new osg::Vec3Array();
	vArray->push_back(osg::Vec3(0, 0, 0));
	vArray->push_back(osg::Vec3(10, 0, 0));
	vArray->push_back(osg::Vec3(10, 10, 0));
	vArray->push_back(osg::Vec3(0, 10, 0));
	vArray->push_back(osg::Vec3(5, 5, 10));
	geom->setVertexArray(vArray);

	osg::DrawElementsUInt* base = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
	base->push_back(3);
	base->push_back(2);
	base->push_back(1);
	base->push_back(0);
	geom->addPrimitiveSet(base);

	osg::DrawElementsUInt* f1 = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	f1->push_back(0);
	f1->push_back(1);
	f1->push_back(4);
	geom->addPrimitiveSet(f1);

	osg::DrawElementsUInt* f2 = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	f2->push_back(1);
	f2->push_back(2);
	f2->push_back(4);
	geom->addPrimitiveSet(f2);

	osg::DrawElementsUInt* f3 = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	f3->push_back(2);
	f3->push_back(3);
	f3->push_back(4);
	geom->addPrimitiveSet(f3);

	osg::DrawElementsUInt* f4 = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	f4->push_back(3);
	f4->push_back(0);
	f4->push_back(4);
	geom->addPrimitiveSet(f4);

	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f)); //index 0 red
	colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f)); //index 1 green
	colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f)); //index 2 blue
	colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)); //index 3 white 
	colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f)); //index 4 red

	geom->setColorArray(colors);
	geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	root->addChild(geode);

	cvr::SceneObject* so = new cvr::SceneObject("Object", false, false, false, true, true);
	so->setShowBounds(true);
	so->addChild(root);

	osg::Matrix trans;
	trans.makeTranslate(osg::Vec3(0, 0, 15));
	so->setTransform(trans);

	cvr::PluginHelper::registerSceneObject(so, "Object");
	
	printf("HapVoxLib initialized\n");

	return true;
}

void HapVoxCVR::preFrame(){
	Haptics::Update(cvr::PluginHelper::getLastFrameDuration());
}