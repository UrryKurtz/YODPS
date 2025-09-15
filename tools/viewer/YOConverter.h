/*
 * YOConverter.h
 *
 *  Created on: Sep 9, 2025
 *      Author: kurtz
 */

#ifndef TOOLS_VIEWER_YOCONVERTER_H_
#define TOOLS_VIEWER_YOCONVERTER_H_
#include "YOTypes.h"
#include "YOVariant.h"
#include "YOViewer.h"
#include "YOKeys.h"

#include "IConverter.h"

class YOConverter
{
    std::map<int, std::shared_ptr<IConverter>> m_converters;
    std::map<int, Node*> m_roots;

public:
    YOConverter();
    virtual ~YOConverter();
    void registerConverter(int type, std::shared_ptr<IConverter> conv);
    void registerRoot(int type,  Node *node);
    void convertFrame(int input, std::shared_ptr<YOVariant> frame, std::shared_ptr<YOVariant> config, Node* parent);
};

#endif /* TOOLS_VIEWER_YOCONVERTER_H_ */
