/*
 * YONode.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#include <zmq.h>
#include <cstdint>
#include "YONode.h"

struct YOSubData
{
    std::string topic;
    YOSubFn fn;
    YOSubSharedFn shared_fn;
    void *data;
};

struct YOPollData
{
    pthread_t thread;
    YOPollFn fn;
    void *data;
};

struct YOPubData
{
    uint16_t type;
    uint16_t subtype;
};
struct YOSocketInfo
{
	void *sock {nullptr};
	bool bind {false};
	std::vector<std::string> sockets;
};

struct YONode::YONodeInfo
{
    void* context;
    zmq_pollitem_t poll[YOSockType::Count];
    std::array<YOSocketInfo, YOSockType::Count> sockets;
    YOSubSharedFn sys_fn;
    void *sys_param;

    std::unordered_map<std::string, YOSubData> sub_map;
    std::unordered_map<std::string, YOPubData> pub_map;
    std::unordered_map<int, YOSigData> sig_map;
};

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

void CreateSocket(YOSocketInfo &sock_data, void* context, int type, const char *default_socket)
{
	sock_data.sock = zmq_socket(context, type);
	sock_data.sockets.push_back(default_socket);
	int size = 128 * 1024 * 1024; // 128MB
	int hwm = 100000; // 100k msgs
    int a0 = zmq_setsockopt(sock_data.sock, ZMQ_RCVHWM, &hwm, sizeof(hwm));
    int a1 = zmq_setsockopt(sock_data.sock, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    int a2 = zmq_setsockopt(sock_data.sock, ZMQ_RCVBUF, &size, sizeof(size));
    int a3 = zmq_setsockopt(sock_data.sock, ZMQ_SNDBUF, &size, sizeof(size));
}

YONode::YONode(const char *node_name) : m_info(std::make_unique<YONodeInfo>())
{
    g_node_ = this;
	m_name = node_name;
    signal(SIGINT, signal_handler);
    m_info->context = zmq_ctx_new();
    CreateSocket(m_info->sockets[YOSockType::DataSub], m_info->context, ZMQ_SUB, YO_SUB_DATA_SRV);
    CreateSocket(m_info->sockets[YOSockType::DataPub], m_info->context, ZMQ_PUB, YO_PUB_DATA_SRV);
    CreateSocket(m_info->sockets[YOSockType::SysSub], m_info->context, ZMQ_SUB, YO_SUB_SYS_SRV);
    CreateSocket(m_info->sockets[YOSockType::SysPub], m_info->context, ZMQ_PUB, YO_PUB_SYS_SRV);

    m_info->poll[YOSockType::DataSub] = { m_info->sockets[YOSockType::DataSub].sock, 0, ZMQ_POLLIN, 0};
    m_info->poll[YOSockType::DataPub] = { m_info->sockets[YOSockType::DataPub].sock, 0, ZMQ_POLLIN, 0};
    m_info->poll[YOSockType::SysSub] = { m_info->sockets[YOSockType::SysSub].sock, 0, ZMQ_POLLIN, 0};
	m_info->poll[YOSockType::SysPub] = { m_info->sockets[YOSockType::SysPub].sock, 0, ZMQ_POLLIN, 0};

    m_info->sys_fn = nullptr;
    m_info->sys_param = nullptr;
	zmq_setsockopt( m_info->sockets[YOSockType::SysSub].sock, ZMQ_SUBSCRIBE, m_name.c_str(), m_name.size());
	std::cout << " System socket subscribed to [" << m_name << "]"<< std::endl;
    /*
    sysctl net.core.rmem_max
    sysctl net.core.wmem_max
    sudo sysctl -w net.core.rmem_max=134217728   # 128 MB
    sudo sysctl -w net.core.wmem_max=134217728
    */
}

YONode::~YONode()
{
    g_node_ = nullptr;
}

void YONode::setConfig(YOVariant &config)
{
    m_config = config;
}

YOVariant& YONode::getConfig()
{
    return m_config;
}

void YONode::setUserData(const std::string &name, void *data)
{
    m_user_data[name] = data;
}

void* YONode::getUserData(const std::string &name)
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
    m_info->sig_map[signal] = {signal, fn, data};
}

YOSigData *YONode::getSignalFunction(int signal)
{
    auto it = m_info->sig_map.find(signal);
    if (it != m_info->sig_map.end()) {
        return &it->second;
    }
    return nullptr;
}

void YONode::sendMessage(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	sendMessage(topic, *message);
}

void YONode::sendMessage(const std::string &topic, YOMessage &message, void *pub_sock)
{
    message.setTimestamp(YONode::getTimestamp());
    message.sendTo(topic, pub_sock);
}

void YONode::sendMessage(const std::string &topic, YOMessage &message)
{
    //TODO check first message size, initSize(0);
    auto stream = m_info->pub_map.find(topic);
    if (stream == m_info->pub_map.end())
    {
        std::cout << " SendMessage: " << topic << " is not advertised " << std::endl;
        return;
    }
    message.setType(stream->second.type);
    message.setSubType(stream->second.subtype);
    sendMessage(topic, message, m_info->sockets[YOSockType::DataPub].sock);
}

void free_fn(void *data, void *hint)
{
	delete (msgpack::sbuffer*) hint;
}

