/*
 * YORecorderPlugin.h
 *
 *  Created on: Oct 17, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YORECORDERPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YORECORDERPLUGIN_H_
#include "IPlugin.h"

class YORecorderPlugin : public IPlugin
{

	YOVariant *recorder_;
	YOVariant *cfg_;

	YOGui gui_;
	YOVariant tmp_brokers_ {"Brokers", YOStringList()};

	YOVariant tmp_broker_ {"Connection", ""};
	YOVariant tmp_broker_cmt_ {"Comment", ""};

	YOVariant tmp_stream_ {"##tmp_stream", ""};
	YOVariant tmp_type_ { "##tmp_type", (uint16_t)0};
	YOVariant tmp_subtype_ { "##tmp_subtype", (uint16_t)0};

public:
	YORecorderPlugin(Context *context);
	virtual ~YORecorderPlugin();

	void OnStart() override;
	void OnGui() override;
};

#endif /* TOOLS_VIEWER_PLUGINS_YORECORDERPLUGIN_H_ */
