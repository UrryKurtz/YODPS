/*
 * IConverter.h
 *
 *  Created on: Sep 9, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_ICONVERTER_H_
#define TOOLS_VIEWER_ICONVERTER_H_

#include <Urho3D/Core/Object.h>
#include <Scene/Node.h>
#include "YOTypes.h"
#include "YOVariant.h"

using namespace Urho3D;

class IConverter : public RefCounted
{
public:
    virtual ~IConverter() = default;
    virtual void convertObject(YOVariant &object, YOVariant &config, Node *node) = 0;
};

#endif /* TOOLS_VIEWER_ICONVERTER_H_ */
