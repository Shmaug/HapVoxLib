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
	printf("HapVoxLib Initialized\n");
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
	
	printf("HapVoxLib initialized\n");
	*/
	printf("HapVoxLib generating densities\n");
	Mesh m = Mesh(osg::Vec3(0, 10, 0), 10, 10, 10);
	int i = 0;
	for (int x = 0; x < 10; x++){
		for (int y = 0; y < 10; y++){
			for (int z = 0; z < 10; z++){
				if (x*x + y*y + z*z < 16)
					m.densities[x][y][z] = 1;
			}
		}
	}
	printf("generating mesh\n");
	m.generate();
	//printf("adding mesh\n");
	//Haptics::addMesh(m);

	/*
	osg::Geometry* geom = new osg::Geometry();
	osg::Geode* geode = new osg::Geode();
	geode->addDrawable(geom);

	cvr::SceneObject* so = new cvr::SceneObject("Object", false, false, false, true, true);
	so->addChild(geode);
	cvr::PluginHelper::registerSceneObject(so, "Object");
	*/
	printf("HapVoxLib done\n");

	return true;
}

void HapVoxCVR::preFrame(){
	Haptics::Update(.03f);
}