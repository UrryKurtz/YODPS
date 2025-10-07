/*
 * YOCANPlugin.h
 *
 *  Created on: Oct 7, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOCANPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOCANPLUGIN_H_

#include "IPlugin.h"
using namespace Urho3D;
class YOCANPlugin: public IPlugin
{
	//data[channel][message]
	std::map<int, std::map<int, tCANData>> data_;
	YOVariant dbc_;
	YOGui gui_;

public:
	YOCANPlugin(Context *context);
	virtual ~YOCANPlugin();

	void OnStart() override ;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override ;
	void OnGui() override ;
};

#endif /* TOOLS_VIEWER_PLUGINS_YOCANPLUGIN_H_ */
