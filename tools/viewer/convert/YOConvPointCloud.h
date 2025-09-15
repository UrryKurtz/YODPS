/*
 * YOConvPointCloud.h
 *
 *  Created on: Sep 9, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_YOCONVPOINTCLOUD_H_
#define TOOLS_VIEWER_YOCONVPOINTCLOUD_H_

#include "../IConverter.h"
#include "YOKeys.h"

class YOConvPointCloud: public IConverter
{
public:
    YOConvPointCloud();
    virtual ~YOConvPointCloud();
    virtual void convertObject(YOVariant &object, YOVariant &config, Urho3D::Node *node);
};

#endif /* TOOLS_VIEWER_YOCONVPOINTCLOUD_H_ */
