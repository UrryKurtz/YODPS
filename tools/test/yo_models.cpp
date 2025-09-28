/*
 * yo_broker.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#include <iostream>
#include "config.h"
#include "YOTypes.h"
#include "YONode.h"
#include "YOKeys.h"


uint16_t counter_ = 0;

const char *topic_ = "INPUT3";

int main(int argc, char **argv)
{
	if(argc > 1)
		topic_ = argv[1];

    std::cout << " ADVERTISE " <<  topic_  << std::endl;

    YOVariant frame("frame");
    frame[yo::k::objects] = YOArray();

    frame[yo::k::sender] = "Model Converter";
    frame[yo::k::frame_id] = counter_++;

    YOVariant &transform = frame[yo::k::transform];
    transform[yo::k::position] = YOVector3{0.0f, 0.0f, 0.0f};
    transform[yo::k::rotation] = YOVector3{0.0f, 0.0f, 0.0f};
    transform[yo::k::scale] = YOVector3{1.0f, 1.0f, 1.0f};

    YOVariant object("object");
    object[yo::k::object_type] = YOObjectType::YOModel;
    object[yo::k::style_id] = 0u;

    YOVariant &obj_transform = object[yo::k::transform];
    obj_transform[yo::k::position] = YOVector3{0.0f, 0.0f, 0.0f};
    obj_transform[yo::k::rotation] = YOVector3{0.0f, 0.0f, 0.0f};
    obj_transform[yo::k::scale] = YOVector3{3.0f, 1.0f, 2.0f};

    object[yo::k::model] = "Models/box.mdl";
    object[yo::k::texture] = "Materials/DefaultGrey.xml";

    frame[yo::k::objects].push_back(object);

    std::cout << " MSG START " << sizeof(tCANData) << std::endl;

    YONode node("ModelTest");
    node.advertise(topic_ , 0xAA, 0xBB);
    node.connect();

    YOVariant &object_ = frame[yo::k::objects][0];
    YOVector3 pos = { 0, 0, 0};
    YOVector3 rot = { 0, 0, 0};
    float angle = 0.0f;

    while(node.isRunning())
    {
        YOMessage msg(frame);
        printf("%010llu DATA %d \n", msg.getTimestamp(), msg.getDataSize());
        node.sendMessage(topic_, msg);
        usleep(5000);

        pos.x = 50*sin(angle);
        pos.y = 50*cos(angle);
        object_[yo::k::transform][yo::k::position] = pos;

        rot.z+=0.002f;
        object_[yo::k::transform][yo::k::rotation] = rot;

        angle += 0.001f;
    }
}

