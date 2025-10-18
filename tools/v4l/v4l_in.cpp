//#include "YOTypes.h"
#include <iostream>
#include "YONode.h"

#include <string>
#include <map>
#include <pthread.h>
#include <getopt.h>

#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

YONode *g_node;
std::string g_name = "V4L_NODE";
std::string g_topic = "VIDEO0";
std::string g_device = "/dev/video0";
std::string g_format = "JPEG";
std::string g_size = "640x480";
YOImageInfo g_image = {};

int g_width;
int g_height;

int g_fd = 0;
uint8_t *g_buffer;

std::map<std::string, int> g_v4l2_caps = {{"V4L2_CAP_VIDEO_CAPTURE", V4L2_CAP_VIDEO_CAPTURE},
{"V4L2_CAP_VIDEO_OUTPUT" ,V4L2_CAP_VIDEO_OUTPUT},
{"V4L2_CAP_VIDEO_OVERLAY" ,V4L2_CAP_VIDEO_OVERLAY},
{"V4L2_CAP_VBI_CAPTURE" ,V4L2_CAP_VBI_CAPTURE},
{"V4L2_CAP_VBI_OUTPUT" ,V4L2_CAP_VBI_OUTPUT},
{"V4L2_CAP_SLICED_VBI_CAPTURE" ,V4L2_CAP_SLICED_VBI_CAPTURE},
{"V4L2_CAP_SLICED_VBI_OUTPUT" ,V4L2_CAP_SLICED_VBI_OUTPUT},
{"V4L2_CAP_VIDEO_OUTPUT_OVERLAY" ,V4L2_CAP_VIDEO_OUTPUT_OVERLAY},
{"V4L2_CAP_HW_FREQ_SEEK" ,V4L2_CAP_HW_FREQ_SEEK},
{"V4L2_CAP_RDS_OUTPUT" ,V4L2_CAP_RDS_OUTPUT},
{"V4L2_CAP_HW_FREQ_SEEK" ,V4L2_CAP_HW_FREQ_SEEK},

{"V4L2_CAP_VIDEO_CAPTURE_MPLANE" ,V4L2_CAP_VIDEO_CAPTURE_MPLANE},
{"V4L2_CAP_VIDEO_OUTPUT_MPLANE" ,V4L2_CAP_VIDEO_OUTPUT_MPLANE},
{"V4L2_CAP_VIDEO_M2M_MPLANE" ,V4L2_CAP_VIDEO_M2M_MPLANE},
{"V4L2_CAP_VIDEO_M2M" ,V4L2_CAP_VIDEO_M2M},

{"V4L2_CAP_TUNER" ,V4L2_CAP_TUNER},
{"V4L2_CAP_AUDIO" ,V4L2_CAP_AUDIO},
{"V4L2_CAP_RADIO" ,V4L2_CAP_RADIO},
{"V4L2_CAP_MODULATOR" ,V4L2_CAP_MODULATOR},

{"V4L2_CAP_SDR_CAPTURE" ,V4L2_CAP_SDR_CAPTURE},
{"V4L2_CAP_EXT_PIX_FORMAT" ,V4L2_CAP_EXT_PIX_FORMAT},
{"V4L2_CAP_SDR_OUTPUT" ,V4L2_CAP_SDR_OUTPUT},
{"V4L2_CAP_META_CAPTURE" ,V4L2_CAP_META_CAPTURE},

{"V4L2_CAP_READWRITE" ,V4L2_CAP_READWRITE},
{"V4L2_CAP_STREAMING" ,V4L2_CAP_STREAMING},
{"V4L2_CAP_META_OUTPUT" ,V4L2_CAP_META_OUTPUT},

{"V4L2_CAP_TOUCH" ,V4L2_CAP_TOUCH},
{"V4L2_CAP_IO_MC" ,V4L2_CAP_IO_MC},

{"V4L2_CAP_DEVICE_CAPS" ,V4L2_CAP_DEVICE_CAPS}};


void split(const std::string &line, int &width, int &height)
{
    int start = 0u;
    int end = line.find("x");
    width = atoi(line.substr(start, end).c_str());
    height = atoi(line.substr(end + 1, line.length() - end - 1).c_str());
}

