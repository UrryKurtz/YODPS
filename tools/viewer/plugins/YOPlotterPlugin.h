/*
 * YOPlotterPlugin.h
 *
 *  Created on: Oct 3, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOPLOTTERPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOPLOTTERPLUGIN_H_
#include "IPlugin.h"
#include "YOGui.h"
#include <deque>

struct YOSampleInfo
{
	YOTimestamp timestamp;
	float value;
};

struct YOStreamInfo
{
	float last;
	std::deque<YOSampleInfo> stream;
	YOVariant *strean_cfg;
};

class YOPlotterPlugin: public IPlugin
{
	YOVariant *streams_ {nullptr};
	YOVariant *settings_ {nullptr};
	std::string topic_ {"PLOTTER"};
	int buf_size_ {512};
	bool mouse_pause_ {false};
	bool pause_ {false};
	std::map <std::string, YOStreamInfo> data_;
	std::map <std::string, std::mutex> lock_;
	float time_scale_ {0.0000001f};
	//int time_period_ {10000000};
	YOTimestamp last_ts_ {0};
	YOGui gui_;

public:
	YOPlotterPlugin(Context *context);
	virtual ~YOPlotterPlugin();
	void OnStart()override;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnUpdate(float timeStep) override;
	void OnGui() override;
	void AddValue(const std::string &name, YOTimestamp ts, const float &value);
	void DrawStream(YOVariant &config, const std::string &name, float last);
	void DrawSettings(YOVariant &config);
};

#endif /* TOOLS_VIEWER_PLUGINS_YOPLOTTERPLUGIN_H_ */
