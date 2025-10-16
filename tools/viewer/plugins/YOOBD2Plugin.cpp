/*
 * YOOBD2Plugin.cpp
 *
 *  Created on: Oct 6, 2025
 *      Author: kurtz
 */
#include "YOOBD2Plugin.h"
#include "YONode.h"
#include "YOXML.h"

namespace yo::k
{
	YO_KEY(can,   "can")
	YO_KEY(init,  "ini")
}


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

	if(!config_->hasChild(yo::k::init))
		(*config_)[yo::k::init] = YOArray();

	if(!config_->hasChild(yo::k::response))
		(*config_)[yo::k::response] = YOArray();

	if(!config_->hasChild(yo::k::settings))
	{
		(*config_)[yo::k::settings] = YOMap();
		(*config_)[yo::k::settings][yo::k::subscribe] = "SERIAL_RESPONSE";
		(*config_)[yo::k::settings][yo::k::advertise] = "SERIAL_REQUEST";
		(*config_)[yo::k::settings][yo::k::can] = "OBD2";
	}

	(*config_)[yo::k::settings][yo::k::can] = "OBD2";

	init_ = &(*config_)[yo::k::init];
	requests_ = &(*config_)[yo::k::requests];
	settings_ = &(*config_)[yo::k::settings];

	Advertise((*config_)[yo::k::settings][yo::k::advertise].getStr());
	Advertise((*config_)[yo::k::settings][yo::k::can].getStr());
	Subscribe((*config_)[yo::k::settings][yo::k::subscribe].getStr());
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

void YOOBD2Plugin::ProcessPIDEnable(uint8_t mode, uint8_t pid, const std::string &line)
{
	printf("ProcessPIDEnable MODE: %02X PID: %02X   %s", mode, pid, line.c_str());
	for(int i = 0; i < 8; i++)
	{
		uint8_t cur = std::stoi(line.substr(i, 2), nullptr, 16);
		printf("%02X :", cur);
		for(int b = 0; b < 8 ; b++)
		{
			printf("   %02X : %d\n",  pid + i * b,  (cur >> b) & 0x1u );
		}
	}
}

void YOOBD2Plugin::RequestLine(int &id, YOVariant &list, bool loop)
{
	if(id < (int) list.getArraySize())
	{	id++;
		if(id == list.getArraySize())
		{
			std::cout << " RECEIVED LAST: " << id << std::endl;
			id = -1;

			if(loop)
				RequestLine(id, list);

			return;
		}
		std::string &send = list[id][yo::k::request].getStr();
		std::cout << " Request : " << id << " " << send << std::endl;
		Transmit((*settings_)[yo::k::advertise].getStr(), (const uint8_t*)send.data(), send.size());
	}
	else
	{
		id = -1;
	}
}

void YOOBD2Plugin::ProcessResponse(const std::string &response)
{
	std::vector<std::string> lines;
	split_lines(response, lines);

	for(auto &line : lines)
	{
		std::cout << line << std::endl;
		if(line[0] == '4')
		{
			uint8_t mode = std::stoi(line.substr(1, 1), nullptr, 16);
			uint8_t pid = std::stoi(line.substr(2, 2), nullptr, 16);
			tCANFDData can {0};
			can.sData.ui32Id = pid;
			can.sData.ui8Length = line.size()/2 - 2;

			printf("GOT RESPONSE: MODE %02X PID %02X ", mode, pid);
			int x = 0;
			for( int i = 4; i < line.length(); i+=2)
			{
				uint8_t cur = std::stoi(line.substr(i, 2), nullptr, 16);
				can.sData.aui8Data[x++] = cur;
				printf(" %02X ", cur);
			}

			YOMessage msg(can);
			Transmit("OBD2", msg);
			printf("\n ");

			if( !(pid % 32) && poll_ && pid < 0xC0)
			{
				ProcessPIDEnable(mode , pid, line);
				printf("POLL AGAIN %02X%02X \n", mode, pid + 32);
				Poll(mode, pid + 32);
			}
			break;
		}
	}
}

void YOOBD2Plugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::string data((const char*)message->getData(), message->getDataSize());
	//std::cout << topic << " " << data << std::endl;
	mutex_.lock();
	response_.push_front({message->getTimestamp(), data});
	if(response_.size() > 50)
		response_.pop_back();

	if(go_init_ >= 0)
	{
		std::cout << go_init_<< " RCV INIT " << init_->getArraySize() << std::endl;
		RequestLine(go_init_, *init_);
	}

	if(go_request_>= 0)
	{
		std::cout << go_request_<< " RCV REQUEST " << requests_->getArraySize() << std::endl;
		RequestLine(go_request_, *requests_, loop_);
	}

	ts_ = message->getTimestamp();
	ProcessResponse(data);
	mutex_.unlock();
}

void YOOBD2Plugin::Poll(const uint8_t &mode, const uint8_t &param)
{
	poll_ = true;
	char buf[64];
	sprintf(buf, "%02X%02X", mode, param);
	Transmit((*settings_)[yo::k::advertise].getStr(), (const uint8_t*) buf, 4);
}

void YOOBD2Plugin::OnGui()
{
	ui::SeparatorText("Settings");
	gui_.draw(*settings_);
	ui::SeparatorText("Init");
	gui_.draw(*init_);
	ui::BeginDisabled(go_init_ > -1);
	if(ui::Button("Init"))
	{
		RequestLine(go_init_, *init_);
	}
	ui::EndDisabled();
	ui::SameLine();
	if(ui::Button("Add init line"))
	{
		YOVariant msg(yo::k::message);
		msg[yo::k::request] = "";
		msg[yo::k::id] = (uint32_t)init_->getArraySize();
		init_->push_back(msg);
	}

	ui::SeparatorText("Request");
	gui_.draw(*requests_);
	ui::BeginDisabled(go_request_ > -1);
	if(ui::Button("Request"))
	{
		RequestLine(go_request_, *requests_);
	}
	ui::EndDisabled();
	ui::SameLine();
	ui::Checkbox("Loop", &loop_);
	ui::SameLine();
	if(ui::Button("Add request line"))
	{
		YOVariant msg(yo::k::message);
		msg[yo::k::request] = "";
		msg[yo::k::id] = (uint32_t)requests_->getArraySize();
		requests_->push_back(msg);
	}

	ui::SeparatorText("Poll");

	if(ui::Button("Poll 01 00"))
	{
		Poll(0x01, 0x00);
	}

	mutex_.lock();

	ui::SeparatorText("Log");

	for( auto &msg : response_)
	{
		ui::Separator();
		ui::Text("%llu : [%s]", msg.ts, msg.msg.c_str() );
	}
	mutex_.unlock();

}
