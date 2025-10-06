/*
 * YOOBD2Plugin.cpp
 *
 *  Created on: Oct 6, 2025
 *      Author: kurtz
 */
#include "YOOBD2Plugin.h"
#include "YONode.h"

YOOBD2Plugin::YOOBD2Plugin(Context *context) : IPlugin(context)
{
	// TODO Auto-generated constructor stub

}

YOOBD2Plugin::~YOOBD2Plugin()
{
	// TODO Auto-generated destructor stub
}

void YOOBD2Plugin::OnStart()
{
	if(!config_->hasChild(yo::k::requests))
		(*config_)[yo::k::requests] = YOArray();

	if(!config_->hasChild(yo::k::settings))
	{
		(*config_)[yo::k::settings] = YOMap();
		(*config_)[yo::k::settings][yo::k::subscribe] = "SERIAL_RESPONSE";
		(*config_)[yo::k::settings][yo::k::advertise] = "SERIAL_REQUEST";

	}
	requests_ = &(*config_)[yo::k::requests];
	settings_ = &(*config_)[yo::k::settings];

	ads_.push_back((*config_)[yo::k::settings][yo::k::advertise].get<std::string>());
	subs_.push_back((*config_)[yo::k::settings][yo::k::subscribe].get<std::string>());
}

void YOOBD2Plugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::cout << topic << std::endl;
	mutex_.lock();
	ts_ = YONode::getTimestamp();
	response_ = std::make_shared<YOVariant>(message->getDataSize(), (const char*)message->getData());
	std::cout << *response_ << std::endl;
	mutex_.unlock();
}

void YOOBD2Plugin::OnGui()
{
	ui::Begin(name_.c_str());
	gui_.drawCfg(*requests_);

	if(ui::Button("Add"))
	{
		YOVariant msg(yo::k::message);
		msg[yo::k::request] = " ";
		msg[yo::k::id] = (uint32_t)requests_->getArraySize();
		requests_->push_back(msg);
	}

	if(ui::Button("Send"))
	{
		Transmit("SERIAL_REQUEST", *requests_);
	}
	ui::End();

	ui::Begin("Data");
	mutex_.lock();
	if(response_)
	{
		ui::Text("Timestamp: %llu", ts_);
		for( auto &msg : response_->get<YOArray>())
		{
			uint32_t id = msg[yo::k::id];
			std::string &req = msg[yo::k::request].get<std::string>();
			std::string &resp = msg[yo::k::response].get<std::string>();
			ui::Separator();
			ui::Text("Message [%u]\nRequest [%s]\nResponse [%s]", id, req.c_str(), resp.c_str() );
		}

	}
	mutex_.unlock();
	ui::End();

}
