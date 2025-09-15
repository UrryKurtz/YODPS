/*
 * YOConverter.cpp
 *
 *  Created on: Sep 9, 2025
 *      Author: kurtz
 */

#include "YOConverter.h"

YOConverter::YOConverter()
{
    
}

YOConverter::~YOConverter()
{

}

void YOConverter::registerConverter(int type, std::shared_ptr<IConverter> conv)
{
    m_converters[type] = conv;
}

void YOConverter::registerRoot(int type,  Node *node)
{
    m_roots[type] = node;
}

void YOConverter::convertFrame(int input, std::shared_ptr<YOVariant> frame, std::shared_ptr<YOVariant> config, Node* parent)
{
    YOArray &objects = frame->get(yo::k::objects);
    for( auto &obj : objects)
    {
        int type = obj[yo::k::type];
        int coord = obj[yo::k::coord];
        auto converter = m_converters.find(type);
        if(converter != m_converters.end())
        {
            converter->second->convertObject(obj, *config, m_roots[coord]);
        }
    }
}

