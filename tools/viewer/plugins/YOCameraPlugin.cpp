/*
 * YOCameraPlugin.cpp
 *
 *  Created on: Sep 25, 2025
 *      Author: kurtz
 */

#include "YOCameraPlugin.h"
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Resource/ResourceCache.h>

namespace yo::k
{
	 YO_KEY(window,    "win")
}



inline void createCameraCfg(uint32_t i, YOVariant &config, const std::string &name)
{
	config.m_name = name;
	config[yo::k::select] = false;
	config[yo::k::record] = false;
	config[yo::k::frames] = 20u;
	config[yo::k::mask] = 0xFFFFFFFFu;
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
 cache_ = GetSubsystem<ResourceCache>();
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
    scene_ = camera_->GetScene();
}

void YOCameraPlugin::CreateCameraWindow(int i)
{
	std::shared_ptr<YOViewStruct> info = std::make_shared<YOViewStruct>();
	info->node = scene_->CreateChild("TEST NODE");
	info->cam = MakeShared<Camera>(context_);
	info->txt = MakeShared<Texture2D>(context_);

	info->fly = info->node->CreateComponent<YOFlyController >();
	info->fly->SetCamera(info->cam);

	info->txt->SetNumLevels(1);
	info->txt->SetSize(1920, 1080, TextureFormat::TEX_FORMAT_RGB32_FLOAT, TextureFlag::BindRenderTarget);
	//auto vp = MakeShared<Viewport>(context_, scene_, camera_);
	auto vp = MakeShared<Viewport>(context_, scene_, info->cam);
	info->txt->SetFilterMode(TextureFilterMode::FILTER_ANISOTROPIC);
	info->txt->GetRenderSurface()->SetViewport(0, vp);
	info->txt->GetRenderSurface()->SetUpdateMode(SURFACE_UPDATEALWAYS);
	auto systemUI = GetSubsystem<SystemUI>();
	systemUI->ReferenceTexture(info->txt);
	auto &camx = (*cameras_cfg_)[i];
	uint32_t &mask = camx[yo::k::mask].getU32();
	info->cam->SetViewMask(mask);
	YOVector3 &pos = camx[yo::k::position].get<YOVector3>();
	YOVector3 &rot = camx[yo::k::rotation].get<YOVector3>();
	YOLimitF &fov = camx[yo::k::fov];
	info->fly->MoveTo(camx[yo::k::frames].get<uint32_t>() / 10, (Vector3&)pos, rot.z, rot.y, rot.x, fov.value);
	info->fly->EnableUpdate(false);
	views_[i] = info;
}

void YOCameraPlugin::DrawCameraWindow(YOFlyController *fly, Texture2D *txt, int id)
{
	ImVec2 avail = ui::GetContentRegionAvail();
	ImVec2 draw(txt->GetWidth(), txt->GetHeight());
	ImVec2 uv0{0,0};
	ImVec2 uv1{1,1};

	if (draw.x > avail.x)
	{
		float vis = avail.x / draw.x;
		float pad = (1.0f - vis) * 0.5f;
		uv0.x = pad; uv1.x = 1.0f - pad;
	}
	if (draw.y > avail.y)
	{
		float vis = avail.y / draw.y;
		float pad = (1.0f - vis) * 0.5f;
		uv0.y = pad; uv1.y = 1.0f - pad;
	}

	ui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(1,1,1,1));
	if (ui::BeginChild("Canvas", avail, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse))
	{
		ui::Image(ToImTextureID(txt), avail, uv0, uv1, ImVec4(1,1,1,1));
		bool focused = ui::IsWindowFocused() && ui::IsItemHovered();
		fly->EnableUpdate(focused);

		if( focused )
		{
			ImVec2 drag_right = ui::GetMouseDragDelta(MouseButton::MOUSEB_RIGHT);
			if(focused && drag_right != ImVec2(0,0))
			{
				fly->DragRight({drag_right.x, drag_right.y}, {avail.x, avail.y});
			}
			ui::ResetMouseDragDelta(MouseButton::MOUSEB_RIGHT);

			ImVec2 drag_mid = ui::GetMouseDragDelta(MouseButton::MOUSEB_MIDDLE);
			ui::ResetMouseDragDelta(MouseButton::MOUSEB_MIDDLE);

			ImVec2 mp = ui::GetMousePos() - ui::GetItemRectMin();
			ImVec2 drag_left = ui::GetMouseDragDelta(MouseButton::MOUSEB_LEFT);
			if(focused && drag_left != ImVec2(0,0))
			{
				float scaleX = avail.x / 1920;
				float scaleY = avail.y / 1080;

				drag_left.x *= scaleX;
				drag_left.y *= scaleY;

				fly->DragLeft({drag_left.x, drag_left.y}, {avail.x, avail.y}, {mp.x, mp.y});
			}
			ui::ResetMouseDragDelta(MouseButton::MOUSEB_LEFT);
		}
		ui::EndChild();
		ui::PopStyleColor();
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

    for ( auto &cam : cameras_cfg_->getArray())
    {
    	if(!cam.hasChild(yo::k::window)) //recreate param
    		cam[yo::k::window] = false;

    	uint32_t id = cam[yo::k::id];

    	if(cam[yo::k::window])
    	{
			if(!views_[id])
				CreateCameraWindow(id);

			ui::Begin( cam[yo::k::name].c_str(), &cam[yo::k::window].getBool());
			DrawCameraWindow(views_[id]->fly.GetPointer(), views_[id]->txt, id);
    	    ui::End();
    	}
    	else if(views_[id])
    	{
    		views_[id].reset();
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

