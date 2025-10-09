/*
 * yo_can.cpp
 *
 *  Created on: Oct 9, 2025
 *      Author: kurtz
 */
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <net/if.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <getopt.h>

#include "config.h"
#include "YOTypes.h"
#include "YONode.h"

std::string g_name = "CAN Receiver";
std::string g_interface = "/dev/can0";
std::string g_input = "CAN0_OUT";
std::string g_output = "CAN0";
uint8_t g_channel = 0;

static struct option long_options[] = {
        {"name", optional_argument, NULL, 'n'},
        {"interface", optional_argument, NULL, 'i'},
        {"input", optional_argument, NULL, 't'},
        {"output", optional_argument, NULL, 'o'},
		{"channel", optional_argument, NULL, 'c'},
        {NULL, 0, NULL, 0}};

int main(int argc, char **argv)
{
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "n:t:t:o:c:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'n':
                g_name = optarg;
                break;
            case 't':
                g_input = optarg;
                break;
            case 'i':
                g_output = optarg;
                break;
            case 'f':
                g_interface = optarg;
                break;
            case 'c':
                g_channel = std::atoi(optarg);
                break;
        }
    }
    std::cout << " Node name    " <<  g_name << std::endl;
    std::cout << " Input topic  " <<  g_input << std::endl;
    std::cout << " Output topic " <<  g_output << std::endl;
    std::cout << " Interface    " <<  g_name << std::endl;
    std::cout << " Channel      " <<  g_channel << std::endl;

    YONode node("CAN NODE");
    node.advertise(g_output.c_str());
    node.connect();

    YOVariant frame("TEST");
    tCANFDData canfd_04;

    float angle = 0;
    while(node.isRunning())
    {
    	frame["SIN"] = sin(angle) * 140.0f;
    	frame["COS"] = cos(angle) * 140.0f;
    	YOMessage msgA(frame);
        node.sendMessage("PLOTTER", msgA);
    }
}

