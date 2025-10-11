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
struct YOMsg {
	YOTimestamp ts;
	std::string msg;
};

class YOOBD2Plugin: public IPlugin
{
	YOVariant enabled_ ;
	YOVariant *init_ {nullptr};
	YOVariant *requests_ {nullptr};
	YOVariant *settings_ {nullptr};
	std::deque<YOMsg> response_;

	std::map<int, std::vector<uint8_t>> data_;

	int go_init_{-1};
	int go_request_{-1};

	std::mutex mutex_;
	YOGui gui_;
	YOTimestamp ts_{0};
	bool poll_{false};
	bool loop_{false};

public:
	YOOBD2Plugin(Context *context);
	virtual ~YOOBD2Plugin();

	void OnStart() override;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnGui() override;

	void RequestLine(int &id, YOVariant &list, bool loop = false);
	void Poll(const uint8_t &mode, const uint8_t &param);
	void ProcessResponse(const std::string &response);
	void ProcessPIDEnable(uint8_t mode, uint8_t pid, const std::string &line);

};

#endif /* TOOLS_VIEWER_PLUGINS_YOOBD2PLUGIN_H_ */
