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

class YOMessage
{
    /*
     *  [TOPIC up 32 byets align right '0'][TS][INFO][DATA_A]
     *  or
     *  [TOPIC up 32 byets align right '0'][TS][INFO][DATA_A]   msg2 : [DATA_B]
     *
     * */

    friend class YONode;
    uint8_t m_topic_len;
    uint8_t m_topic_start;
    YOMessageData m_data;
    YOMessageData m_ext;
    YOHeaderBase *m_header_ptr;

    bool m_multi;
    bool m_received;
    bool m_shmem;
    void init();

public:

    YOMessage(); //empty message to receive

    template<typename T>
    YOMessage(const T &data) { //create message to send
        init();

        int data_size = sizeof(data);

        uint8_t *data_ptr = (uint8_t*) &data;
        msgpack::sbuffer buffer;
        YOMessageType data_type = YOMessageType::YOData;
        //std::cout << " CONSTRUCTOR " << std::endl;

        if constexpr (std::is_same_v<T, YOImageData>) {
            data_type = YOMessageType::YOImage;
        } else if constexpr (std::is_same_v<T, tCANData>) {
            data_type = YOMessageType::YOCAN;
        } else if constexpr (std::is_same_v<T, tCANFDData>) {
            data_type = YOMessageType::YOCANFD;
        } else if constexpr (std::is_same_v<T, YOVariant>) {
            //std::cout << " CONSTRUCTOR VARIANT " << std::endl;
            data_type = YOMessageType::YOConfig;
            msgpack::pack(buffer, data);
            data_size = buffer.size();
            data_ptr = (uint8_t*) buffer.data();
        } else {
            //std::cout << " CONSTRUCTOR DATA" << std::endl;
            //static_assert(dependent_false<T>::value, "Unknown meta type");
        }

        initData(data_ptr, data_size);
        m_header_ptr->type = 0xAABB;
        m_header_ptr->subtype = 0xCCDD;
    }

	virtual ~YOMessage();

	void setTimestamp(YOTimestamp ts);
	YOTimestamp getTimestamp();

    void setType(uint16_t type);
    uint16_t getType();

    void setSubType(uint16_t subtype);
    uint16_t getSubType();

    uint8_t *getData();
    uint32_t getDataSize();
    void initData(const uint8_t *data, uint32_t size); //COPYING TO A MESSAGE BUFFER
    uint8_t *initSize(uint32_t data_size);


    void setExtData(uint8_t *data, uint32_t size);
    uint8_t *getExtData();
    uint32_t getExtDataSize();
    void initExtData(uint8_t *data, uint32_t size);
    uint8_t *initExtSize(uint32_t size);

	void setTopic(const std::string &topic);
	const char *getTopic();

};

#endif /* UTILS_YOMESSAGE_H_ */
