/*
 * YOPluginBus.h
 *
 *  Created on: Sep 23, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_YOPLUGINBUS_H_
#define TOOLS_VIEWER_YOPLUGINBUS_H_


class IPlugin;
class YOPluginBus;

#include "YONode.h"
#include "YOKeys.h"
#include  <map>
#include  <vector>
#include  <pthread.h>

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>


using namespace Urho3D;
struct YOPluginInfo
{
	std::string name;
	bool show_gui {false};
	IPlugin *plugin {};
	Urho3D::Node* node {};
	YOVariant *config {nullptr};
	std::string file;
    std::vector<std::string> subs;       // GetSubscriptions()
	std::vector<std::string> adverts;    // GetAdvertisements()
	pthread_t thread {};
	YONode *yonode {nullptr};

};


class YOPluginBus : public Object {

	URHO3D_OBJECT(YOPluginBus, Object);
	std::map<std::string, YOPluginInfo> plugins_;
	YOVariant *config_ {nullptr};

public:
	YOPluginBus(Context *context);
	virtual ~YOPluginBus();
	void AddPlugin(const std::string &name, IPlugin* plugin);
	void Transmit(IPlugin* self, const std::string &topic, const uint8_t* data, size_t size);
	void Transmit(IPlugin* self, const std::string &topic, const YOVariant &data);
	void Transmit(IPlugin* self, const std::string &topic, YOMessage &message);

	void OnStart(Scene *scene);
	void OnStop();
	void OnUpdate(float timeStep);
	void OnMenu();
	void OnGui();
	void SetConfig(YOVariant *config);
};

#endif /* TOOLS_VIEWER_YOPLUGINBUS_H_ */
