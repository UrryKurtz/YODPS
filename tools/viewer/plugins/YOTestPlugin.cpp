/*
 * YOTestPlugin.cpp
 *
 *  Created on: Oct 11, 2025
 *      Author: kurtz
 */

#include "YOTestPlugin.h"

namespace yo::k {
    YO_KEY(output,    "out")
}

YOTestPlugin::YOTestPlugin(Context *context) : IPlugin(context)
{

}

YOTestPlugin::~YOTestPlugin()
{

}

void YOTestPlugin::OnStart()
{
	if(!config_->hasChild(yo::k::data))
	{
		(*config_)[yo::k::data][yo::k::input][yo::k::topic] = "INPUT";
		(*config_)[yo::k::data][yo::k::input][yo::k::value] = "";
		(*config_)[yo::k::data][yo::k::output][yo::k::topic] = "OUTPUT";
		(*config_)[yo::k::data][yo::k::output][yo::k::value] = "";
	}
	subs_.push_back( (*config_)[yo::k::data][yo::k::input][yo::k::topic].getStr() );
	ads_.push_back( (*config_)[yo::k::data][yo::k::output][yo::k::topic].getStr() );

	if(!config_->hasChild(yo::k::system))
	{
		(*config_)[yo::k::system][yo::k::input][yo::k::topic] = name_.c_str();
		(*config_)[yo::k::system][yo::k::input][yo::k::value] = "";
		(*config_)[yo::k::system][yo::k::output][yo::k::topic] = name_.c_str();
		(*config_)[yo::k::system][yo::k::output][yo::k::value] = "";
	}
}

void YOTestPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::string data((const char*)message->getData(), message->getDataSize());
	(*config_)[yo::k::data][yo::k::input][yo::k::value] = data.c_str();
}

void YOTestPlugin::OnSystem(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::string data((const char*)message->getData(), message->getDataSize());
	(*config_)[yo::k::system][yo::k::input][yo::k::value] = data.c_str();
}

void YOTestPlugin::OnGui()
{
	gui_.draw((*config_)[yo::k::data]);
	if(ui::Button("Send Data"))
	{
		std::string &topic = (*config_)[yo::k::data][yo::k::output][yo::k::topic].getStr() ;
		std::string &out = (*config_)[yo::k::data][yo::k::output][yo::k::value].getStr() ;
		//std::cout << "SEND !! " << out << std::endl;
		Transmit(topic, (const uint8_t*) out.data(), out.size());
	}
	gui_.draw((*config_)[yo::k::system]);
	if(ui::Button("Send System"))
	{
		std::string &topic = (*config_)[yo::k::system][yo::k::output][yo::k::topic].getStr() ;
		std::string &out = (*config_)[yo::k::system][yo::k::output][yo::k::value].getStr() ;
		YOMessage msg;
		msg.initData((const uint8_t*)out.c_str(), out.size());
		TransmitSys(topic, msg);
	}
}
