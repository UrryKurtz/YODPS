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
	Subscribe( (*config_)[yo::k::data][yo::k::input][yo::k::topic].getStr() );
	Advertise( (*config_)[yo::k::data][yo::k::output][yo::k::topic].getStr() );

	if(!config_->hasChild(yo::k::system))
	{
		(*config_)[yo::k::system][yo::k::input][yo::k::topic] = name_.c_str();
		(*config_)[yo::k::system][yo::k::input][yo::k::value] = "";
		(*config_)[yo::k::system][yo::k::output][yo::k::topic] = name_.c_str();
		(*config_)[yo::k::system][yo::k::output][yo::k::value] = "";
	}

	data_cfg_[yo::k::input][yo::k::topic] = "";
	data_cfg_[yo::k::input][yo::k::value] = "";
	data_cfg_[yo::k::output][yo::k::topic] = "";
	data_cfg_[yo::k::output][yo::k::value] = "";
	data_cfg_old_ = data_cfg_;

	sys_cfg_= data_cfg_;
	sys_cfg_.m_name = "System";
	sys_cfg_old_ = sys_cfg_;

}

void YOTestPlugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::string data((const char*)message->getData(), message->getDataSize());
	std::cout << data << std::endl;
	data_cfg_[yo::k::input][yo::k::value] = data.c_str();
}

void YOTestPlugin::OnSystem(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::string data((const char*)message->getData(), message->getDataSize());
	std::cout << data << std::endl;
	sys_cfg_[yo::k::input][yo::k::value] = data.c_str();
}

void YOTestPlugin::OnGui()
{
	ui::SeparatorText("Data Topics");
	if(gui_.draw(data_cfg_))
	{
		if(gui_.getPath() == "/Data/input/topic")
		{
			Unsubscribe(data_cfg_old_[yo::k::input][yo::k::topic].getStr());
			Subscribe(data_cfg_[yo::k::input][yo::k::topic].getStr());
		}
		if(gui_.getPath() == "/Data/output/topic")
		{
			Unadvertise(data_cfg_old_[yo::k::output][yo::k::topic].getStr());
			Advertise(data_cfg_[yo::k::output][yo::k::topic].getStr());
		}
		data_cfg_old_ = data_cfg_;
	}

	if(ui::Button("Send Data"))
	{
		std::string val = data_cfg_[yo::k::output][yo::k::value].getStr();

		std::cout << "SEND " << val << std::endl;
		Transmit(data_cfg_[yo::k::output][yo::k::topic].getStr(), (const uint8_t*)val.c_str(), val.size());
	}

	ui::SeparatorText("System Topics");

	if(gui_.draw(sys_cfg_))
	{
		std::cout << "SEND " << gui_.getPath() << std::endl;
		if(gui_.getPath() == "/System/input/topic")
		{
			std::cout << "CHANGE " << gui_.getPath() << std::endl;
			UnsubscribeSys(sys_cfg_old_[yo::k::input][yo::k::topic].getStr());
			SubscribeSys(sys_cfg_[yo::k::input][yo::k::topic].getStr());
		}
		sys_cfg_old_ = sys_cfg_;
	}

	if(ui::Button("Send System"))
	{
		std::string val = sys_cfg_[yo::k::output][yo::k::value].getStr();

		std::cout << "SEND " << val << std::endl;
		YOMessage msg;
		msg.initData((const uint8_t*)val.data(), val.size());

		TransmitSys(sys_cfg_[yo::k::output][yo::k::topic].getStr(), msg);
	}
}
