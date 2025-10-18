/*
 * YOTestPlugin.h
 *
 *  Created on: Oct 11, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOTESTPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOTESTPLUGIN_H_

#include "IPlugin.h"

using namespace Urho3D;
class YOTestPlugin: public IPlugin
{
	std::string in_topic_;
	std::string in_value_;
	std::string out_topic_;
	std::string out_value_;

	YOVariant data_cfg_ {"Data"};
	YOVariant data_cfg_old_ {"Data"};

	YOVariant sys_cfg_ {"System"};
	YOVariant sys_cfg_old_ {"System"};

	YOGui gui_;
public:
	YOTestPlugin(Context *context);
	virtual ~YOTestPlugin();
	void OnStart() override;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnSystem(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnGui()override;
};

#endif /* TOOLS_VIEWER_PLUGINS_YOTESTPLUGIN_H_ */
