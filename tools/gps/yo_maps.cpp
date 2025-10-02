/*
 * yo_maps.cpp
 *
 *  Created on: Sep 29, 2025
 *      Author: kurtz
 */
#include "config.h"

#include <iostream>
#include <getopt.h>
#include <cmath>

#include "YOTypes.h"
#include "YONode.h"
#include "YOKeys.h"


//#include "YOGPSUtils.h"

uint16_t counter_ = 0;

std::string g_name_ = "GPSNode";
std::string g_input_ = "GPS";
std::string g_topic_ = "VIDEO3";
uint32_t g_x_ = 0;
uint32_t g_y_ = 0;
uint32_t g_z_ = 0;

float g_lon_ = 0.0f;
float g_lat_ = 0.0f;

YOVector3 g_gps_pos_ = { 0, 0, 0};
YOVector3 g_request_ = { 0, 0, 0};
YOImageData g_image_ = {};

inline int lon2x(float lon_deg, int z) {
    float n = std::pow(2.0, z);
    int ret = ((lon_deg + 180.0) / 360.0 * n);
    std::cout << " x !! "<< ret << std::endl;
    return ret;
}

inline int lat2y(float lat_deg, int z) {
    float lat_rad = lat_deg * M_PI / 180.0;
    float n = std::pow(2.0, z);
    int ret = ((1.0 - std::log(std::tan(lat_rad) + 1.0/std::cos(lat_rad)) / M_PI) / 2.0 * n);
    std::cout << " y !! "<< ret << std::endl;
    return ret;
}

double lon2xf(double lon, int z) {
    return (lon + 180.0) / 360.0 * (1 << z);
}

double lat2yf(double lat, int z) {
    double lat_rad = lat * M_PI / 180.0;
    return (1.0 - std::log(std::tan(lat_rad) + 1.0/std::cos(lat_rad)) / M_PI) / 2.0 * (1 << z);
}

static struct option long_options[] = {
    { "name", optional_argument, NULL, 'n' },
    { "topic", required_argument, NULL, 't' },
    { "input", required_argument, NULL, 'i' },
    { "x", required_argument, NULL, 'x' },
    { "y", required_argument, NULL, 'y' },
	{ "z", required_argument, NULL, 'z' },
    { "lat", required_argument, NULL, 'a' },
	{ "lon", required_argument, NULL, 'o' },

    { NULL, 0, NULL, 0 } };

int main(int argc, char **argv)
{
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "n:t:i:x:y:z:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'n':
                g_name_ = optarg;
                break;
            case 't':
                g_topic_ = optarg;
                break;
            case 'i':
                g_input_ = optarg;
                break;
            case 'x':
                g_x_ = std::atoi(optarg);
                break;
            case 'y':
            	g_y_ = std::atoi(optarg);
                break;
            case 'z':
            	g_z_ = std::atoi(optarg);
                break;
            case 'o':
            	g_lon_ = std::atof(optarg);
            	g_x_ = lon2x(g_lon_, g_z_);
                break;
            case 'a':
            	g_lat_ = std::atof(optarg);
            	g_y_ = lat2y(g_lat_, g_z_);
                break;
        }
    }

    std::cout << " ADVERTISE " <<  g_topic_  << std::endl;
    std::cout << " INPUT     " <<  g_input_  << std::endl;
    std::cout << " REQ  lon: " <<  g_lon_ << " lat: "<< g_lat_ << std::endl;
    std::cout << " DATA x: " <<  g_x_ << " y: "<< g_y_ << " z: " << g_z_ << std::endl;

    //YOGPSUtils gps;
    //gps.SetCoord(g_lat_, g_lon_, g_z_);
    //YOVector2I tile = gps.GetTileId();
    //YOVector2I pix = gps.GetImagePos();

   // std::cout << " TILE : " << tile << " IMAGE POS "  <<  pix << std::endl;

    return 0;

//    float xf = lon2xf(g_lon_, g_z_);
//    float yf = lat2yf(g_lat_, g_z_);
//    int x = (xf - floor(xf)) * 256;
//    int y = (yf - floor(yf)) * 256;
    //printf( "X,Y (%d, %d) \n" ,  x,  y);


    YONode node("ModelTest");
    node.advertise(g_topic_.c_str() , 0x0, 0x0);
    node.connect();

    while(node.isRunning())
    {
    	std::vector<unsigned char> out;
    	//fetch_tile_png(g_z_, g_x_, g_y_, out);
    	g_image_.format = YOFrameFormat::YO_PNG;
    	g_image_.height = 256;
    	g_image_.width = 256;
    	g_image_.size = out.size();
        YOMessage msg(g_image_);
        msg.setExtData(out.data(), out.size());

        printf("%010llu DATA %d \n", msg.getTimestamp(), msg.getExtDataSize());
        node.sendMessage(g_topic_.c_str(), msg);
        usleep(500000);
    }
}