void YONode::sendMessageSys(const std::string &topic, YOMessage &message)
{
	sendMessage(topic, message, m_info->sockets[YOSockType::SysPub].sock);
}

void YONode::subscribeSys(const std::string &topic)
{
	zmq_setsockopt(m_info->sockets[YOSockType::SysSub].sock, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
	std::cout << " System socket subscribed to [" << topic << "]"<< std::endl;
}

void YONode::unsubscribeSys(const std::string &topic)
{
	zmq_setsockopt(m_info->sockets[YOSockType::SysSub].sock, ZMQ_UNSUBSCRIBE, topic.c_str(), topic.size());
	std::cout << " System socket unsubscribed from [" << topic << "]"<< std::endl;
}

void YONode::subscribeSysFn(YOSubSharedFn fn, void *param)
{
	m_info->sys_fn = fn;
	m_info->sys_param = param;
}

void YONode::subscribe(const std::string &topic, YOSubFn fn, void *data)
{
   	std::cout << " Subscribed to [" << topic<< "]"<< std::endl;
   	m_info->sub_map[topic] = {topic, fn, 0, data};
    zmq_setsockopt(m_info->sockets[YOSockType::DataSub].sock, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
}

void YONode::subscribe(const std::string &topic, YOSubSharedFn fn, void *data)
{
	std::cout << " Subscribed ext to [" << topic<< "]"<< std::endl;
	m_info->sub_map[topic] = {topic, 0, fn, data};
    zmq_setsockopt(m_info->sockets[YOSockType::DataSub].sock, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
}

void YONode::unsubscribe(const std::string &topic)
{
    std::cout << " Unsubscribed from [" << topic<< "]"<< std::endl;
    m_info->sub_map.erase(topic);
    zmq_setsockopt(m_info->sockets[YOSockType::DataSub].sock, ZMQ_UNSUBSCRIBE, topic.c_str(), topic.size());
}

void YONode::advertise(const std::string &topic, uint16_t type, uint16_t subtype)
{
	printf("Advertise topic [%s] type: %02X subtype: %02X  \n", topic.c_str(), type, subtype);
	m_info->pub_map[topic] = {type, subtype};
}

void YONode::unadvertise(const std::string &topic)
{
    printf("Unadvertise topic [%s] \n", topic.c_str());
    m_info->pub_map.erase(topic);
}

int YONode::connect()
{
	zmq_connect(m_info->sockets[YOSockType::DataSub].sock, YO_SUB_DATA_SRV);
	zmq_connect(m_info->sockets[YOSockType::DataPub].sock, YO_PUB_DATA_SRV);
	zmq_connect(m_info->sockets[YOSockType::SysSub].sock, YO_SUB_SYS_SRV);
	zmq_connect(m_info->sockets[YOSockType::SysPub].sock, YO_PUB_SYS_SRV);
    usleep(2000);
    std::cout << " Connected: " << m_name << std::endl;
    return 0;
}

int YONode::disconnect()
{
    zmq_disconnect(m_info->sockets[YOSockType::DataSub].sock, YO_SUB_DATA_SRV);
	zmq_disconnect(m_info->sockets[YOSockType::DataPub].sock, YO_PUB_DATA_SRV);
    zmq_disconnect(m_info->sockets[YOSockType::SysSub].sock, YO_SUB_SYS_SRV);
    zmq_disconnect(m_info->sockets[YOSockType::SysPub].sock, YO_PUB_SYS_SRV);
    return 0;
}

int YONode::shutdown()
{
	zmq_close(m_info->sockets[YOSockType::DataPub].sock);
	zmq_close(m_info->sockets[YOSockType::DataSub].sock);
	zmq_close(m_info->sockets[YOSockType::SysPub].sock);
	zmq_close(m_info->sockets[YOSockType::SysSub].sock);
    zmq_ctx_shutdown(m_info->context);
    zmq_ctx_term(m_info->context);
    return 0;
}

int YONode::stop()
{
    stop_flag = 1;
    return 0;
}

std::shared_ptr<YOMessage> YONode::readMessage(void *sock_sub)
{
	std::shared_ptr<YOMessage> msg = std::make_shared<YOMessage>();
	msg->readFrom(sock_sub);

	return msg;
}

int YONode::getMessage(int wait)
{
    int rc = zmq_poll(m_info->poll, 4, wait);

    if (rc == -1)
        return rc; // Interrupted

    if (m_info->poll[YOSockType::SysSub].revents & ZMQ_POLLIN) //system message
    {
    	std::shared_ptr<YOMessage> msg = readMessage(m_info->sockets[YOSockType::SysSub].sock);
    	if(m_info->sys_fn)
    	{
    		m_info->sys_fn(msg->getTopic(), msg, m_info->sys_param);
    	}
    }

    if (m_info->poll[YOSockType::DataSub].revents & ZMQ_POLLIN)
    {
    	std::shared_ptr<YOMessage> msg = readMessage(m_info->sockets[YOSockType::DataSub].sock);
        auto cb = m_info->sub_map.find(msg->getTopic());

        if (cb != m_info->sub_map.end())
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
    std::cout << m_name << ": Start polling " << std::endl;
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
