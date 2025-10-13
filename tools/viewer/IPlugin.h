/*
 * IPlugin.h
 *
 *  Created on: Sep 23, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_IPLUGIN_H_
#define TOOLS_VIEWER_IPLUGIN_H_

class IPlugin;
class YOPluginBus;

#include "YOKeys.h"
#include "YOGui.h"
#include <YOMessage.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;

//namespace PluginBase
//{
  //  void Send(IPlugin* self, std::string_view topic, const uint8_t* data, size_t size);
  //  void Send(IPlugin* self, std::string_view topic, std::shared_ptr<YOVariant> data);
//}

class IPlugin : public Object {
	 URHO3D_OBJECT(IPlugin, Object);
	 YOVariant *config_ {nullptr};
	 Node *node_;
	 std::map<std::string, int> topics_;
	 std::vector<std::string> subs_;
	 std::vector<std::string> ads_;

protected:
	 YOPluginBus *bus_;
	 std::string name_;

public:
	using Object::Object;
	virtual ~IPlugin() = default;
	void SetBus(YOPluginBus* bus) { bus_ = bus; }
	void SetName(const std::string &name) { name_ = name; }
	const std::string &GetName() { return name_; }

	virtual void OnStart(){}
	void SetConfig(YOVariant *config) { config_ = config; OnSetConfig(config_); }
	void SetNode(Node *node) { node_ = node; OnSetNode(node_); }
	virtual void OnSetConfig(YOVariant *config){}
	virtual void OnSetNode(Node *node){}
	virtual void OnStop(){}
	virtual void OnData(const std::string &topic, std::shared_ptr<YOMessage> message){}
	virtual void OnUpdate(float timeStep){}
	virtual void OnGui(){}

	std::vector<std::string> GetSubscriptions() const { return subs_; }
	std::vector<std::string> GetAdvertisements() const { return ads_; }
	void RegisterTopic(const std::string &name, int num) {topics_[name] = num; subs_.push_back(name); }
    int GetTopicId(const std::string &name) {return topics_[name];}
	void Transmit(const std::string &topic, const uint8_t* data, size_t size);
	void Transmit(const std::string &topic, const YOVariant &data);
	void Transmit(const std::string &topic, YOMessage &message);

};

#endif /* TOOLS_VIEWER_IPLUGIN_H_ */

