/*
 * YOPolylinePlugin.h
 *
 *  Created on: Sep 23, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOPOLYLINEPLUGIN_H_
#define TOOLS_VIEWER_PLUGINS_YOPOLYLINEPLUGIN_H_
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/RenderPipeline/ShaderConsts.h>

#include "IPlugin.h"
#include "YOKeys.h"
#include "YOGui.h"

struct YOInputData
{
	Node* root;
	std::array<std::vector<Node*>, YO_TYPE_NUM> types;
	std::array<std::vector<CustomGeometry*>, YO_TYPE_NUM> geom_lines;
	std::array<std::vector<CustomGeometry*>, YO_TYPE_NUM> geom_fills;
	std::array<std::vector<CustomGeometry*>, YO_TYPE_NUM> geoms;
};

class YOPolylinePlugin : public IPlugin {

	SharedPtr<Texture2D> texture_param_;
	SharedPtr<Texture2D> texture_line_;
	SharedPtr<Texture2D> texture_fill_;
	SharedPtr<Texture2D> texture_text_;

	SharedPtr<Material> material_param_;
	SharedPtr<Material> material_line_;
	SharedPtr<Material> material_fill_;
	SharedPtr<Material> material_text_;

	std::array<std::shared_ptr<YOVariant>, YO_INPUT_NUM> data_in_;
    std::array<std::shared_ptr<YOInputData>, YO_INPUT_NUM> data_;
    std::array<std::mutex, YO_INPUT_NUM> data_lock_;

    ResourceCache *cache_;
	YOGui gui_;
	YOVariant *inputs_cfg_ {nullptr};
	YOVariant *params_cfg_ {nullptr};

public:
	YOPolylinePlugin(Context* context);
	virtual ~YOPolylinePlugin();
	void OnSetConfig(YOVariant *config) override;
	void OnStart() override ;
	void OnStop() override {}
	void OnData(const std::string &topic, std::shared_ptr<YOMessage> message) override;
	void OnUpdate(float timeStep) override;
	void OnGui() override;

	SharedPtr<Texture2D> CreateTexture();
	void AddFrame(std::shared_ptr<YOVariant> frame, int frame_id);
    void OnGuiChanged(const std::string &path, std::vector<int> &addr,  YOVariant *cfg);

    std::shared_ptr<YOInputData> ConvertFrame(std::shared_ptr<YOVariant> frame, int frame_id);
    void ConvertGeometry(YOVariant &obj, std::shared_ptr<YOInputData> fdata, YOVariant &input_cfg, int frame_id);

    void SetLineEnabled(const std::string &path, YOVariant *cfg, int input, int type);
    void SetLineColor(const std::string &path, YOVariant *cfg, int input, int type);
    void SetLineWidth(const std::string &path, YOVariant *cfg, int input, int type);
    void SetFillEnabled(const std::string &path, YOVariant *cfg, int input, int type);
    void SetFillColor(const std::string &path, YOVariant *cfg, int input, int type);
    void SetTextEnabled(const std::string &path, YOVariant *cfg, int input, int type);
    void SetTextColor(const std::string &path, YOVariant *cfg, int input, int type);

};

#endif /* TOOLS_VIEWER_PLUGINS_YOPOLYLINEPLUGIN_H_ */
