/*
 * YOOBD2Plugin.cpp
 *
 *  Created on: Oct 6, 2025
 *      Author: kurtz
 */
#include "YOOBD2Plugin.h"
#include "YONode.h"
#include "YOXML.h"

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

	ads_.push_back((*config_)[yo::k::settings][yo::k::advertise].getStr());
	subs_.push_back((*config_)[yo::k::settings][yo::k::subscribe].getStr());
}

inline void split_lines(const std::string& s, std::vector<std::string>& out) {
    size_t i=0;
    while (i<s.size()) {
        size_t j=i;
        while (j<s.size() && s[j]!='\n' && s[j]!='\r' && s[j]!='>') ++j;
        if (j>i) out.emplace_back(s.substr(i,j-i));
        // skip \r\n and '>'
        while (j<s.size() && (s[j]=='\n'||s[j]=='\r'||s[j]=='>')) ++j;
        i=j;
    }
}

void YOOBD2Plugin::ProcessResponse(const std::string &request, const std::string &response)
{
	if(request.substr(0,2) == "01") //request mode 01
	{
		std::cout << "REQUEST :: 01 " << std::endl;
		std::vector<std::string> lines;
		split_lines(response, lines);
		for(auto &line : lines)
		{
			std::cout << line << std::endl;
			size_t start = line.find("41");
			if(start != std::string::npos)
			{
				uint8_t mode = std::stoi(line.substr(start, 2), nullptr, 16);
				uint8_t pid = std::stoi(line.substr(start+2, 2), nullptr, 16);
				printf("GOT RESPONSE: MODE %02X PID %02X ", mode, pid);
				uint8_t data[64];
				int x = 0;
				for( int i = start + 4; i < line.length(); i+=2)
				{
					uint8_t cur = std::stoi(line.substr(i, 2), nullptr, 16);
					data[x++] = cur;
					printf(" %02X ", cur);
				}
				printf("\n ");
				break;
			}
		}
	}
}

void YOOBD2Plugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	//std::cout << topic << std::endl;
	mutex_.lock();
	ts_ = YONode::getTimestamp();
	response_ = std::make_shared<YOVariant>(message->getDataSize(), (const char*)message->getData());

	for(auto &msg : response_->get<YOArray>())
	{
		//std::cout << msg[yo::k::request].m_value << std::endl;
		//std::cout << "!!![" << msg[yo::k::response].m_value << "]!!!" << std::endl;
		ProcessResponse(msg[yo::k::request].getStr(), msg[yo::k::response].getStr());
	}
	mutex_.unlock();
}

void YOOBD2Plugin::Poll(uint8_t mode, uint8_t param)
{
	YOVariant req(yo::k::requests, YOArray());
	YOVariant msg(yo::k::message);
	char buf[64];
	sprintf(buf, "%02X%02X", mode, param);
	msg[yo::k::request] = buf;
	msg[yo::k::id] = (uint32_t) req.getArraySize();
	req.push_back(msg);
	Transmit((*settings_)[yo::k::advertise].getStr(), req);
}

void YOOBD2Plugin::OnGui()
{
	gui_.draw(*requests_);

	if(ui::Button("Add"))
	{
		YOVariant msg(yo::k::message);
		msg[yo::k::request] = " ";
		msg[yo::k::id] = (uint32_t)requests_->getArraySize();
		requests_->push_back(msg);
	}

	if(ui::Button("Send"))
	{
		Transmit((*settings_)[yo::k::advertise].getStr(), *requests_);
	}

	if(ui::Button("Poll 01 00"))
	{
		Poll(0x01, 0x00);
	}

	ui::Begin("Data");
	mutex_.lock();
	if(response_)
	{
		ui::Text("Timestamp: %llu", ts_);
		for( auto &msg : response_->get<YOArray>())
		{
			uint32_t id = msg[yo::k::id];
			std::string &req = msg[yo::k::request];
			std::string &resp = msg[yo::k::response];
			ui::Separator();
			ui::Text("Message [%u]\nRequest [%s]\nResponse [%s]", id, req.c_str(), resp.c_str() );
		}

	}
	mutex_.unlock();
	ui::End();

}
