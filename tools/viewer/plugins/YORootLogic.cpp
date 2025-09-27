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

void YORootLogic::ConvertRoot(std::shared_ptr<YOVariant> frame, YOVariant &input_cfg)
{
	YOVariant &transform = input_cfg[yo::k::transform];
	node_->SetPosition((Vector3&)transform[yo::k::position].get<YOVector3>());
    node_->SetRotation(Quaternion((Vector3&)transform[yo::k::rotation].get<YOVector3>()));
    node_->SetScale((Vector3&)transform[yo::k::scale].get<YOVector3>());

    YOVariant &objects = (*frame)[yo::k::objects];
    uint32_t input_id = input_cfg[yo::k::id];
    YOVariant &types = input_cfg[yo::k::types];
    auto *cache = GetSubsystem<ResourceCache>();
    Font *font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

    auto &nodes = node_->GetChildren();
    for(int i = nodes.size(); i < objects.getArraySize(); i++ )
    {
    	Node *node  = node_->CreateChild();
    	node->SetTemporary(true);
		YONodeLogic *nl = node->CreateComponent<YONodeLogic>();
		nl->SetMaterials(material_fill_, material_line_, material_text_);
		nl->SetFont(font);
    }

    for(int i = 0; i < objects.getArraySize(); i++)
	{
		YOVariant &object = objects[i];
		uint32_t style_id = object[yo::k::style_id];
		//subs_[style_id].push_back(nl);
		SharedPtr<Node> node = nodes[i];
		auto nl = node->GetComponent<YONodeLogic>();
		nl->Convert(object, types[style_id], input_id);
	}
}


void YORootLogic::EnableInput(bool enable)
{

}

void YORootLogic::EnableType(int type, bool enable)
{

}

void YORootLogic::EnableLine(int type, bool enable)
{

}

void YORootLogic::EnableText(int type, bool enable)
{

}

void EnableFill(int type, bool enable)
{

}
