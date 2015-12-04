#include "Haptics.h"
#include <Windows.h>
#include "HapVoxCVR.h"
#include "hvlIO.h"

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
	cvr::PluginHelper::setClearColor(osg::Vec4(.3f,.3f,.3f, 1));
	Haptics::init();

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
	*/

	printf("Generating meshes\n");
	readImage();

	int s = 50;
	Mesh* m = new Mesh(osg::Vec3(-s / 2.f, -s / 2.f, -s / 2.f), s, s, s);
	m->shape = Mesh::MESH_SHAPE_DENSITY;
	m->scale = 30.f;
	m->size = 20;
	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			for (int z = 0; z < s; z++) {
				m->setDensity(x, y, z, 1.f - (osg::Vec3(x, y, z) - osg::Vec3(m->width / 2, m->height / 2, m->depth / 2)).length() / m->size);
			}
		}
	}
	m->generate();

	printf("HapVoxLib initialized\n");

	return true;
}

void HapVoxCVR::preFrame(){
	Haptics::Update(cvr::PluginHelper::getLastFrameDuration());

	osg::Vec3 pos;
	osg::Vec3 force;
	Haptics::doFrame(pos, force);
}