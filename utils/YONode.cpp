/*
 * YONode.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#include <zmq.h>
#include <cstdint>
#include "YONode.h"

YONode *g_node_ = nullptr;

volatile sig_atomic_t stop_flag = 0;

void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        std::cout << "\nCTRL+C caught!\n";
        stop_flag = 1;
    }

    if(YOSigData *sig_data = g_node_->getSignalFunction(signum))
    {
        std::cout << "\nCALLING FN FOR " << signum << "\n";
        sig_data->fn(signum, sig_data->data);
    }
}

void free_data_fn(void *data_, void *hint_)
{
    //std::cout << " free_data_fn!!! " << std::endl;
    delete (uint8_t*) hint_;
}

bool YONode::isRunning()
{
    return !stop_flag;
}

YONode::YONode(const char *node_name)
{
    g_node_ = this;
    signal(SIGINT, signal_handler);
    m_poll = malloc(sizeof(zmq_pollitem_t) * 2);
    m_context = zmq_ctx_new();
    m_socket_sub = zmq_socket(m_context, ZMQ_SUB);
    m_socket_pub = zmq_socket(m_context, ZMQ_PUB);

    int size = 128 * 1024 * 1024; // 128 MB
    zmq_setsockopt(m_socket_sub, ZMQ_RCVBUF, &size, sizeof(size));
    zmq_setsockopt(m_socket_sub, ZMQ_SNDBUF, &size, sizeof(size));

    zmq_setsockopt(m_socket_pub, ZMQ_RCVBUF, &size, sizeof(size));
    zmq_setsockopt(m_socket_pub, ZMQ_SNDBUF, &size, sizeof(size));

    int hwm = 100000; // 100k сообщений
    zmq_setsockopt(m_socket_sub, ZMQ_RCVHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(m_socket_sub, ZMQ_SNDHWM, &hwm, sizeof(hwm));

    zmq_setsockopt(m_socket_pub, ZMQ_RCVHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(m_socket_pub, ZMQ_SNDHWM, &hwm, sizeof(hwm));


    /*
    sysctl net.core.rmem_max
    sysctl net.core.wmem_max
    sudo sysctl -w net.core.rmem_max=134217728   # 128 MB
    sudo sysctl -w net.core.wmem_max=134217728
*/
    m_poll = malloc(sizeof(zmq_pollitem_t) * 2);
    zmq_pollitem_t *items = (zmq_pollitem_t*) m_poll;
    items[0] = {m_socket_sub, 0, ZMQ_POLLIN, 0};
    items[1] = {m_socket_pub, 0, ZMQ_POLLIN, 0};
}

YONode::~YONode()
{
    g_node_ = nullptr;
    delete (zmq_pollitem_t*) m_poll;
}

void YONode::setConfig(YOVariant &config)
{
    m_config = config;
}

YOVariant& YONode::getConfig()
{
    return m_config;
}

void YONode::setUserData(const char *name, void *data)
{
    m_user_data[name] = data;
}

void* YONode::getUserData(const char *name)
{
    return m_user_data[name];
}

YOTimestamp YONode::getTimestamp()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

void YONode::addSignalFunction(int signal, YOSigFn fn, void *data)
{
    m_sig_map[signal] = {signal, fn, data};
}
YOSigData *YONode::getSignalFunction(int signal)
{
    auto it = m_sig_map.find(signal);
    if (it != m_sig_map.end()) {
        return &it->second;
    }
    return nullptr;
}

void YONode::sendMessage(const char *topic, YOMessage &message)
{
    //TODO check first message size, initSize(0);
    auto stream = m_pub_map.find(topic);
    if (stream == m_pub_map.end())
    {
        std::cout << " SendMessage: " << topic << " is not advertised " << std::endl;
        return;
    }
    //std::cout << " SendMessage: " << topic << std::endl;

    message.setTopic(topic);
    message.setTimestamp(YONode::getTimestamp());
    message.setType(stream->second.type);
    message.setSubType(stream->second.subtype);

    int flag = message.getExtDataSize() ? ZMQ_SNDMORE : 0;
    int new_size = message.m_data.size - message.m_topic_start;
    zmq_msg_init_data((zmq_msg_t*) &message.m_data.message, message.m_data.buffer + message.m_topic_start, new_size, free_data_fn, message.m_data.buffer);
    zmq_msg_send((zmq_msg_t*) &message.m_data.message, m_socket_pub, flag);

    if (flag != 0)
    {
        zmq_msg_init_data((zmq_msg_t*) &message.m_ext.message, message.m_ext.buffer, message.m_ext.size, free_data_fn, message.m_ext.buffer);
        zmq_msg_send((zmq_msg_t*) &message.m_ext.message, m_socket_pub, 0);
        //std::cout << " SENT EXT " << std::endl;
    }
}

