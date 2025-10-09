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
	std::map<int, std::vector<uint8_t>> data_;

	std::mutex mutex_;
	YOGui gui_;
	YOTimestamp ts_{0};
	bool poll_{false};

public:
	YOOBD2Plugin(Context *context);
	virtual ~YOOBD2Plugin();

	void OnStart() override;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnGui() override;

	void Poll(uint8_t mode = 01, uint8_t param = 00);
	void ProcessResponse(const std::string &request, const std::string &response);
};

#endif /* TOOLS_VIEWER_PLUGINS_YOOBD2PLUGIN_H_ */
