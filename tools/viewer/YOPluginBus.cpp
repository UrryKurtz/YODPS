/*
 * YOPluginBus.cpp
 *
 *  Created on: Sep 23, 2025
 *      Author: kurtz
 */

#include <Urho3D/SystemUI/SystemUI.h>
#include <Urho3D/SystemUI/SystemUIEvents.h> // E_SYSTEMUI
#include <Urho3D/SystemUI/ImGui.h>
#include "YOXML.h"
#include "YOPluginBus.h"
#include "IPlugin.h"

void IPlugin::Transmit(const std::string &topic, const uint8_t* data, size_t size)
{
	bus_->Transmit(this, topic, data, size);
}

void IPlugin::Transmit(const std::string &topic, YOMessage &message)
{
	bus_->Transmit(this, topic, message);
}

void IPlugin::Transmit(const std::string &topic, const YOVariant &data)
{
	bus_->Transmit(this, topic, data);
}

void IPlugin::TransmitSys(const std::string &topic, const YOVariant &data)
{
	YOMessage msg(data);
	bus_->TransmitSys(this, topic, msg);
}

void IPlugin::TransmitSys(const std::string &topic, YOMessage &data)
{
	bus_->TransmitSys(this, topic, data);
}

void IPlugin::SubscribeSys(const std::string &topic)
{
	bus_->SubscribeSys(this, topic);
}

void IPlugin::UnsubscribeSys(const std::string &topic)
{
	bus_->UnsubscribeSys(this, topic);
}

void IPlugin::Subscribe(const std::string &topic)
{
	bus_->Subscribe(this, topic);
}

void IPlugin::Unsubscribe(const std::string &topic)
{
	bus_->Unsubscribe(this, topic);
}

void IPlugin::Advertise(const std::string &topic)
{
	bus_->Advertise(this, topic);
}

void IPlugin::Unadvertise(const std::string &topic)
{
	bus_->Unadvertise(this, topic);
}

int fn(const std::string &topic, std::shared_ptr<YOMessage> message, void *param)
{
	YOPluginInfo *pi = (YOPluginInfo *) param;
	pi->plugin->OnData(topic, message);
	return 0;
}

int fn_sys(const std::string &topic, std::shared_ptr<YOMessage> message, void *param)
{
	YOPluginInfo *pi = (YOPluginInfo *) param;
	pi->plugin->OnSystem(topic, message);
	return 0;
}

void *fn_thread(void *param)
{
	YOPluginInfo *pi = (YOPluginInfo *) param;
	pi->yonode->subscribeSysFn(fn_sys, param);
	pi->yonode->start(); //blocking
	pi->yonode->shutdown();
    return  param;
}

YOPluginBus::YOPluginBus(Context *context) : Object(context)
{

}

YOPluginBus::~YOPluginBus()
{
	for( auto &pi : plugins_)
	{
		pthread_join(pi.second.thread, NULL);
	}
}

void YOPluginBus::Transmit(IPlugin* self, const std::string &topic, const uint8_t* data, size_t size)
{

	YOPluginInfo &p_info = plugins_[self->GetName()];
	YOMessage msg;
	msg.initData(data, size);
	p_info.yonode->sendMessage(topic.c_str(), msg);

}

void YOPluginBus::Transmit(IPlugin* self, const std::string &topic, YOMessage &message)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	p_info.yonode->sendMessage(topic.c_str(), message);
}

void YOPluginBus::TransmitSys(IPlugin* self, const std::string &topic, YOMessage &message)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	p_info.yonode->sendMessageSys(topic.c_str(), message);
}

void YOPluginBus::TransmitSys(IPlugin* self, const std::string &topic, const uint8_t* data, size_t size)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	YOMessage msg;
	msg.initData(data, size);
	p_info.yonode->sendMessageSys(topic.c_str(), msg);
}

void YOPluginBus::Transmit(IPlugin* self, const std::string &topic, const YOVariant &data)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	YOMessage msg(data);
	p_info.yonode->sendMessage(topic.c_str(), msg);
}


void YOPluginBus::Subscribe(IPlugin* self, const std::string &topic)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	p_info.yonode->subscribe(topic.c_str(), fn, &p_info);
}

void YOPluginBus::Unsubscribe(IPlugin* self, const std::string &topic)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	p_info.yonode->unsubscribe(topic.c_str());
}

