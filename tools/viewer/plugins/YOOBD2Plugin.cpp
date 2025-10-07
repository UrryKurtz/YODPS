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

#include <tinyxml2.h>
using namespace tinyxml2;



void readSignal(tinyxml2::XMLElement* signal, YOVariant &cut_cfg)
{
	 std::string s_id = signal->Attribute("id");
   	 std::string name = signal->Attribute("name");
   	 std::string bytes = signal->Attribute("bytes");
   	 std::string offset = signal->Attribute("offset");
   	 std::string mul = signal->Attribute("mul");
   	 std::string div = signal->Attribute("div");
   	 std::string unit = signal->Attribute("unit");
   	 std::string mask = signal->Attribute("mask");
   	 std::string type = signal->Attribute("type");
   	 int offset_i = std::stoi(offset.c_str());
   	 int mul_i = std::stoi(mul.c_str());
   	 int div_i = std::stoi(div.c_str());

   	 std::cout << "SIGNAL: "
   			 << " name: [" << name << "] "
				 << " bytes: [" << bytes << "] "
				 << " offset: [" << offset << "] " << offset_i
				 << " div: [" << div << "] "
				 << " mul: [" << mul << "] "
				 << " unit: [" << unit << "] "
				 << std::endl;
   	 cut_cfg[s_id]["name"] = name.c_str();
   	 cut_cfg[s_id]["bytes"] = bytes.c_str();
   	 cut_cfg[s_id]["offset"] = offset_i;
   	 cut_cfg[s_id]["div"] = div_i;
   	 cut_cfg[s_id]["mul"] = mul_i;
   	 cut_cfg[s_id]["unit"] = unit.c_str();
   	 cut_cfg[s_id]["mask"] = mask.c_str();
   	 cut_cfg[s_id]["type"] = type.c_str();
}


void readXML(YOVariant &OBD2)
{
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError err = doc.LoadFile("obd2.xml");
  tinyxml2::XMLElement* root = doc.RootElement();
  std::cout << "READ FILE. ROOT: " << root->Name() << " " <<  root->FirstChildElement("pid") << std::endl;

     for (tinyxml2::XMLElement* pid = root->FirstChildElement("pid"); pid != nullptr; pid = pid->NextSiblingElement("pid"))
     {
    	 std::string p_id = pid->Attribute("id");
    	 int p_size = pid->Int64Attribute("size");
    	 YOVariant &cur_cfg = OBD2[p_id];
    	 OBD2[p_id][yo::k::size] = p_size;

    	 for (tinyxml2::XMLElement* signal = pid->FirstChildElement("signal"); signal != nullptr; signal = signal->NextSiblingElement("signal"))
    	 {
    		 readSignal(signal, cur_cfg["SIGNALS"]);
    	 }

    	 for (tinyxml2::XMLElement* group = pid->FirstChildElement("group"); group != nullptr; group = group->NextSiblingElement("group"))
    	 {
        	 std::string g_id = group->Attribute("name");
    		 YOVariant &group_cfg = cur_cfg["GROUPS"][g_id];
        	 for (tinyxml2::XMLElement* signal = group->FirstChildElement("signal"); signal != nullptr; signal = signal->NextSiblingElement("signal"))
        	 {
        		 readSignal(signal, group_cfg);
        	 }
    	 }
     }

     YOXML xml;
     xml.writeXML("obd2_pids.xml", OBD2);
}

void YOOBD2Plugin::OnStart()
{
	OBD2_ = new YOVariant("OBD2");
	readXML(*OBD2_);

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


void YOOBD2Plugin::ProcessResponse(const std::string &request, const std::string &response)
{
	if(request.substr(0,2) == "01")
	{
		std::cout << "!!!! :: 01 " <<  request << std::endl;
	}
}

void YOOBD2Plugin::OnData(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	std::cout << topic << std::endl;
	mutex_.lock();
	ts_ = YONode::getTimestamp();
	response_ = std::make_shared<YOVariant>(message->getDataSize(), (const char*)message->getData());
	for(auto &msg : response_->get<YOArray>())
	{
		std::cout << msg[yo::k::request].m_value << std::endl;
		std::cout << "!!![" << msg[yo::k::response].m_value << "]!!!" << std::endl;
		ProcessResponse(msg[yo::k::request].get<std::string>(), msg[yo::k::response].get<std::string>());
	}

	std::cout << *response_ << std::endl;
	mutex_.unlock();
}

void YOOBD2Plugin::Poll(int mode)
{
	YOVariant req(yo::k::requests);
	YOVariant msg(yo::k::message);
	char buf[64];
	sprintf(buf, "%02X00", mode);
	msg[yo::k::request] = buf;
	msg[yo::k::id] = (uint32_t) requests_->getArraySize();
	req.push_back(msg);
	Transmit((*settings_)[yo::k::advertise].get<std::string>().c_str(), req);
}


void YOOBD2Plugin::OnGui()
{

	ui::Begin("OBD2");
	if(OBD2_)
		gui_.drawCfg(*OBD2_);
	ui::End();

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
		Transmit((*settings_)[yo::k::advertise].get<std::string>().c_str(), *requests_);
	}

	if(ui::Button("Poll 01"))
	{
		Poll(0x01);
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
