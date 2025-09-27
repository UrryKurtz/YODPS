/*
 * YONodeLogic.h
 *
 *  Created on: Sep 26, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_PLUGINS_YONODELOGIC_H_
#define TOOLS_VIEWER_PLUGINS_YONODELOGIC_H_

#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/UI/Text3D.h>

#include "YOVariant.h"
#include "YOKeys.h"
#include <map>
#include <vector>

using namespace Urho3D;

enum YODrawType {
	YOLine,
	YOFill
};

class YONodeLogic: public LogicComponent {
	URHO3D_OBJECT(YONodeLogic, LogicComponent);
	Text3D *text_ {nullptr};
	Node   *text_node_ {nullptr};
	Node   *line_node_ {nullptr};
	Node   *fill_node_ {nullptr};
	Node   *model_node_ {nullptr};
	CustomGeometry *line_geom_ {nullptr};
	CustomGeometry *fill_geom_ {nullptr};
	StaticModel *smodel_ {nullptr};

	YOVariant *type_cfg_ {nullptr};
	YOVariant *object_ {nullptr};

	SharedPtr<Material> material_fill_;
	SharedPtr<Material> material_line_;
	SharedPtr<Material> material_text_;
	YODrawType type_ {YODrawType::YOLine};

	ResourceCache *cache_ {nullptr};

	bool en_input {true};
	bool en_type {true};
	bool en_line {true};
	bool en_fill {true};
	bool en_text {true};
	Font *font_ {nullptr};
public:
	YONodeLogic(Context* context);
	virtual ~YONodeLogic();
	void SetMaterials(SharedPtr<Material> fill, SharedPtr<Material> line, SharedPtr<Material> text);
	void SetFont(Font *font);
	void Convert(YOVariant &object, YOVariant &type_cfg, int input_id);
	void ConvertGeometry(YOVariant &object, int input_id);
	void ConvertModel(YOVariant &object, int input_id);
	void ConvertText(YOVariant &object, int input_id);


	//void OnNodeSet(Node* previousNode, Node* currentNode) override;
	static void RegisterObject(Context* context)
	{
	    context->RegisterFactory<YONodeLogic>();
	}

	void CheckEnable();
	void EnableInput(bool enable);
	void EnableType(bool enable);
	void EnableLine(bool enable);
	void EnableFill(bool enable);
	void EnableText(bool enable);

};

#endif /* TOOLS_VIEWER_PLUGINS_YONODELOGIC_H_ */