void YOPluginBus::Advertise(IPlugin* self, const std::string &topic)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	p_info.yonode->advertise(topic.c_str(), 0, 0);
}

void YOPluginBus::Unadvertise(IPlugin* self, const std::string &topic)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	p_info.yonode->unadvertise(topic.c_str());
}

void YOPluginBus::SubscribeSys(IPlugin* self, const std::string &topic)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	p_info.yonode->subscribeSys(topic.c_str());
}

void YOPluginBus::UnsubscribeSys(IPlugin* self, const std::string &topic)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	p_info.yonode->unsubscribeSys(topic.c_str());
}



void YOPluginBus::SetConfig(YOVariant *config)
{
	config_ = config;
	std::cout << "YOPluginBus::SetConfig " << config_->m_name << std::endl;
}

void YOPluginBus::AddPlugin(const std::string &name, IPlugin* plugin)
{
	std::cout << "YOPluginBus::AddPlugin " << name << std::endl;
	plugins_[name].name = name;
	plugin->SetBus(this);
	plugin->SetName(name);
	plugins_[name].plugin = plugin;
	plugins_[name].config = new YOVariant(name);

	YOVariant &cur_cfg = config_->get(name);
	if(!cur_cfg.hasChild(yo::k::enabled))
	{
		cur_cfg[yo::k::enabled] = false;
	}

	if(cur_cfg.hasChild(yo::k::file))
	{
		std::string &file = cur_cfg[yo::k::file];
		std::cout << "FILE: " << file << std::endl;
		plugins_[name].file = file;
		YOXML xml;
		if(xml.readXML(plugins_[name].file, *plugins_[name].config))
		{
			std::cout << "LOADED CONFIG " << file << " PLUGIN " <<  name  << " [" << plugins_[name].config->m_name << "] " <<  std::endl;
		}
		else
		{
			std::cout << "ERROR LOADING CONFIG " << file << " PLUGIN "<<  name  << " [" << plugins_[name].config->m_name << "] " << std::endl;
		}
	}
	else
	{
		std::string file =  name + ".xml";
		cur_cfg[yo::k::file] = file.c_str();
		plugins_[name].file = file;
	}
}

void YOPluginBus::OnStop()
{
	for( auto &pi : plugins_)
	{
		pi.second.plugin->OnStop();
		YOXML xml;
		xml.writeXML(pi.second.file, *pi.second.config);
		std::cout << "SAVED CONFIG [" << pi.second.file << "] PLUGIN "<<  pi.first  << std::endl;
	}
}

void YOPluginBus::OnStart(Scene *scene)
{
	std::cout << "YOPluginBus::OnStart " << std::endl;
	for( auto &pi : plugins_)
	{
		std::cout << "OnStart plugin " << pi.first << " " << scene << " " << pi.first << std::endl;
		pi.second.yonode = new YONode(pi.first.c_str());
		pi.second.node = scene->CreateChild(pi.first.c_str());
		pi.second.plugin->SetNode(pi.second.node);
		pi.second.plugin->SetConfig(pi.second.config);
		pi.second.plugin->OnStart();
	    pthread_create(&pi.second.thread, NULL, fn_thread, &pi.second);
	}
}

void YOPluginBus::OnUpdate(float timeStep)
{
	for( auto &pi : plugins_)
	{
		pi.second.plugin->OnUpdate(timeStep);
	}
}

void YOPluginBus::OnGui()
{
	int i = 0;
	for( auto &pi : plugins_)
	{
		bool &show_gui = (*config_)[pi.second.name][yo::k::enabled].getBool();
		if(show_gui)
		{
			ui::SetNextWindowSize(ImVec2(550, 500), ImGuiCond_FirstUseEver);
		    ui::SetNextWindowPos(ImVec2(250 + ++i * 50, 50 + i * 50), ImGuiCond_FirstUseEver);
			if(ui::Begin(pi.first.c_str(), &show_gui, ImGuiWindowFlags_NoCollapse))
			{
				pi.second.plugin->OnGui();
				ui::End();
			}
		}
	}
}

void YOPluginBus::OnMenu()
{
	for( auto &pi : plugins_)
	{
		bool &show_gui = (*config_)[pi.second.name][yo::k::enabled].getBool();
		if (ui::MenuItem(pi.first.c_str(), nullptr, show_gui))
		{
			show_gui = !show_gui;
		}
	}
}
