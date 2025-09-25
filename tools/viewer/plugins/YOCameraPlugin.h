/*
 * YOCameraPlugin.h
 *
 *  Created on: Sep 25, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOCAMERAPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOCAMERAPLUGIN_H_

#include "IPlugin.h"
#include "YOFlyController.h"

class YOCameraPlugin: public IPlugin {
	YOVariant *cameras_cfg_ {nullptr};
	YOVariant *params_cfg_ {nullptr};
	int camera_select_ {0};
	YOGui gui_;
	Camera *camera_;
	YOFlyController *fc_;

public:
	YOCameraPlugin(Context *context, Camera *camera, YOFlyController *fc);
	virtual ~YOCameraPlugin();
	void OnStart() override;
	void OnGui() override;
	void OnUpdate(float timeStep) override;

	void OnGuiChanged(const std::string &path, std::vector<int> &addr,  YOVariant *cfg);
};

#endif /* TOOLS_VIEWER_PLUGINS_YOCAMERAPLUGIN_H_ */
