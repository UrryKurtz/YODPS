/*
 * YOMessage.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */
#include <zmq.h>

#include "YOMessage.h"

struct YOMessage::YOMessageData
{
	zmq_msg_t topic;
	zmq_msg_t header;
	zmq_msg_t data;
};

YOMessage::YOMessage()
{
	data_ = std::make_unique<YOMessageData>();
	zmq_msg_init(&data_->topic);
	zmq_msg_init_size(&data_->header, sizeof(YOHeader));
	zmq_msg_init(&data_->data);
}

YOMessage::YOMessage(YOImageInfo &image)
{
	data_ = std::make_unique<YOMessageData>();
	zmq_msg_init(&data_->topic);
	zmq_msg_init_size(&data_->header, sizeof(YOHeader) + sizeof(image));
	uint8_t *base = (uint8_t *) zmq_msg_data(&data_->header);
	size_t hdr = sizeof(YOHeader);
	memcpy(base  + hdr, &image, sizeof(image));
	zmq_msg_init(&data_->data);
	setType(4);
}

YOMessage::YOMessage(const tCANData &can)
{
	data_ = std::make_unique<YOMessageData>();
	zmq_msg_init(&data_->topic);
	zmq_msg_init_size(&data_->header, sizeof(YOHeader));
	zmq_msg_init_size(&data_->data, sizeof(can));
	memcpy(zmq_msg_data(&data_->data), &can, sizeof(can));
	setType(512);
}

YOMessage::YOMessage(const tCANFDData &canfd)
{
	data_ = std::make_unique<YOMessageData>();
	zmq_msg_init(&data_->topic);
	zmq_msg_init_size(&data_->header, sizeof(YOHeader));
	zmq_msg_init_size(&data_->data, sizeof(canfd));
	memcpy(zmq_msg_data(&data_->data), &canfd, sizeof(canfd));
	setType(512);
	setSubType(3);
}

void free_msgpak(void* /*data*/, void* hint)
{
	msgpack::sbuffer* sb = (msgpack::sbuffer*)(hint);
	delete sb;
}

YOMessage::YOMessage(const YOVariant &variant)
{
	data_ = std::make_unique<YOMessageData>();
	zmq_msg_init(&data_->topic);
    zmq_msg_init_size(&data_->header, sizeof(YOHeader));
    msgpack::sbuffer *buffer = new msgpack::sbuffer();
    msgpack::pack(*buffer, variant);
    zmq_msg_init_data(&data_->data, buffer->data(), buffer->size(), free_msgpak, buffer);
    setType(4096);
}

YOMessage::~YOMessage()
{
	zmq_msg_close(&data_->topic);
    zmq_msg_close(&data_->header);
    zmq_msg_close(&data_->data);
}

void YOMessage::init()
{
}

void YOMessage::setTimestamp(YOTimestamp ts)
{
      getHeader()->timestamp = ts;
}

YOTimestamp YOMessage::getTimestamp()
{
    return getHeader()->timestamp;
}

uint8_t *YOMessage::initSize(uint32_t size)
{
	zmq_msg_init_size(&data_->data, size);
	return (uint8_t*)zmq_msg_data(&data_->data);
}

void YOMessage::initData(uint8_t *data, uint32_t size, yo_free_fn *fn, void *hint)
{
	zmq_msg_init_data(&data_->data, data, size, fn, hint);
}

void YOMessage::setData(const uint8_t *data, uint32_t size)
{
	zmq_msg_init_size(&data_->data, size);
	memcpy(zmq_msg_data(&data_->data), data, size);
}

uint8_t* YOMessage::getData()
{
    return (uint8_t*) zmq_msg_data(&data_->data);
}

uint32_t YOMessage::getDataSize()
{
	return zmq_msg_size(&data_->data);
}

YOHeader *YOMessage::getHeader()
{
	return (YOHeader *) zmq_msg_data(&data_->header);
}

uint32_t YOMessage::getHeaderSize()
{
	return zmq_msg_size(&data_->header);
}

void YOMessage::setType(uint16_t type)
{
    getHeader()->type = type;
}

uint16_t YOMessage::getType()
{
    return getHeader()->type;
}

void YOMessage::setSubType(uint16_t subtype)
{
    getHeader()->subtype = subtype;
}

uint16_t YOMessage::getSubType()
{
	return getHeader()->subtype;
}

const char *YOMessage::getTopic()
{
    return (const char*)  zmq_msg_data(&data_->topic);
}

void YOMessage::setTopic(const std::string &topic)
{
	zmq_msg_init_size(&data_->topic, topic.size() + 1);
    memcpy(zmq_msg_data(&data_->topic), topic.c_str(), topic.size() + 1);
}

void YOMessage::sendTo(const std::string &topic, void *socket)
{
	setTopic(topic);
	zmq_msg_send(&data_->topic, socket, ZMQ_SNDMORE);
	zmq_msg_send(&data_->header, socket, ZMQ_SNDMORE);
	zmq_msg_send(&data_->data, socket, 0);
}

bool YOMessage::readFrom(void *sock_sub)
{
	size_t more_size = sizeof(int);
	int more_hdr;
	zmq_msg_recv(&data_->topic, sock_sub, 0);
	zmq_getsockopt(sock_sub, ZMQ_RCVMORE, &more_hdr, &more_size);
	if (!more_hdr)
	{
		std::cout << " Error reading ZMQ Message " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
		return false;
	}

	int more_data;
	zmq_msg_recv(&data_->header, sock_sub, 0);

	zmq_getsockopt(sock_sub, ZMQ_RCVMORE, &more_data, &more_size);
	if (!more_data)
	{
		std::cout << " Error reading ZMQ Message " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
		return false;
	}
	zmq_msg_recv(&data_->data, sock_sub, 0);
	return true;
}
