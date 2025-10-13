/*
 * YORootLogic.h
 *
 *  Created on: Sep 26, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YOROOTLOGIC_H_
#define TOOLS_VIEWER_PLUGINS_YOROOTLOGIC_H_

#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include "YOVariant.h"
#include "YOKeys.h"
#include "YONodeLogic.h"

using namespace Urho3D;

class YORootLogic: public LogicComponent
{
	URHO3D_OBJECT(YORootLogic, LogicComponent);
	 SharedPtr<Material> material_fill_;
	 SharedPtr<Material> material_line_;
	 SharedPtr<Material> material_text_;
	 YOVariant *input_cfg_ {nullptr};
	 YOVariant *types_cfg_ {nullptr};
	 std::shared_ptr<YOVariant> frame_;
	 uint32_t input_id_ {0};
  	 std::map<int, std::vector<YONodeLogic*>> subs_;
  	 YOStatus input_status_;
  	 YOStatus type_status_[YO_TYPE_NUM];
  	 ResourceCache *cache_ = GetSubsystem<ResourceCache>();
     Font *font_ {nullptr};
     Node *root_ovelay_{nullptr};
     std::vector<SharedPtr<Node>> nodes_;


public:
	YORootLogic(Context *context);
	virtual ~YORootLogic();

	void setMaterials(SharedPtr<Material> fill, SharedPtr<Material> line, SharedPtr<Material> text)
	{
		material_fill_ = fill->Clone();
		material_line_ = line->Clone();
		material_text_ = text->Clone();
	}

	void UpdateConfig();

	void SetOverlayRoot(Node *overlay);
	void SetInputConfig(YOVariant &input_cfg);
	void ConvertRoot(std::shared_ptr<YOVariant> frame);
	static void RegisterObject(Context* context)
	{
		context->RegisterFactory<YORootLogic>();
	}

	void SetPosition(const Vector3 &pos);
	void SetRotation(const Vector3 &rot);
	void SetScale(const Vector3 &scale);
	void SetTransformMode(int mode);

	void SetInputEnabled(int type, bool enable);
	void SetTypeEnabled(int type, bool enable);
	void SetLineEnabled(int type, bool enable);
	void SetTextEnabled(int type, bool enable);
	void SetFillEnabled(int type, bool enable);

	void SetLineColor(int type, const Color &color);
	void SetLineWidth(int type, const float width);
	void SetFillColor(int type, const Color &color);
	void SetTextColor(int type, const Color &color);
	void SetTextFont(int type, const std::string &font);
	void SetTextSize(int type, const float size);
	void SetTextPosition(int type, const Vector3 &pos);


};

#endif /* TOOLS_VIEWER_PLUGINS_YOROOTLOGIC_H_ */
