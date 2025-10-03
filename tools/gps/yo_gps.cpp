/*
 * yo_gps.cpp
 *
 *  Created on: Sep 29, 2025
 *      Author: kurtz
 */
#include <gps.h>
#include <iostream>
#include "config.h"
#include "YOTypes.h"
#include "YONode.h"
#include "YOKeys.h"

uint16_t counter_ = 0;

const char *topic_ = "COORDINATES";

int main(int argc, char **argv)
{
	if(argc > 1)
		topic_ = argv[1];

    std::cout << " ADVERTISE " <<  topic_  << std::endl;
    YOVariant frame("COORDINATES");

    YONode node("GPSTest");
    node.advertise(topic_ , 0x0, 0x0);
    node.connect();

	gps_data_t gps{};
	if (gps_open("localhost", "2947", &gps) != 0)
	{
		std::cout << " Can not open GPSD socket " << std::endl;
		return 1;
	}

	gps_stream(&gps, WATCH_ENABLE | WATCH_JSON, nullptr);
    while(node.isRunning())
    {
		if (gps_waiting(&gps, 500000) && gps_read(&gps, nullptr, 0) > 0)
		{
			if ((gps.set & LATLON_SET) && gps.fix.mode >= MODE_2D)
			{
				frame[yo::k::coord] = YOVector2 {(float)gps.fix.latitude, (float)gps.fix.longitude};
				//frame[yo::k::coord] = YOVector2 {50.0865f, 14.4110f}; //TEST GPS
		    	YOMessage msg(frame);
		    	node.sendMessage(topic_, msg);
			}
		}
        usleep(250000);
    }
}
