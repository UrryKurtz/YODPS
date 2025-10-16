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
