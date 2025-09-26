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
	 std::shared_ptr<YOVariant> frame_;
	 uint32_t frame_id_ {0};
  	std::map<int, std::vector<YONodeLogic*>> subs_;

public:
	YORootLogic(Context *context);
	virtual ~YORootLogic();

	void setMaterials(SharedPtr<Material> fill, SharedPtr<Material> line, SharedPtr<Material> text)
	{
		material_fill_ = fill;
		material_line_ = line;
		material_text_ = text;
	}

	void ConvertRoot(std::shared_ptr<YOVariant> frame, YOVariant &input_cfg);
	static void RegisterObject(Context* context)
	{
		context->RegisterFactory<YORootLogic>();
	}

	void SetPosition(const Vector3 &pos);
	void SetRotation(const Vector3 &rot);
	void SetScale(const Vector3 &scale);
	void SetTransformMode(int mode);

	void EnableInput(bool enable);
	void EnableType(int type, bool enable);
	void EnableLine(int type, bool enable);
	void EnableText(int type, bool enable);
	void EnableFill(int type, bool enable);

	void SetLineColor(int type, const Color &color);
	void SetLineWidth(int type, const float width);
	void SetFillColor(int type, const Color &color);
	void SetTextColor(int type, const Color &color);
	void SetTextFont(int type, const std::string &font);
	void SetTextSize(int type, const float size);
	void SetTextPosition(int type, const Vector3 &pos);


};

#endif /* TOOLS_VIEWER_PLUGINS_YOROOTLOGIC_H_ */
