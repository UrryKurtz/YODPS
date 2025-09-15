/*
 * YOXML.h
 *
 *  Created on: Sep 13, 2025
 *      Author: kurtz
 */

#ifndef UTILS_YOXML_H_
#define UTILS_YOXML_H_
#include "YOVariant.h"
#include <tinyxml2.h>


class YOXML
{
    tinyxml2::XMLDocument doc_;
public:
    YOXML();
    virtual ~YOXML();
    bool readXML(const std::string &file, YOVariant &output);
    bool writeXML(const std::string &file, YOVariant &input);
    void writeNode(YOVariant &node, tinyxml2::XMLElement* parent,  const std::string &tab = "  ");
    void readNode(YOVariant &parent, tinyxml2::XMLElement* node,  const std::string &tab = "  ");


};

#endif /* UTILS_YOXML_H_ */
