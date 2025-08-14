/*
 * YOMessage.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#include "YOMessage.h"
#include <zmq.h>

void YOMessage::init()
{
    m_topic_len = 0;
    zmq_msg_init((zmq_msg_t*) &m_data.message);
    m_data.buffer = nullptr;
    m_data.size = 0;
    zmq_msg_init((zmq_msg_t*) &m_ext.message);
    m_ext.buffer = nullptr;
    m_ext.size = 0;

    m_header_ptr = nullptr;
    m_shmem = false;
    m_received = false;
    m_multi = false;
}

YOMessage::YOMessage()
{
    init();
}

YOMessage::~YOMessage()
{
    zmq_msg_close((zmq_msg_t*) &m_data.message);
    zmq_msg_close((zmq_msg_t*) &m_ext.message);
}

void YOMessage::setTimestamp(YOTimestamp ts)
{
    if(m_header_ptr)
        m_header_ptr->timestamp = ts;
}

YOTimestamp YOMessage::getTimestamp()
{
    return m_header_ptr ? m_header_ptr->timestamp : 0;
}

uint8_t *YOMessage::initSize(uint32_t data_size)
{
    std::cout << " INIT " << __FUNCTION__ << std::endl;
    m_data.size = YO_MAX_TOPIC_LENGTH + offsetof(YOHeaderBase, data) + data_size;
    std::cout << " m_data.size " << m_data.size << std::endl;
    m_data.buffer = (uint8_t*) malloc(m_data.size);
    m_header_ptr  = &((YOHeader*) m_data.buffer)->base;
    m_header_ptr->size = data_size;
    return m_header_ptr->data;
}

void YOMessage::initData(uint8_t *data, uint32_t size)
{
    //can not just set ptr and size. Need to alloc for a Topic, TS & Info and copy data
   initSize(size);
   memcpy(m_header_ptr->data, data, size);
}

uint8_t* YOMessage::getData()
{
    return m_header_ptr->data;//zmq_msg_data((zmq_msg_t*)m_msg_data);
}

uint32_t YOMessage::getDataSize()
{
    return m_header_ptr->size; // zmq_msg_size((zmq_msg_t*)m_msg_data);
}

void YOMessage::setType(uint16_t type)
{
    m_header_ptr->type = type;
}

uint16_t YOMessage::getType()
{
    return m_header_ptr->type;
}

void YOMessage::setSubType(uint16_t subtype)
{
    m_header_ptr->subtype = subtype;
}

uint16_t YOMessage::getSubType()
{
    return m_header_ptr->subtype;
}

const char *YOMessage::getTopic()
{
    return (const char*) m_data.buffer;
}

void YOMessage::setTopic(const char *topic)
{
    // [000...topic0][TS][INFO]
    m_topic_len = std::strlen(topic);
    m_topic_start = YO_MAX_TOPIC_LENGTH - m_topic_len - 1;
    memcpy(m_data.buffer + m_topic_start, topic, m_topic_len);
}

uint8_t *YOMessage::getExtData()
{
    return m_ext.buffer;
}

uint32_t YOMessage::getExtDataSize()
{
    return m_ext.size;
}

void YOMessage::initExtData(uint8_t *data, uint32_t size)
{
    if(m_header_ptr)
        m_header_ptr->ext_size = size;

    m_ext.buffer = data;
    m_ext.size = size;
}
uint8_t *YOMessage::initExtSize(uint32_t size)
{
    if(m_header_ptr)
        m_header_ptr->ext_size = size;

    m_ext.size = size;
    m_ext.buffer = (uint8_t*) malloc(size);
    return 0;
}

