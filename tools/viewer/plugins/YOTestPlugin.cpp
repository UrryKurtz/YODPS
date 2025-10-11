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
	// TODO Auto-generated constructor stub

}

YOTestPlugin::~YOTestPlugin()
{
	// TODO Auto-generated destructor stub
}

void YOTestPlugin::OnStart()
{
	if(!config_->hasChild(yo::k::input))
	{
		(*config_)[yo::k::input][yo::k::topic] = "INPUT";
		(*config_)[yo::k::input][yo::k::value] = "";
	}

	if(!config_->hasChild(yo::k::output))
	{
		(*config_)[yo::k::output][yo::k::topic] = "OUTPUT";
		(*config_)[yo::k::output][yo::k::value] = "";
	}

	subs_.push_back( (*config_)[yo::k::input][yo::k::topic].getStr() );
	ads_.push_back( (*config_)[yo::k::output][yo::k::topic].getStr() );
}

void YOTestPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::string data((const char*)message->getData(), message->getDataSize());
	//std::cout << "OnData "  << topic  << " [" << data << "]" << std::endl;
	(*config_)[yo::k::input][yo::k::value] = data.c_str();
}

void YOTestPlugin::OnGui()
{
	gui_.draw(*config_);
	if(ui::Button("Send"))
	{
		std::string &topic = (*config_)[yo::k::output][yo::k::topic].getStr() ;
		std::string &out = (*config_)[yo::k::output][yo::k::value].getStr() ;
		//std::cout << "SEND !! " << out << std::endl;
		Transmit(topic, (const uint8_t*) out.data(), out.size());
	}
}
