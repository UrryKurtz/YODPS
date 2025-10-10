/*
 * YOCameraPlugin.cpp
 *
 *  Created on: Sep 25, 2025
 *      Author: kurtz
 */

#include "YOCameraPlugin.h"

inline void createCameraCfg(uint32_t i, YOVariant &config, const std::string &name)
{
	config.m_name = name;
	config[yo::k::select] = false;
	config[yo::k::record] = false;
	config[yo::k::frames] = 20u;
	config[yo::k::id] = i;
	config[yo::k::name] = (yo_keys[name] + " " + std::to_string(i)).c_str();
	config[yo::k::comment] = "";
	config[yo::k::overlay] = true;
	config[yo::k::orthogonal] = false;
	config[yo::k::position] = YOVector3{};
    config[yo::k::rotation] = YOVector3{};
    config[yo::k::fov] = YOLimitF { .value = 55.0f, .min = 0.0f, .max = 180.f, .speed = 0.1f};
}

YOCameraPlugin::YOCameraPlugin(Context *context, Camera *camera, YOFlyController *fc) : IPlugin(context), camera_(camera), fc_(fc){
	// TODO Auto-generated constructor stub

}

YOCameraPlugin::~YOCameraPlugin() {
	// TODO Auto-generated destructor stub
}

void YOCameraPlugin::OnStart()
{
    cameras_cfg_ = &(*config_)[yo::k::cameras];
    params_cfg_ = &(*config_)[yo::k::config];

    if(cameras_cfg_->getTypeId() != 1  || cameras_cfg_->getArraySize() != YO_CAMERA_NUM)
    {
    	cameras_cfg_->m_value = YOArray(YO_CAMERA_NUM);
        for(int i = 0; i < YO_CAMERA_NUM; i++)
        {
            createCameraCfg(i, (*cameras_cfg_)[i], yo::k::camera);
        }
    }
    for(int i = 0; i < YO_CAMERA_NUM; i++)
    {
    	YOVariant &cam = (*cameras_cfg_)[i];
    	if(cam[yo::k::select])
    	{
    		camera_select_ = i;
    		YOVector3 &pos = cam[yo::k::position].get<YOVector3>();
    		YOVector3 &rot = cam[yo::k::rotation].get<YOVector3>();
    		YOLimitF &fov = cam[yo::k::fov];
    		fc_->MoveTo(cam[yo::k::frames].get<uint32_t>() / 10, (Vector3&)pos, rot.z, rot.y, rot.x, fov.value);
    		break;
    	}
    }
}

void YOCameraPlugin::OnGui()
{
	//if(config_)
	int start = config_->m_name.length() + 1;

	if(gui_.draw(*config_))
	{
		std::string path = gui_.getPath().substr(start);
		YOVariant *cfg = gui_.getConfig();
		std::vector<int> addr = gui_.getIndex();
		std::cout << "GUI changed: " <<  path << std::endl;
		OnGuiChanged(path, addr, cfg);
	}
}

void YOCameraPlugin::OnUpdate(float timeStep)
{
    if(camera_select_>-1)
    {
    	if((*cameras_cfg_)[camera_select_][yo::k::record])
    	{
    		(*cameras_cfg_)[camera_select_][yo::k::position] = (YOVector3&) fc_->GetPosition();
    		Vector3 ang = fc_->GetRotation();
    		(*cameras_cfg_)[camera_select_][yo::k::rotation] = YOVector3 {ang.x_, ang.y_, ang.z_};
    	}
    }
}

void YOCameraPlugin::OnGuiChanged(const std::string &path, std::vector<int> &addr,  YOVariant *cfg)
{
	std::cout << " OnGuiChanged " <<  path  << " " << addr[0] << std::endl;

	YOVariant &cam = (*cameras_cfg_)[addr[0]];

	if(path == "/cameras/#0/select")
	{
		camera_select_ = addr[0];
		YOVector3 &pos = cam[yo::k::position].get<YOVector3>();
		YOVector3 &rot = cam[yo::k::rotation].get<YOVector3>();
		YOLimitF &fov = cam[yo::k::fov];
		fc_->MoveTo(cam[yo::k::frames].get<uint32_t>(), (Vector3&)pos, rot.z, rot.y, rot.x, fov.value);
	}

	if(addr[0] != camera_select_)
		return;

	if(path == "/cameras/#0/position")
	{
		fc_->SetPosition((const Vector3 &) cam[yo::k::position].get<YOVector3>());
	}
	else if(path == "/cameras/#0/rotation")
	{
		YOVector3 &rot = cam[yo::k::rotation].get<YOVector3>();
		fc_->SetRotation( rot.x , rot.y, rot.z);
	}
	else if(path == "/cameras/#0/fov")
	{
		camera_->SetFov(cfg->get<YOLimitF>().value);
	}

}