void init_device(int &fd, const std::string &device)
{
    if((fd = open(device.c_str(), O_RDWR | O_NONBLOCK)) < 0)
    {
        perror((device + "open").c_str());
        exit(1);
    }

    struct v4l2_capability cap = {0};
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
        perror("VIDIOC_QUERYCAP");
    else
        printf("[%s] v4l2_capability 0x%08X\n", device.c_str(), cap.capabilities);

    for(auto &cap_pair : g_v4l2_caps)
    {
        if((cap.capabilities & cap_pair.second) )
            std::cout <<cap_pair.first << ": 1 " << std::endl;
    }

    struct v4l2_format fmt = {0};
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.width  = g_width;
    fmt.fmt.pix.height = g_height;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
        perror("VIDIOC_S_FMT");
    else
        printf("[%s] set format V4L2_PIX_FMT_MJPEG %dx%d\n", device.c_str(), g_width, g_height);

    struct v4l2_requestbuffers req = {0};
    req.count  = 3;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
        perror("VIDIOC_REQBUFS");
    else
        printf("[%s] set request V4L2_MEMORY_MMAP\n", device.c_str());

    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
        perror("VIDIOC_QUERYBUF");
    else
        printf("[%s] created buffer len: %d\n", device.c_str(), buf.length);

    g_buffer = (uint8_t*)mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED , fd, buf.m.offset);
    if (g_buffer == MAP_FAILED)
        perror("mmap");
    else
        printf("[%s] created buffer used: %d  addr: %p\n", device.c_str(), buf.bytesused, g_buffer);

}


int capture_image(int fd)
{
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("Query Buffer");
        return 1;
    }

    if(-1 == ioctl(fd, VIDIOC_STREAMON, &buf.type))
    {
        perror("Start Capture");
        return 1;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 2;
    int r = select(fd+1, &fds, NULL, NULL, &tv);
    if(-1 == r)
    {
        perror("Waiting for Frame");
        return 1;
    }

    if(-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
    {
        perror("Retrieving Frame");
        return 1;
    }

    printf("\n[%s] image size: %d\n", g_device.c_str(),  buf.bytesused);
    //printf("timestamp: %d.%d\n", buf.timestamp.tv_sec, buf.timestamp.tv_usec);
    g_image.id++;
    g_image.ts = YONode::getTimestamp();
    g_image.size =  buf.bytesused;
    YOMessage msg(g_image);
    msg.setData(g_buffer, buf.bytesused);
    g_node->sendMessage(g_topic.c_str(), msg);


/*
    printf("index: %d  type: %d used: %d flags: %d field: %d \n", buf.index, buf.type, buf.bytesused, buf.flags, buf.field );

    printf("timecode: type: %d flags: %d\n", buf.timecode.type, buf.timecode.flags);
    printf("timecode: %02d:%02d:%02d.%d\n", buf.timecode.hours, buf.timecode.minutes, buf.timecode.seconds,buf.timecode.frames);
*/

/*
    IplImage* frame;
    CvMat cvmat = cvMat(480, 640, CV_8UC3, (void*)buffer);
    frame = cvDecodeImage(&cvmat, 1);
    cvNamedWindow("window",CV_WINDOW_AUTOSIZE);
    cvShowImage("window", frame);
    cvWaitKey(0);
    cvSaveImage("image.jpg", frame, 0);
*/

    return 0;
}

void close_device(int &fd)
{
    close(fd);
}

static struct option long_options[] = {
    { "name", optional_argument, NULL, 'n' },
    { "topic", required_argument, NULL, 't' },
    { "device", required_argument, NULL, 'd' },
    { "format", required_argument, NULL, 'f' },
    { "size", required_argument, NULL, 's' },
    { NULL, 0, NULL, 0 } };

int main(int argc, char **argv)
{
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "n:t:i:f:s:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'n':
                g_name = optarg;
                break;
            case 't':
                g_topic = optarg;
                break;
            case 'd':
                g_device = optarg;
                break;
            case 'f':
                g_format = optarg;
                break;
            case 's':
                g_size = optarg;
                break;
        }
    }

    std::cout << "g_name = " << g_name << std::endl;
    std::cout << "g_topic = " << g_topic << std::endl;
    std::cout << "g_device = " << g_device<< std::endl;
    std::cout << "g_format = " << g_format << std::endl;
    std::cout << "g_size = " << g_size << std::endl;

    split(g_size, g_width, g_height);
    std::cout << "g_width = " << g_width << std::endl;
    std::cout << "g_height = " << g_height << std::endl;

    init_device(g_fd, g_device.c_str());

    //g_image.data_type_ = YO_IMAGE;
    g_image.format = YOFrameFormat::YO_JPEG;
    g_image.width = g_width;
    g_image.height = g_height;

    g_node = new YONode(g_name.c_str());
    g_node->advertise(g_topic.c_str());
    g_node->connect();
    std::cout << "Connected " << std::endl;
    while( g_node->isRunning() && !capture_image(g_fd))
    {

    }
    close_device(g_fd);


    return 0;
}
