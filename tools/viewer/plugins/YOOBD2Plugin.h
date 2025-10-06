/*
 * YOOBD2Plugin.h
 *
 *  Created on: Oct 6, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOOBD2PLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOOBD2PLUGIN_H_
#include <mutex>
#include "IPlugin.h"
using namespace Urho3D;

class YOOBD2Plugin: public IPlugin
{
	YOVariant *requests_ {nullptr};
	YOVariant *settings_ {nullptr};
	std::shared_ptr<YOVariant> response_ {nullptr};
	std::mutex mutex_;
	YOGui gui_;
	YOTimestamp ts_{0};
public:
	YOOBD2Plugin(Context *context);
	virtual ~YOOBD2Plugin();

	void OnStart() override;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnGui() override;
};

#endif /* TOOLS_VIEWER_PLUGINS_YOOBD2PLUGIN_H_ */
