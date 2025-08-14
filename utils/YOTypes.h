/*
 * YOTypes.h
 *
 *  Created on: Jun 8, 2025
 *      Author: kurtz
 */

#ifndef UTILS_YOTYPES_H_
#define UTILS_YOTYPES_H_
#define MSGPACK_USE_STD_VARIANT_ADAPTOR
#include <msgpack.hpp>

struct YOIPv4 {
    uint8_t ip[4];
    uint16_t port;
    MSGPACK_DEFINE(ip, port);
};

struct YOIPv6 {
    uint8_t ip[6];
    uint16_t port;
    MSGPACK_DEFINE(ip, port);
};

struct YOVector2I {
    int32_t x, y;
    MSGPACK_DEFINE(x, y);
};

struct YOVector2U {
    uint32_t x, y;
    MSGPACK_DEFINE(x, y);
};

struct YOVector2 {
    float x, y;
    MSGPACK_DEFINE(x, y);
};

struct YOVector3 {
    float x, y, z;
    MSGPACK_DEFINE(x, y, z);
};

struct YOVector4 {
    float x, y, z, w;
    MSGPACK_DEFINE(x, y, z, w);
};

struct YOColor3F {
    float r, g, b;
    MSGPACK_DEFINE(r, g, b);
};

struct YOColor4F {
    float r, g, b, a;
    MSGPACK_DEFINE(r, g, b, a);
};

struct YOColor3C {
    uint8_t r, g, b;
    MSGPACK_DEFINE(r, g, b);
};

struct YOColor4C {
    uint8_t r, g, b, a;
    MSGPACK_DEFINE(r, g, b, a);
};

using YOData = std::vector<uint8_t>;
using YODataF = std::vector<float>;

using YOVector2IList = std::vector<YOVector2I>;
using YOVector2UList = std::vector<YOVector2U>;

using YOVector2List = std::vector<YOVector2>;
using YOVector3List = std::vector<YOVector3>;
using YOVector4List = std::vector<YOVector4>;
using YOColor3FList = std::vector<YOColor3F>;
using YOColor4FList = std::vector<YOColor4F>;
using YOColor3CList = std::vector<YOColor3C>;
using YOColor4CList = std::vector<YOColor4C>;

//31 + '0'
#define YO_MAX_TOPIC_LENGTH 32u

using YOTimestamp = int64_t;

enum class YOMessageType : uint32_t
{
    YOData,
    YOConfig,
    YOShmem,
    YOImage,
    YOCAN,
    YOCANFD
};

enum class YOFrameFormat : uint16_t
{
    YO_Unknown,
    YO_JPEG,
    YO_PNG,
    YO_APNG,
    YO_BMP,
    YO_TIFF,
    YO_WEBP,
    YO_GIF,
    YO_HEIC,
    YO_EPS,
    YO_SVG,
    YO_PDF,
    YO_MP4,
    YO_H264,
    YO_H265,
    YO_HEVC,
    YO_GRAY8,
    YO_GRAY12,
    YO_RGB_888,
    YO_BGR_888,
    YO_RGBA_8888,
    YO_ARGB_8888,
    YO_YUYV422,
    YO_UYVY422,
    YO_NV12,
    YO_YUV444P,
    YO_YUV422P,
    YO_YUV420P,
};

struct tMessageHeader
{
    uint8_t      ui8Tag;
    uint8_t      ui8Channel;
    int64_t      tmTimeStamp;
};

struct tData
{
    uint32_t     ui32Id;
    uint8_t      ui8Length;
    uint8_t      ui8Reserved;
    uint16_t     ui16Flags;
    uint16_t     ui16Reserved;
    uint32_t     ui32Reserved;
    uint8_t      aui8Data[8];
};

struct tDataFD
{
    uint32_t     ui32Id;
    uint8_t      ui8Length;
    uint8_t      ui8Reserved;
    uint16_t     ui16Flags;
    uint16_t     ui16Reserved;
    uint32_t     ui32Reserved;
    uint8_t      aui8Data[64];
};

struct tCANData
{
    tMessageHeader sHeader;
    tData sData;
};

struct tCANFDData
{
    tMessageHeader sHeader;
    tDataFD sData;
};

struct YOImageData {
    YOFrameFormat format;
    uint16_t width;
    uint16_t height;
    uint16_t stride;
    uint8_t reserved[8];
};

struct YOHeaderBase //size = 16
{
    YOTimestamp timestamp;
    uint16_t type;
    uint16_t subtype;
    uint32_t size;
    uint32_t ext_size;
    uint8_t data[0];
};

struct YOMessageData
{
    uint8_t  message[64]; //TODO make abstract
    uint8_t *buffer; //message data starts
    uint32_t size; //the whole buffer size
};

struct YOHeader //size = 48
{
    uint8_t topic[YO_MAX_TOPIC_LENGTH];
    YOHeaderBase base;
};

#define YO_SUB_SRV "tcp://127.0.0.1:5550"
#define YO_PUB_SRV "tcp://127.0.0.1:5551"


#endif /* UTILS_YOTYPES_H_ */
