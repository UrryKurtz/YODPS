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

uint16_t g_counter = 0;

void create_frame()
{
    //std::cout  << " create_frame: !!! " << std::endl;
    frame[yo::k::objects] = YOArray(32);

    for(uint32_t i = 0; i < 32 ; i++)
    {
        frame[yo::k::objects][i][yo::k::color][yo::k::line] = convert(colors[i]);
        frame[yo::k::objects][i][yo::k::color][yo::k::fill] = YOColor4F{0.3f, 0.3f, 0.3f, 0.5f};
        frame[yo::k::objects][i][yo::k::color][yo::k::text] = YOColor4F{1.0f, 1.0f, 1.0f, 0.8f};
        frame[yo::k::objects][i][yo::k::type] = (int) YOPointCloud;
        frame[yo::k::objects][i][yo::k::style_id] = i;
        frame[yo::k::objects][i][yo::k::overlay] = false;
        frame[yo::k::objects][i][yo::k::vertices] = YOVector3List();
    }

    frame[yo::k::sender] = "Velodyne decoder";
    frame[yo::k::enabled] = true;
    frame[yo::k::frame_id] = g_counter++;

    frame[yo::k::transform][yo::k::position] = YOVector3{0, 0, 0};
    frame[yo::k::transform][yo::k::rotation] = YOVector3{0, 0, 0};
    frame[yo::k::transform][yo::k::scale] = YOVector3{1.0f, 1.0f, 1.0f};
    frame[yo::k::colors] = colors;
}

int fn_hdl32(const std::string &topic, std::shared_ptr<YOMessage> message, void *param)
{
    //std::cout  << topic << " fn_hdl32 !!! data size: "  << message->getDataSize() << std::endl;
    if (frame[yo::k::objects].getArraySize() == 0)
    {
        create_frame();
    }
    //std::cout  << topic << " A: " << std::endl;

    LidarData *data = (LidarData*) message->getData();

    for (int i = 0; i < 12; i++)
    {
        //static uint16_t last_ang = data->Data[i].header.Rotation;
//        if (data->Data[i].header.Rotation - last_ang > 30)
//            std::cout << i << " +++ rot " << data->Data[i].header.Rotation << " PREV: " << last_ang << " DIFF: " << data->Data[i].header.Rotation - last_ang << std::endl;

        //if(data->Data[i].header.Rotation - last_ang  > 30)
        //  std::cout <<  "+++" <<  std::endl;

        uint8_t clr = (int) data->Data[i].header.Rotation / 1000;
        //std::cout <<  "COLOR " << (int ) clr <<  std::endl;
        if (data->Data[i].header.Rotation < last_ang) //&& (last_ang - data->Data[i].header.Rotation) > 35900
        {
            //std::cout << "----------------------------------------------------------------------------------- " << std::endl;
            //frame.print();

            YOMessage msg(frame);
            g_node->sendMessage("INPUT0", msg);

            create_frame();
            //frame = YOVariant("Frame Cloud #0");
            //frame["Objects"].Resize(32);

            //std::cout << "---------- NEW MEASURMENT -------------------- SEND CLOUD " << str.size() << std::endl;
        }
        //std::cout << " " << data->Data[i].header.LaserBlockID <<  " rot: " << data->Data[i].header.Rotation << std::endl;
        //static
        float rot = 0.000174533f * data->Data[i].header.Rotation;

        for (int j = 0; j < 32; j++)
        {
            frame[yo::k::objects][j][yo::k::type_id] = j;

            if (data->Data[i].laser[j].Distance)
            {
                //std::vector<uint8_t> &pointsd = frame["Objects"][data->Data[i].laser[j].Intencity / 8]["Vertices"].GetData();
                //std::vector<float> &points = (std::vector<float>&) frame["Objects"][data->Data[i].laser[j].Intencity / 8]["Vertices"].GetData();
                //std::vector<float> &points = (std::vector<float> &) frame["Objects"][j]["Vertices"].GetData();
                int id = data->Data[i].laser[j].Intencity / 8;
                float r = 0.002f * data->Data[i].laser[j].Distance;
                float x = r * cos(angles[j]) * cos(rot);
                float y = r * cos(angles[j]) * sin(rot);
                float z = r * sin(angles[j]) + 2.2f;

                YOVector3List &list = frame[yo::k::objects][id][yo::k::vertices];
                //std::cout << "LIST size " << list.size() << std::endl;
                list.push_back(YOVector3{x,y,z});
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
        angles[i] = 0.0174533f * angles[i];

    g_node = new YONode(g_type.c_str());
    g_node->advertise(g_output.c_str());
    g_node->connect();
    g_node->subscribe(g_input.c_str(), fn_hdl32, 0);
    g_node->start();

    return 0;
}
