/*
 * YORecorderPlugin.cpp
 *
 *  Created on: Oct 17, 2025
 *      Author: kurtz
 */
#include <vector>
#include <string>
#include <algorithm>
#include "YORecorderPlugin.h"
namespace yo::k
{
	YO_KEY(connect, "cnc")
	YO_KEY(recorder, "rcr")
}

YORecorderPlugin::YORecorderPlugin(Context *context) : IPlugin(context)
{
	// TODO Auto-generated constructor stub

}

YORecorderPlugin::~YORecorderPlugin()
{
	// TODO Auto-generated destructor stub
}

void YORecorderPlugin::OnStart()
{
	if(config_->hasChild(yo::k::directory))
		(*config_)[yo::k::directory] = "Recorder";

	recorder_ = &(*config_)[yo::k::settings][yo::k::recorder];
	(*recorder_)[yo::k::system][yo::k::publish] = YO_SUB_SYS_SRV;
	(*recorder_)[yo::k::system][yo::k::subscribe] = YO_SUB_SYS_SRV;

	cfg_ = &(*config_)[yo::k::settings][yo::k::config];

	(*cfg_)[yo::k::brokers] = YOArray();
	YOVariant broker0(yo::k::broker);
	broker0[yo::k::comment] = "Local broker";
	broker0[yo::k::connect] = "tcp://127.0.0.1:5550";
	broker0[yo::k::streams] =  YOArray();

	YOVariant stream0(yo::k::stream);
	stream0[yo::k::comment] = "TEST COMMENT CAN";
	stream0[yo::k::name] = "OBD2";
	stream0[yo::k::type] = (uint16_t) 512;
	stream0[yo::k::subtype] =  (uint16_t) 0;

	YOVariant stream1(yo::k::stream);
	stream1[yo::k::comment] = "TEST COMMENT CAN0";
	stream1[yo::k::name] = "CAN0";
	stream1[yo::k::type] = (uint16_t) 512;
	stream1[yo::k::subtype] =  (uint16_t) 0;

	broker0[yo::k::streams].getArray().push_back(stream0);
	broker0[yo::k::streams].getArray().push_back(stream1);

	(*cfg_)[yo::k::brokers].getArray().push_back(broker0);

}

void YORecorderPlugin::OnGui()
{
	ui::SeparatorText("Recorder System Ports");
	gui_.draw(*recorder_);
	ui::SeparatorText("Recorder Config");
	gui_.draw(*cfg_);

	YOStringList &list = tmp_brokers_.get<YOStringList>();

	if(gui_.draw(tmp_broker_))
	{
		std::cout << tmp_broker_.getStr() << std::endl;


		if(std::find(list.items.begin(), list.items.end(), tmp_broker_.getStr()) == list.items.end())
		{
			list.items.push_back(tmp_broker_.getStr());
			list.select = list.items.size() - 1;
		}
	}

	if(gui_.draw(tmp_brokers_))
	{
		std::cout << tmp_brokers_<< std::endl;
	}

/*
	ui::SameLine();
	if(ui::Button("Delete"))
	{
		if(list.items.size())
			list.items.erase(list.items.begin() + list.select);
	}
*/

	if(gui_.draw(tmp_stream_))
	{
		std::cout << tmp_stream_.getStr() << std::endl;
	}

	if(gui_.draw(tmp_type_))
	{
		std::cout << tmp_type_.get<uint16_t>() << std::endl;
	}

	if(gui_.draw(tmp_subtype_))
	{
		std::cout << tmp_subtype_.get<uint16_t>() << std::endl;
	}

}
