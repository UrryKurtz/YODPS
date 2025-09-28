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
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>

#include "YONodeLogic.h"

YONodeLogic::YONodeLogic(Context *context) : LogicComponent(context)
{

}

YONodeLogic::~YONodeLogic()
{

}

void YONodeLogic::SetMaterials(SharedPtr<Material> fill, SharedPtr<Material> line, SharedPtr<Material> text)
{
	cache_ = GetSubsystem<ResourceCache>();
	material_fill_ = fill;
	material_line_ = line;
	material_text_ = text;

	if(!line_node_)
	{
		line_node_ = node_->CreateChild("line");
		line_node_->SetTemporary(true);
		line_geom_ = line_node_->CreateComponent<CustomGeometry>();
		line_geom_->SetMaterial(0, material_line_);
	}
	if(!fill_node_)
	{
		fill_node_ = node_->CreateChild("line");
		fill_node_->SetTemporary(true);
		fill_geom_ = fill_node_->CreateComponent<CustomGeometry>();
		fill_geom_->SetMaterial(0, material_fill_);
	}
	if(!text_node_)
		text_node_ = node_->CreateChild("text");

	if(!text_)
		text_ = text_node_ ->CreateComponent<Text3D>();

}

void YONodeLogic::SetFont(Font *font)
{
	font_ = font;
}

void YONodeLogic::Convert(YOVariant &object, YOVariant &type_cfg, int input_id)
{
	//std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
	object_ = &object;
	type_cfg_ = &type_cfg;
	style_id_ = type_cfg[yo::k::id];

	if(object.hasChild(yo::k::text))
	{
		ConvertText(object, input_id);
	}

	if (object[yo::k::object_type].get<int>() == (int) YOObjectType::YOGeomery)
	{
		ConvertGeometry(object, input_id);
	}
	else if (object[yo::k::object_type].get<int>() == (int) YOObjectType::YOModel)
	{
		ConvertModel(object, input_id);
	}
}

void YONodeLogic::SetInput(YOStatus *input)
{
	input_ = input ;
}

void YONodeLogic::SetType(YOStatus *type)
{
	type_ = type;
}

void YONodeLogic::Update(float timeStep)
{
	//TODO update on changes only
	CheckEnabled();
}

void YONodeLogic::CheckEnabled()
{
	line_node_->SetEnabled(input_->enable && input_->line && type_->enable && type_->line);
	fill_node_->SetEnabled(input_->enable && input_->fill && type_->enable && type_->fill);
	text_->SetEnabled(input_->enable && input_->text && type_->enable && type_->text);
}

void YONodeLogic::SetInputConfig(YOVariant &input_cfg)
{
	input_cfg_ = &input_cfg;
}

void YONodeLogic::ConvertGeometry(YOVariant &object, int input_id)
{
	YOVariant &geom = object[yo::k::geometry];
	uint32_t style_id = object[yo::k::style_id];
	PrimitiveType geomType = (PrimitiveType) geom[yo::k::geometry_type].get<int32_t>();
	YOVector3List &vertices = (YOVector3List&) geom[yo::k::vertices].get<YOFloatList>();

	CustomGeometry *cg;

	bool en_type = (*type_cfg_)[yo::k::enabled].get<bool>();
	bool en_geom = true;
	switch (geomType)
	{
	case TRIANGLE_LIST:
	case TRIANGLE_FAN:
	case TRIANGLE_STRIP:
		cg = fill_geom_;
		cg->BeginGeometry(0, geomType);
		en_geom = (*type_cfg_)[yo::k::fill][yo::k::enabled].get<bool>();
		break;
	case LINE_LIST:
	case POINT_LIST:
	case LINE_STRIP:
		cg = line_geom_;
		cg->BeginGeometry(0, geomType);
		en_geom = (*type_cfg_)[yo::k::line][yo::k::enabled].get<bool>();
		break;
	}

	for (auto &vertex : vertices)
	{
		cg->DefineNormal(Vector3::FORWARD);
		cg->DefineVertex((Urho3D::Vector3&) vertex);
		cg->DefineTexCoord(Vector2((float) input_id / YO_INPUT_NUM, (float) style_id / YO_TYPE_NUM));
	}
	cg->Commit();
}

void YONodeLogic::ConvertText(YOVariant &object, int input_id)
{
	YOVariant &text_cfg = (*type_cfg_)[yo::k::text];
	YOColor4F &color = text_cfg[yo::k::color];

	text_node_->SetPosition((Vector3 &)text_cfg[yo::k::position].get<YOVector3>());
    //!!!txt->SetLightMask(YO_LMASK_OVERLAY);
    //auto* cache = GetSubsystem<ResourceCache>();
    text_->SetColor((Color&) color);
    text_->SetFont(font_, text_cfg[yo::k::size].get<YOLimitF>().value);
    text_->SetText(object[yo::k::text].get<std::string>().c_str());
    text_->SetFaceCameraMode(FC_ROTATE_XYZ);
    text_->SetFixedScreenSize(true);
    text_->SetDepthTest(false);
    text_->SetOccludee(false);
    text_->SetCastShadows(false);
}

void YONodeLogic::ConvertModel(YOVariant &object, int input_id)
{
    if(!smodel_)
    	smodel_= node_->CreateComponent<StaticModel>();

    std::string &model = object[yo::k::model];
    std::string &texture = object[yo::k::texture];

    smodel_->SetModel(cache_->GetResource<Model>(model.c_str()));

    YOVariant &transform = object[yo::k::transform];

    node_->Rotate(Quaternion::IDENTITY);
    node_->SetScale(Vector3::ONE);

    node_->SetPosition((Vector3&) transform[yo::k::position].get<YOVector3>());
    node_->Rotate(Quaternion((Vector3&) transform[yo::k::rotation].get<YOVector3>()));
    node_->Scale((Vector3&) transform[yo::k::scale].get<YOVector3>());

    //SharedPtr<Material>mat(new Material(context_));
    auto mat = cache_->GetResource<Material>(texture.c_str());

//    mat->SetTechnique(0, technique_overlay_);
//    mat->SetRenderOrder(250);
//    mat->SetShaderParameter("MatDiffColor", Color(1, 1, 1, 0.95));
    //mat->SetCullMode(CULL_NONE);
    smodel_->SetMaterial(mat);
}

//void YONodeLogic::OnNodeSet(Node* previousNode, Node* currentNode)
//{
//	std::cout << __FUNCTION__ << std::endl;
//}
