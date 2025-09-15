/*
 * YOConvPointCloud.cpp
 *
 *  Created on: Sep 9, 2025
 *      Author: kurtz
 */

#include "YOConvPointCloud.h"
#include <Graphics/CustomGeometry.h>

YOConvPointCloud::YOConvPointCloud()
{
}

YOConvPointCloud::~YOConvPointCloud()
{
}

void YOConvPointCloud::convertObject(YOVariant &object, YOVariant &config, Urho3D::Node *parent)
{
    Urho3D::Node *node = parent->CreateChild(0, true);
    YOVector3List &vertices = object[yo::k::vertices];

    CustomGeometry *cg = node->CreateComponent<CustomGeometry>();
    cg->BeginGeometry(0u, PrimitiveType::POINT_LIST);

    //YOColor4C &clr = colors[i];
    //Color cur_clr(inv * clr.r, inv * clr.g, inv * clr.b, inv * clr.a);

    for( auto &vertex : vertices)
    {
        cg->DefineNormal(Vector3::FORWARD);
        cg->DefineVertex((Urho3D::Vector3&)vertex);
        //cg->DefineColor(cur_clr);
    }
    //cg->SetMaterial(mat);
    cg->Commit();
}
