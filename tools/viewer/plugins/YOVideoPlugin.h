/*
 * YOVideoPlugin.h
 *
 *  Created on: Sep 23, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOVIDEOPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOVIDEOPLUGIN_H_

#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Resource/Image.h>

#include "IPlugin.h"
#include "YOKeys.h"
#include "YOGui.h"

inline void createVideoCfg(uint32_t i, YOVariant &config, const std::string &name)
{
	config.m_name = name;
	config[yo::k::enabled] = true;
	config[yo::k::id] = i;
	config[yo::k::name] = (yo_keys[name] + " " + std::to_string(i)).c_str();
	config[yo::k::comment] = "";
	config[yo::k::overlay] = true;
	config[yo::k::opacity] = YOLimitF{.value = 1.0f, .min=0.0f, .max=1.0f, .speed=0.01};

	config[yo::k::size][yo::k::width] = (uint16_t) 0;
	config[yo::k::size][yo::k::height] = (uint16_t) 0;

	config[yo::k::plane][yo::k::width] = 3.0f;
	config[yo::k::plane][yo::k::height] = 4.0f;

    config[yo::k::transform][yo::k::position] = YOVector3{};
    config[yo::k::transform][yo::k::rotation] = YOVector3{};
    config[yo::k::transform][yo::k::scale] = YOVector3{1.0f, 1.0f, 1.0f};
}

class YOVideoPlugin : public IPlugin {

    std::array<SharedPtr<Texture2D>, YO_VIDEO_NUM> video_;
    std::array<std::mutex, YO_VIDEO_NUM> video_lock_;
    std::array<SharedPtr<Sprite>, YO_VIDEO_NUM> video_pad_;
    std::array<SharedPtr<Image>, YO_VIDEO_NUM> video_img_;

	YOGui gui_;
    YOVariant *video_cfg_ {nullptr};
    YOVariant *params_cfg_ {nullptr};

public:
	YOVideoPlugin(Context* context);
	virtual ~YOVideoPlugin();

	void OnStart() override;
	void OnUpdate(float timeStep) override;
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnGui() override;
	void ConvertVideos();
	void AddVideo(SharedPtr<Image> frame, int frame_id);

	void CreateTextures();
};

#endif /* TOOLS_VIEWER_PLUGINS_YOVIDEOPLUGIN_H_ */
