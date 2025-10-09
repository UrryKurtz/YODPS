/*
 * yo_can.cpp
 *
 *  Created on: Oct 9, 2025
 *      Author: kurtz
 */
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
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
std::string g_interface = "can0";
std::string g_output = "CAN0";
uint8_t g_channel = 0;
int g_socket = 0;

static struct option long_options[] =
{
{ "name", optional_argument, NULL, 'n' },
{ "interface", optional_argument, NULL, 'i' },
{ "output", optional_argument, NULL, 'o' },
{ "channel", optional_argument, NULL, 'c' },
{ NULL, 0, NULL, 0 } };

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
	std::cout << " Node name    " << g_name << std::endl;
	std::cout << " Output topic " << g_output << std::endl;
	std::cout << " Interface    " << g_name << std::endl;
	std::cout << " Channel      " << g_channel << std::endl;

	YONode node(g_name.c_str());
	node.advertise(g_output.c_str());
	node.connect();

	g_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);

//    	can_filter flt{};
//		flt.can_id   = 0x42;
//		flt.can_mask = CAN_SFF_MASK; // check up to 11-bit ID
//		setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &flt, sizeof(flt));

	ifreq ifr{};
	std::strcpy(ifr.ifr_name, g_interface.c_str());
	ioctl(g_socket, SIOCGIFINDEX, &ifr);

	int off = 1;
	setsockopt(g_socket, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &off, sizeof(off)); //CAN_RAW_LOOPBACK

	sockaddr_can addr{};
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	bind(g_socket, (sockaddr*) &addr, sizeof(addr));

    struct pollfd pfd{};
    pfd.fd = g_socket;
    pfd.events = POLLIN;

	while (node.isRunning())
	{
		can_frame f{};
		int rc = poll(&pfd, 1, 200); // 200 ms timeout
		if (rc <= 0) continue;
		if (pfd.revents & POLLIN)
		{
			struct can_frame fr{};
			ssize_t n = read(g_socket, &fr, sizeof(fr));
			if (n == sizeof(fr))
			{
				tCANData can{0};
				can.sHeader.ui8Channel = g_channel;
				can.sHeader.tmTimeStamp = YONode::getTimestamp();
				can.sData.ui32Id = fr.can_id & CAN_SFF_MASK;
				can.sData.ui8Length = fr.can_dlc;
				memcpy(can.sData.aui8Data, fr.data, fr.can_dlc);
				YOMessage msg(can);
				node.sendMessage(g_output.c_str(), msg);

				std::printf("%03X [%d] ", fr.can_id & CAN_SFF_MASK, fr.can_dlc);
				for (int i = 0; i < fr.can_dlc; ++i)
					std::printf("%02X ", fr.data[i]);
				std::printf("\n");
			}
		}
	}
	std::printf("EXIT FROM LOOP\n");
	close(g_socket);
}

