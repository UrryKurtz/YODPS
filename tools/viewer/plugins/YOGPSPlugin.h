/*
 * YOGPSPlugin.h
 *
 *  Created on: Sep 29, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOGPSPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOGPSPLUGIN_H_

#include <Urho3D/Container/Str.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/HttpRequest.h>
#include <Urho3D/Resource/Image.h>
#include <Urho3D/IO/MemoryBuffer.h>

#include "YONode.h"
#include "IPlugin.h"
#include "YOGPSUtils.h"

using YOCacheY = std::map <uint32_t, SharedPtr<Image>> ;
using YOCacheX = std::map <uint32_t, YOCacheY> ;
using YOCache = std::map <uint32_t, YOCacheX> ;

using namespace Urho3D;
class YOGPSPlugin: public IPlugin
{
	std::string path_{"Data/OSM"};
	YOGui gui_;
	YOGPSUtils gps_;
	YONode *sender_ {nullptr};
	std::string topic_;
	YOCache cache_;
	SharedPtr<Image> full_img_;

	bool update_ {true};
	std::mutex locks_;
	SharedPtr<Texture2D> map_;
	SharedPtr<HttpRequest> req_;

public:
	YOGPSPlugin(Context *context);
	virtual ~YOGPSPlugin();
	void OnStart() override;
	void OnUpdate(float timeStep) override;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnGui() override;

	void RequestTile (int scale, int x, int y);
};

#endif /* TOOLS_VIEWER_PLUGINS_YOGPSPLUGIN_H_ */
