/*
 * YOCameraPlugin.h
 *
 *  Created on: Sep 25, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOCAMERAPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOCAMERAPLUGIN_H_

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Container/Ptr.h>

#include "IPlugin.h"
#include "YOFlyController.h"

struct YOViewStruct {
	SharedPtr<Node> node {nullptr};
	SharedPtr<Camera> cam {nullptr};
	SharedPtr<YOFlyController> fly {nullptr};
	SharedPtr<Texture2D> txt {nullptr};
};

class YOCameraPlugin: public IPlugin {
	YOVariant *cameras_cfg_ {nullptr};
	YOVariant *params_cfg_ {nullptr};
	int camera_select_ {0};
	YOGui gui_;
	Camera *camera_ {nullptr};
	Scene *scene_ {nullptr};
	YOFlyController *fc_{nullptr};
	ResourceCache *cache_;
	std::shared_ptr<YOViewStruct> views_[YO_CAMERA_NUM];

public:
	YOCameraPlugin(Context *context);
	virtual ~YOCameraPlugin();
	void OnStart() override;
	void OnGui() override;
	void OnUpdate(float timeStep) override;

	void CreateCameraWindow(int i);
	void DrawCameraWindow(YOFlyController *fly, Texture2D *txt, int id);
	void OnGuiChanged(const std::string &path, std::vector<int> &addr,  YOVariant *cfg);
};

#endif /* TOOLS_VIEWER_PLUGINS_YOCAMERAPLUGIN_H_ */
