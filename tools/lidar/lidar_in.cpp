#include <YOKeys.h>
#include <YOTypes.h>

#include <iostream>
#include <pthread.h>
#include "YONode.h"
#include <getopt.h>

YONode *g_node;
std::string g_name = "LIDAR_NODE";
std::string g_input = "HDL32";
std::string g_output = "INPUT0";
std::string g_type = "HDL32";

float angles[] = {
        -30.67f, -9.33f, -29.33f, -8.00f,
        -28.00f, -6.66f, -26.66f, -5.33f,
        -25.33f, -4.00f, -24.00f, -2.67f,
        -22.67f, -1.33f, -21.33f, 0.00f,
        -20.00f, 1.33f, -18.67f, 2.67f,
        -17.33f, 4.00f, -16.00f, 5.33f,
        -14.67f, 6.67f, -13.33f, 8.00f,
        -12.00f, 9.33f, -10.67f, 10.67f
};

float sins[32];
float coss[32];

#pragma pack(1)

struct Laser
{
    uint16_t Distance;
    uint8_t Intencity;
};

struct FiringHeader
{
    uint16_t LaserBlockID;
    uint16_t Rotation;
};

struct LidarStatus
{
    uint32_t GPSTimestamp;
    uint8_t StatusType;
    uint8_t StatusValue;
};

struct FiringData
{
    FiringHeader header;
    Laser laser[32];
};

struct LidarData
{
    FiringData Data[12];
    LidarStatus Status;
};

//#pragma pop(pack)

YOColor4CList colors = {
        {32, 0, 255, 255}, {32, 16, 240, 255}, {32, 32, 224, 255}, {32, 48, 208, 255},
        {32, 64, 192, 255}, {32, 80, 176, 255}, {32, 96, 160, 255}, {32, 112, 144, 255},
        {32, 128, 128, 255}, {32, 144, 112, 255}, {32, 160, 96, 255}, {32, 176, 80, 255},
        {32, 192, 64, 255}, {32, 208, 48, 255}, {32, 224, 32, 255}, {32, 240, 16, 255},
        {255, 255, 32, 255}, {255, 240, 32, 255}, {255, 224, 32, 255}, {255, 208, 32, 255},
        {255, 192, 32, 255}, {255, 176, 32, 255}, {255, 160, 32, 255}, {255, 144, 32, 255},
        {255, 128, 32, 255}, {255, 112, 32, 255}, {255, 96, 32, 255}, {255, 80, 32, 255},
        {255, 64, 32, 255}, {255, 48, 32, 255}, {255, 32, 32, 255}, {255, 16, 32, 255}};

static uint16_t last_ang = 0;
static YOVariant frame("Frame Cloud #0");

//static YOVector3List *vert_list[16];
static YOFloatList *vert_list[16];

uint16_t g_counter = 0;

void create_frame()
{
    //std::cout  << " create_frame: !!! " << std::endl;
    frame[yo::k::objects] = YOArray(1);

    YOVariant &cloud = frame[yo::k::objects][0];
    cloud[yo::k::object_type] = YOObjectType::YOGeomery;

    cloud[yo::k::geometries] = YOArray(16);
    YOVariant &geoms = cloud[yo::k::geometries];

    for(uint32_t i = 0; i < 16 ; i++)
    {
        YOVariant &geom = geoms[i];
        YOVariant &color = geom[yo::k::color];
        color[yo::k::line] = convert(colors[i * 2]);
        color[yo::k::fill] = YOColor4F{0.3f, 0.3f, 0.3f, 0.5f};
        //color[yo::k::text] = YOColor4F{1.0f, 1.0f, 1.0f, 0.8f};
        geom[yo::k::geometry_type] = YOGeomType::YOPointList;
        geom[yo::k::style_id] = i;
        geom[yo::k::overlay] = false;
        //geom[yo::k::vertices] = YOVector3List();
        //vert_list[i] = &geom[yo::k::vertices].get<YOVector3List>();
        geom[yo::k::vertices] = YOFloatList();
        vert_list[i] = &geom[yo::k::vertices].get<YOFloatList>();
        vert_list[i]->reserve(100000);
    }

    frame[yo::k::sender] = "Velodyne decoder";
    frame[yo::k::enabled] = true;
    frame[yo::k::frame_id] = g_counter++;

    YOVariant &transform = frame[yo::k::transform];
    transform[yo::k::position] = YOVector3{0, 0, 0};
    transform[yo::k::rotation] = YOVector3{0, 0, 0};
    transform[yo::k::scale] = YOVector3{1.0f, 1.0f, 1.0f};
    frame[yo::k::colors] = colors;
}

int fn_hdl32(const std::string &topic, std::shared_ptr<YOMessage> message, void *param)
{
    //std::cout  << topic << " fn_hdl32 !!! data size: "  << message->getDataSize() << std::endl;
    if (frame[yo::k::objects].getArraySize() == 0)
    {
        create_frame();
    }

    LidarData *data = (LidarData*) message->getData();

    for (int i = 0; i < 12; i++)
    {
        FiringData *fd = &data->Data[i];
        if (fd->header.Rotation < last_ang) //&& (last_ang - data->Data[i].header.Rotation) > 35900
        {
            YOMessage msg(frame);
            g_node->sendMessage(g_output.c_str(), msg);
            create_frame();
        }
        float rot = 0.000174533f * fd->header.Rotation;

        for (int j = 0; j < 32; j++)
        {
            Laser *laser = &fd->laser[j];
            if (laser->Distance)
            {
            	//std::cout << " "  << laser->Intencity << std::endl;
                int id = (int)log2((laser->Intencity * sqrt(laser->Intencity) + 1)); //laser->Intencity / 16;log(laser->Intencity);
                //std::cout << id << " "  << (int) laser->Intencity << std::endl;
                float r = 0.002f * laser->Distance;
                float rcos = r * coss[j];
                float x = rcos * cos(rot);
                float y = rcos * sin(rot);
                float z = r * sins[j];

                //YOVector3List *list = vert_list[id];
                //list->push_back(YOVector3{x,y,z});

                YOFloatList *list = vert_list[id];
                list->push_back(x);
                list->push_back(y);
                list->push_back(z);
            }
        }
        last_ang = data->Data[i].header.Rotation;
    }
    return 0;
}

static struct option long_options[] = {
        { "name", optional_argument, NULL, 'n' },
        { "input", required_argument, NULL, 'i' },
        { "output", required_argument, NULL, 'o' },
        { "type", optional_argument, NULL, 't' },
        { NULL, 0, NULL, 0 } };

int main(int argc, char **argv)
{
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "n:i:o:t:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'n':
            g_name = optarg;
            break;
        case 'i':
            g_input = optarg;
            break;
        case 'o':
            g_output = optarg;
            break;
        case 't':
            g_type = optarg;
            break;
        }
    }

    std::cout << "g_name = " << g_name << std::endl;
    std::cout << "g_input = " << g_input << std::endl;
    std::cout << "g_output = " << g_output << std::endl;
    std::cout << "g_type = " << g_type << std::endl;

    create_frame();

    for (int i = 0; i < 32; i++)
    {
        angles[i] = 0.0174533f * angles[i];
        sins[i] = sin(angles[i]);
        coss[i] = cos(angles[i]);
    }

    g_node = new YONode(g_type.c_str());
    g_node->advertise(g_output.c_str());
    g_node->connect();
    g_node->subscribe(g_input.c_str(), fn_hdl32, 0);
    g_node->start();

    return 0;
}