void YONode::subscribe(const char *topic, YOSubFn fn, void *data)
{
    std::cout << " Subscribe fn to topic [" << topic << "]" << std::endl;
    m_sub_map[topic] = {topic, fn, 0, data};
}

void YONode::subscribe(const char *topic, YOSubSharedFn fn, void *data)
{
    std::cout << " Subscribe shared fn to topic [" << topic << "]" << std::endl;
    m_sub_map[topic] = {topic, 0, fn, data};
}

void YONode::unsubscribe(const char *topic)
{
    m_sub_map.erase(topic);
    zmq_setsockopt(m_socket_sub, ZMQ_SUBSCRIBE, topic, std::strlen(topic));
}

void YONode::advertise(const char *topic, uint16_t type, uint16_t subtype)
{
    m_pub_map[topic] = {type, subtype};
    printf("Advertise topic [%s] type: %02X subtype: %02X  \n", topic, type, subtype);
}

int YONode::connect()
{
    for (auto topics : m_sub_map)
    {
        zmq_setsockopt(m_socket_sub, ZMQ_SUBSCRIBE, topics.first.c_str(), topics.first.size());
        std::cout << " Subscribed to " << topics.first.c_str() << std::endl;
    }

    zmq_connect(m_socket_sub, YO_PUB_SRV);
    zmq_connect(m_socket_pub, YO_SUB_SRV);

    usleep(2000);
    std::cout << " Connected " << std::endl;
    return 0;
}

int YONode::disconnect()
{
    zmq_disconnect(m_socket_sub, YO_PUB_SRV);
    zmq_disconnect(m_socket_pub, YO_SUB_SRV);
    return 0;
}

int YONode::shutdown()
{
    zmq_close(m_socket_sub);
    zmq_close(m_socket_pub);
    zmq_ctx_shutdown(m_context);
    zmq_ctx_term(m_context);
    return 0;
}

int YONode::stop()
{
    stop_flag = 1;
    return 0;
}

int YONode::getMessage(int wait)
{

    zmq_pollitem_t *items = (zmq_pollitem_t*) m_poll;
    int rc = zmq_poll(items, 1, 1000);
    if (rc == -1)
        return rc; // Interrupted

    if (items[0].revents & ZMQ_POLLIN)
    {
        std::shared_ptr<YOMessage> msg = std::make_shared<YOMessage>();
        zmq_msg_recv((zmq_msg_t*) &msg->m_data.message, m_socket_sub, 0);

        msg->m_data.buffer = (uint8_t*) zmq_msg_data((zmq_msg_t*) &msg->m_data.message);
        msg->m_data.size = zmq_msg_size((zmq_msg_t*) &msg->m_data.message);

        msg->m_topic_len = std::strlen((char*) msg->m_data.buffer);

        msg->m_header_ptr = (YOHeaderBase*) (msg->m_data.buffer + msg->m_topic_len + 1);
        //logInfo("Received: data [%s] %lX Topic len %i\n", msg->getTopic(), msg->getTimestamp(), msg->m_topic_len);

//        for (int i = 0; i < 99 && i < msg->m_data.size; i++)
//        {
//            printf("%02X ", msg->m_data.buffer[i]);
//        }
//        printf("\n");

        int more;
        size_t more_size = sizeof(more);
        zmq_getsockopt(m_socket_sub, ZMQ_RCVMORE, &more, &more_size);
        if (more)
        {
            zmq_msg_recv((zmq_msg_t*) &msg->m_ext.message, m_socket_sub, 0);
            msg->m_ext.buffer = (uint8_t*) zmq_msg_data((zmq_msg_t*) &msg->m_ext.message);
            msg->m_ext.size = zmq_msg_size((zmq_msg_t*) &msg->m_ext.message);
            //logInfo("!!!!!Received: ext data size: %i \n", msg->m_ext.size);
        }

        auto cb = m_sub_map.find(std::string((const char*) msg->m_data.buffer));

        if (cb != m_sub_map.end())
        {
            //logInfo("PROCESS %s!!!!\n", (const char*) msg->m_data.buffer);
            if (cb->second.fn)
                cb->second.fn(cb->second.topic, msg.get(), cb->second.data);
            else if (cb->second.shared_fn)
                cb->second.shared_fn(cb->second.topic, msg, cb->second.data);
        }
    }
    else
    {
        //logInfo("No message within 1s\n");
    }
    return 0;
}

int YONode::start()
{
    connect();
    //zmq_pollitem_t *items = (zmq_pollitem_t*) m_poll;
    std::cout << " Start polling " << std::endl;

    while (isRunning())
    {
        getMessage(1000);

    }
    logInfo("POLLING STOPPED\n");
    disconnect();
    shutdown();
    return 0;
}

void YONode::logInfo(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
