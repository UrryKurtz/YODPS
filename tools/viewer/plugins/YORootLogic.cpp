/*
 * YORootLogic.cpp
 *
 *  Created on: Sep 26, 2025
 *      Author: kurtz
 */

#include "YORootLogic.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Font.h>

YORootLogic::YORootLogic(Context *context) : LogicComponent(context)
{

}

YORootLogic::~YORootLogic()
{

}

void YORootLogic::SetInputConfig(YOVariant &input_cfg)
{
	input_cfg_ = &input_cfg;
    input_id_ = (*input_cfg_)[yo::k::id];
    material_fill_->SetRenderOrder(input_id_ * 10);
    material_line_->SetRenderOrder(input_id_ * 10);
    material_text_->SetRenderOrder(input_id_ * 10);

    types_cfg_ = &(*input_cfg_)[yo::k::types];
    cache_ = GetSubsystem<ResourceCache>();
    font_ = cache_->GetResource<Font>("Fonts/Anonymous Pro.ttf");
}

void YORootLogic::UpdateConfig()
{
	input_status_.enable = (*input_cfg_)[yo::k::enabled];
	input_status_.line = (*input_cfg_)[yo::k::line][yo::k::enabled];
	input_status_.fill = (*input_cfg_)[yo::k::fill][yo::k::enabled];
	input_status_.text = (*input_cfg_)[yo::k::text][yo::k::enabled];

    for( int i = 0; i < types_cfg_->getArraySize(); i++)
    {
    	YOVariant &type = (*types_cfg_)[i];
    	type_status_[i].enable = type[yo::k::enabled];
    	type_status_[i].line = type[yo::k::line][yo::k::enabled];
    	type_status_[i].fill = type[yo::k::fill][yo::k::enabled];
    	type_status_[i].text = type[yo::k::text][yo::k::enabled];
    }
}

void YORootLogic::ConvertRoot(std::shared_ptr<YOVariant> frame)
{
	YOVariant &transform = (*input_cfg_)[yo::k::transform];
	node_->SetPosition((Vector3&)transform[yo::k::position].get<YOVector3>());
    node_->SetRotation(Quaternion((Vector3&)transform[yo::k::rotation].get<YOVector3>()));
    node_->SetScale((Vector3&)transform[yo::k::scale].get<YOVector3>());

    YOVariant &objects = (*frame)[yo::k::objects];
    auto &nodes = node_->GetChildren();
    for(int i = nodes.size(); i < objects.getArraySize(); i++ )
    {
    	Node *node  = node_->CreateChild();
    	node->SetTemporary(true);
		YONodeLogic *nl = node->CreateComponent<YONodeLogic>();
		nl->SetMaterials(material_fill_, material_line_, material_text_);
		nl->SetFont(font_);
    }

    subs_.clear();
    for(int i = 0; i < objects.getArraySize(); i++)
	{
		YOVariant &object = objects[i];
		uint32_t style_id = object[yo::k::style_id];
		SharedPtr<Node> node = nodes[i];
		auto nl = node->GetComponent<YONodeLogic>();
		subs_[style_id].push_back(nl);
		nl->SetInput(&input_status_);
		nl->SetType(&type_status_[style_id]);
		nl->Convert(object, (*types_cfg_)[style_id], input_id_);
		nl->CheckEnabled();
	}
}

void YORootLogic::SetInputEnabled(int type, bool enable)
{

}

void YORootLogic::SetTypeEnabled(int type, bool enable)
{

}

void YORootLogic::SetLineEnabled(int type, bool enable)
{

}

void YORootLogic::SetTextEnabled(int type, bool enable)
{

}

void SetFillEnabled(int type, bool enable)
{

}
