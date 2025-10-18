/*
 * YOMessage.h
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#ifndef UTILS_YOMESSAGE_H_
#define UTILS_YOMESSAGE_H_

#include "YOVariant.h"
#include <iostream>

struct YOHeader //size = 16
{
    YOTimestamp timestamp;
    uint16_t type;
    uint16_t subtype;
    uint32_t size;
};

struct YOImageInfo {
	int64_t ts;
    uint16_t id;
    uint16_t width;
    uint16_t height;
    uint16_t stride;
    uint32_t size;
    YOFrameFormat format;
    uint8_t reserved[8];
};

struct YOHeaderImage
{
	YOHeader header;
	YOImageInfo image;
};


typedef void (yo_free_fn) (void *data_, void *hint_);

class YOMessage
{
    friend class YONode;
    struct YOMessageData;
    std::unique_ptr<YOMessageData> data_;
    void init();

public:

    YOMessage(); //empty message to receive
    YOMessage(YOImageInfo &image);
    YOMessage(const tCANData &can);
    YOMessage(const tCANFDData &canfd);
    YOMessage(const YOVariant &variant);
 	virtual ~YOMessage();

	void setTimestamp(YOTimestamp ts);
	YOTimestamp getTimestamp();

	YOHeader *getHeader();
	uint32_t getHeaderSize();

    void setType(uint16_t type);
    uint16_t getType();

    void setSubType(uint16_t subtype);
    uint16_t getSubType();

    uint8_t *getData();
    uint32_t getDataSize();

    void sendTo(const std::string &topic, void *socket);
    bool readFrom(void *sock_sub);

    void setData(const uint8_t *data, uint32_t size); //COPYING TO A MESSAGE BUFFER
    void initData(uint8_t *data, uint32_t size, yo_free_fn *fn, void *hint); //Zero Copy
    uint8_t *initSize(uint32_t data_size);

	void setTopic(const std::string &topic);
	const char *getTopic();
};

#endif /* UTILS_YOMESSAGE_H_ */
