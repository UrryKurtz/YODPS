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

int fn(const std::string &topic, std::shared_ptr<YOMessage> message, void *param)
{
	YOPluginInfo *pi = (YOPluginInfo *) param;
	pi->plugin->OnData(topic, message);
	return 0;
}

void *fn_thread(void *param)
{
	YOPluginInfo *pi = (YOPluginInfo *) param;

	pi->yonode = new YONode(pi->name.c_str());

	for(auto &topic : pi->adverts)
	{
		pi->yonode->advertise(topic.c_str());
	}

	for(auto &topic : pi->subs)
	{
		pi->yonode->subscribe(topic.c_str(), fn, param);
	}

	if(pi->subs.size())
	{
		pi->yonode->start(); //blocking
		pi->yonode->disconnect();
		pi->yonode->shutdown();
	}
	else
	{
		pi->yonode->connect();
	}

	//pi->yonode->addSignalFunction(SIGINT, sig_fn, &node);
	std::cout << " THREAD FINISHED !!!!!!!!!!!!" << pi->name << std::endl;
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

void YOPluginBus::Transmit(IPlugin* self, const std::string &topic, const YOVariant &data)
{
	YOPluginInfo &p_info = plugins_[self->GetName()];
	YOMessage msg(data);
	p_info.yonode->sendMessage(topic.c_str(), msg);
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
		pi.second.node = scene->CreateChild(pi.first.c_str());
		pi.second.plugin->SetNode(pi.second.node);
		pi.second.plugin->SetConfig(pi.second.config);
		pi.second.plugin->OnStart();
		pi.second.subs = pi.second.plugin->GetSubscriptions();
		pi.second.adverts = pi.second.plugin->GetAdvertisements();

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
		if(pi.second.show_gui)
		{
			ui::SetNextWindowSize(ImVec2(550, 500), ImGuiCond_FirstUseEver);
		    ui::SetNextWindowPos(ImVec2(250 + ++i * 50, 50 + i * 50), ImGuiCond_FirstUseEver);
			ui::Begin(pi.first.c_str(), &pi.second.show_gui, ImGuiWindowFlags_NoCollapse);
			pi.second.plugin->OnGui();
			ui::End();
		}
	}
}

void YOPluginBus::OnMenu()
{
	for( auto &pi : plugins_)
	{
		if (ui::MenuItem(pi.first.c_str(), nullptr, pi.second.show_gui))
		{
			pi.second.show_gui = !pi.second.show_gui;
		}
	}
}
