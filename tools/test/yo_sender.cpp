/*
 * yo_broker.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */
#include <iostream>
#include <cmath>
#include "config.h"
#include "YOTypes.h"
#include "YONode.h"

int main(int argc, char **argv)
{
    char *sub = argv[1];
    std::cout << " ADVERTISE " <<  sub << std::endl;

    YONode node("TEST NODE");
    node.advertise(sub, 0xAA, 0xBB);
    node.connect();
    YOVariant frame("TEST");

    float angle = 0;
    while(node.isRunning())
    {
    	frame["SIN"] = sin(angle) * 140.0f;
    	frame["COS"] = cos(angle) * 140.0f;
    	YOMessage msgA(frame);
        //printf("%010llu DATA %010llu\n", msgA.getTimestamp(), *data);
        node.sendMessage(sub, msgA);
        usleep(50000);
        angle+=0.05;
    }
}

