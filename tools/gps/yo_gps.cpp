/*
 * yo_gps.cpp
 *
 *  Created on: Sep 29, 2025
 *      Author: kurtz
 */

#include <iostream>
#include "config.h"
#include "YOTypes.h"
#include "YONode.h"
#include "YOKeys.h"

uint16_t counter_ = 0;

const char *topic_ = "GPS";

int main(int argc, char **argv)
{
	if(argc > 1)
		topic_ = argv[1];

    std::cout << " ADVERTISE " <<  topic_  << std::endl;

    YOVariant frame("frame");

    YONode node("GPSTest");
    node.advertise(topic_ , 0x0, 0x0);
    node.connect();

    while(node.isRunning())
    {
        YOMessage msg(frame);
        printf("%010llu DATA %d \n", msg.getTimestamp(), msg.getDataSize());
        node.sendMessage(topic_, msg);
        usleep(500000);

    }
}

