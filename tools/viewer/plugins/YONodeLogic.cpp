/*
 * YONodeLogic.cpp
 *
 *  Created on: Sep 26, 2025
 *      Author: kurtz
 */
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "YONodeLogic.h"

YONodeLogic::YONodeLogic(Context *context) : LogicComponent(context)
{

}

YONodeLogic::~YONodeLogic()
{

}

void YONodeLogic::setMaterials(SharedPtr<Material> fill, SharedPtr<Material> line, SharedPtr<Material> text)
{
	material_fill_ = fill;
	material_line_ = line;
	material_text_ = text;
}

void YONodeLogic::Convert(YOVariant &object, YOVariant &type_cfg, int input_id)
{
	object_ = &object;
	type_cfg_ = &type_cfg;
	if (object[yo::k::object_type].get<int>() == (int) YOObjectType::YOGeomery)
	{
		ConvertGeometry(object, input_id);
	}
	else if (object[yo::k::object_type].get<int>() == (int) YOObjectType::YOModel)
	{
		ConvertModel(object, input_id);
	}
}

void YONodeLogic::CheckEnable()
{
	node_->SetEnabled(en_type && ((type_ == YOLine && en_line) || (type_ == YOFill && en_fill)));
}

void YONodeLogic::EnableInput(bool enable)
{
	en_input = enable;
	CheckEnable();
}

void YONodeLogic::EnableLine(bool enable)
{
	en_line = enable;
	CheckEnable();
}

void YONodeLogic::EnableFill(bool enable)
{
	en_fill = enable;
	CheckEnable();
}

void YONodeLogic::EnableText(bool enable)
{
	en_text = enable;
	CheckEnable();
}

void YONodeLogic::ConvertGeometry(YOVariant &object, int input_id)
{
	node_->SetTemporary(true);
	YOVariant &geom = object[yo::k::geometry];
	uint32_t style_id = object[yo::k::style_id];
	PrimitiveType geomType = (PrimitiveType) geom[yo::k::geometry_type].get<int32_t>();
	YOVector3List &vertices = (YOVector3List&) geom[yo::k::vertices].get<YOFloatList>();
	CustomGeometry *cg = node_->CreateComponent<CustomGeometry>();

	bool en_type = (*type_cfg_)[yo::k::enabled].get<bool>();
	bool en_geom = true;
	switch (geomType)
	{
	case TRIANGLE_LIST:
	case TRIANGLE_FAN:
	case TRIANGLE_STRIP:
		cg->SetMaterial(0, material_fill_);
		cg->BeginGeometry(0, geomType);
		en_geom = (*type_cfg_)[yo::k::fill][yo::k::enabled].get<bool>();
		type_ = YOFill;
		break;
	case LINE_LIST:
	case POINT_LIST:
	case LINE_STRIP:
		cg->SetMaterial(0, material_line_);
		cg->BeginGeometry(0, geomType);
		en_geom = (*type_cfg_)[yo::k::line][yo::k::enabled].get<bool>();
		type_ = YOLine;
		break;
	}
	cg->SetEnabled(en_geom && en_type);

	for (auto &vertex : vertices)
	{
		cg->DefineNormal(Vector3::FORWARD);
		cg->DefineVertex((Urho3D::Vector3&) vertex);
		cg->DefineTexCoord(Vector2((float) input_id / YO_INPUT_NUM, (float) style_id / YO_TYPE_NUM));
	}
	cg->Commit();
}

void YONodeLogic::ConvertModel(YOVariant &object, int input_id)
{
    auto *boxModel = node_->CreateComponent<StaticModel>();

    auto *cache_ = GetSubsystem<ResourceCache>();

    std::string &model = object[yo::k::model];
    std::string &texture = object[yo::k::texture];

    boxModel->SetModel(cache_->GetResource<Model>(model.c_str()));

    YOVariant &transform = object[yo::k::transform];
    node_->SetPosition((Vector3&) transform[yo::k::position].get<YOVector3>());
    node_->Rotate(Quaternion((Vector3&) transform[yo::k::rotation].get<YOVector3>()));
    node_->Scale((Vector3&) transform[yo::k::scale].get<YOVector3>());
    //SharedPtr<Material>mat(new Material(context_));
    auto mat = cache_->GetResource<Material>(texture.c_str());
//    mat->SetTechnique(0, technique_overlay_);
//    mat->SetRenderOrder(250);
//    mat->SetShaderParameter("MatDiffColor", Color(1, 1, 1, 0.95));
    //mat->SetCullMode(CULL_NONE);
    boxModel->SetMaterial(mat);
}

//void YONodeLogic::OnNodeSet(Node* previousNode, Node* currentNode)
//{
//	std::cout << __FUNCTION__ << std::endl;
//}
