/*
 * YOPlotterPlugin.h
 *
 *  Created on: Oct 3, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOPLOTTERPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOPLOTTERPLUGIN_H_
#include "IPlugin.h"
#include <deque>


struct YOSampleInfo
{
	YOTimestamp timestamp;
	float value;
};

struct YOStreamInfo
{
	std::deque<YOSampleInfo> stream;
	ImColor color {256,64,64,128};
	float width {2.0f};
};

class YOPlotterPlugin: public IPlugin
{
	std::string topic_ {"PLOTTER"};
	int buf_size_ {512};
	std::map <std::string, YOStreamInfo> data_;
	float time_scale_ {0.0000001f};

public:
	YOPlotterPlugin(Context *context);
	virtual ~YOPlotterPlugin();
	void OnStart()override;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnUpdate(float timeStep) override;
	void OnGui() override;
	void AddValue(const std::string &name, YOTimestamp ts, const float &value);
};

#endif /* TOOLS_VIEWER_PLUGINS_YOPLOTTERPLUGIN_H_ */
