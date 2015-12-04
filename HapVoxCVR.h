#ifndef HAPVOXCVR
#define HAPVOXCVR

#include <cvrKernel/CVRPlugin.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/SceneManager.h>
#include <cvrKernel/SceneObject.h>
#include <cvrMenu/MenuSystem.h>
#include <cvrMenu/SubMenu.h>
#include <cvrMenu/MenuButton.h>

#include <osg/MatrixTransform>
#include <osg/CullFace>
#include <osg/Matrix>

#include <HD/hd.h>

#include <vector>

namespace hvl{
	static HHD hHD;
	static HDSchedulerHandle updatehandle;
}
class HapVoxCVR : public cvr::CVRPlugin{
public:
	HapVoxCVR();
	virtual ~HapVoxCVR();

	bool init();
	void preFrame();
};

#endif