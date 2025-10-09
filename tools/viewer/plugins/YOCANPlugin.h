/*
 * YOCANPlugin.h
 *
 *  Created on: Oct 7, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOCANPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOCANPLUGIN_H_

#include "IPlugin.h"
#include <Urho3D/Container/Str.h>
#include <libdbc/dbc.hpp>
#include <mutex>

using namespace Urho3D;

struct YOCANInfo
{
	uint32_t id;
	YOTimestamp ts;
	tCANFDData canfd;
	std::string hex;
	std::string header;
	Libdbc::Message *message;
	std::vector<Libdbc::Signal> signals;
	std::vector<double> values;
	std::vector<uint8_t> send;
	std::mutex mutex;
};

struct YOParserInfo
{
	std::shared_ptr<Libdbc::DbcParser> parser;
	std::vector<Libdbc::Message> messages;
};

class YOCANPlugin: public IPlugin
{
	//data[channel][message]
	std::map<int, std::map<uint32_t, YOCANInfo>> data_;
	std::map<std::string, YOParserInfo> parsers_;

	YOVariant dbc_;
	YOVariant *settings_ {nullptr};
	YOVariant *channels_ {nullptr};
	YOVariant *send_ {nullptr};
	char files_tmp_[8][256] {0};

	YOGui gui_;
	ea::vector<ea::string> files_;

public:
	YOCANPlugin(Context *context);
	virtual ~YOCANPlugin();

	void OnStart() override ;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override ;
	void OnGui() override ;

	void SendToPlotter(const std::string &pid, const std::string &signal, float value);
};

#endif /* TOOLS_VIEWER_PLUGINS_YOCANPLUGIN_H_ */
