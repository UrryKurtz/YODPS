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
    node.advertise(sub);
    node.advertise("PLOTTER");
    node.connect();

    YOVariant frame("TEST");

    tCANFDData canfd_04;
    canfd_04.sHeader.ui8Channel = 0;
    canfd_04.sData.ui32Id = 4;
    canfd_04.sData.ui8Length = 1;

    tCANFDData canfd_05;
    canfd_05.sHeader.ui8Channel = 0;
    canfd_05.sData.ui32Id = 5;
    canfd_05.sData.ui8Length = 1;


    float angle = 0;
    while(node.isRunning())
    {
    	frame["SIN"] = sin(angle) * 140.0f;
    	frame["COS"] = cos(angle) * 140.0f;
    	YOMessage msgA(frame);
        node.sendMessage("PLOTTER", msgA);

    	canfd_04.sData.aui8Data[0]++;
    	YOMessage msg_04(canfd_04);
    	node.sendMessage(sub, msg_04);

    	canfd_05.sData.aui8Data[0]++;
    	YOMessage msg_05(canfd_05);
    	node.sendMessage(sub, msg_05);

        usleep(50000);
        angle+=0.05;
    }
}

