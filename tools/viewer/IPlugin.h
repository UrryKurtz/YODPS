/*
 * IPlugin.h
 *
 *  Created on: Sep 23, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_IPLUGIN_H_
#define TOOLS_VIEWER_IPLUGIN_H_

#include "YOKeys.h"
#include "YOGui.h"
#include <YOMessage.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;

class IPlugin;
namespace PluginBase
{
    void Send(IPlugin* self, std::string_view topic, const uint8_t* data, size_t size);
}

class IPlugin : public Object {
	 URHO3D_OBJECT(IPlugin, Object);
	 YOVariant *config_ {nullptr};
	 Node *node_;
	 std::map<std::string, int> topics_;
	 std::vector<std::string> subs_;
	 std::vector<std::string> ads_;

public:
	using Object::Object;
	virtual ~IPlugin() = default;
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

	inline void Transmit(std::string_view topic, const uint8_t* data, size_t size, IPlugin* self)
	{
	    PluginBase::Send(self, topic, data, size);
	}

};



#endif /* TOOLS_VIEWER_IPLUGIN_H_ */
