/*
 * YONode.cpp
 *
 *  Created on: Jun 5, 2025
 *      Author: kurtz
 */

#include <zmq.h>
#include <cstdint>
#include "YONode.h"

struct YOSocketInfo
{
	void *pub;
	void *sub;
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

void SetSocket(void *sock)
{
	int size = 128 * 1024 * 1024; // 128MB
	int hwm = 100000; // 100k msgs
    zmq_setsockopt(sock, ZMQ_RCVHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(sock, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(sock, ZMQ_RCVBUF, &size, sizeof(size));
    zmq_setsockopt(sock, ZMQ_SNDBUF, &size, sizeof(size));
}

void SetSockets(YOSocketInfo *si)
{
	SetSocket(si->pub);
	SetSocket(si->sub);
}
YOSocketInfo* CreateSockets(void* context)
{
	YOSocketInfo* sock_data = (YOSocketInfo*) malloc(sizeof(YOSocketInfo));
	sock_data->pub = zmq_socket(context, ZMQ_PUB);
	sock_data->sub = zmq_socket(context, ZMQ_SUB);
	SetSockets(sock_data);
	return sock_data;
}

YONode::YONode(const char *node_name)
{
    g_node_ = this;
	m_name = node_name;
    signal(SIGINT, signal_handler);
    m_context = zmq_ctx_new();
    m_sock_data = CreateSockets(m_context);
    m_sock_sys = CreateSockets(m_context);
    //m_sock_log = CreateSockets(m_context);
    m_sys_fn = nullptr;
    m_sys_param = nullptr;
    zmq_setsockopt(m_sock_sys->sub, ZMQ_SUBSCRIBE, m_name.c_str(), m_name.size());
	std::cout << " System socket subscribed to [" << m_name << "]"<< std::endl;

    /*
    sysctl net.core.rmem_max
    sysctl net.core.wmem_max
    sudo sysctl -w net.core.rmem_max=134217728   # 128 MB
    sudo sysctl -w net.core.wmem_max=134217728
*/
    m_poll = malloc(sizeof(zmq_pollitem_t) * 4);
    zmq_pollitem_t *items = (zmq_pollitem_t*) m_poll;
    items[0] = {m_sock_sys->sub, 0, ZMQ_POLLIN, 0};
    items[1] = {m_sock_sys->pub, 0, ZMQ_POLLIN, 0};
    items[2] = {m_sock_data->sub, 0, ZMQ_POLLIN, 0};
    items[3] = {m_sock_data->pub, 0, ZMQ_POLLIN, 0};
}

YONode::~YONode()
{
    g_node_ = nullptr;
    delete (zmq_pollitem_t*) m_poll;
    delete m_sock_data;
    delete m_sock_sys;
    //delete m_sock_log;
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

void YONode::sendMessage(const std::string &topic, std::shared_ptr<YOMessage> message)
{
	sendMessage(topic, *message);
}

void YONode::sendMessage(const std::string &topic, YOMessage &message, void *pub_sock)
{
    message.setTopic(topic);
    message.setTimestamp(YONode::getTimestamp());

	int flag = message.getExtDataSize() ? ZMQ_SNDMORE : 0;
    int new_size = message.m_data.size - message.m_topic_start;
    message.setTimestamp(YONode::getTimestamp());
    zmq_msg_init_data((zmq_msg_t*) &message.m_data.message, message.m_data.buffer + message.m_topic_start, new_size, free_data_fn, message.m_data.buffer);
    zmq_msg_send((zmq_msg_t*) &message.m_data.message, pub_sock, flag);
    if (flag != 0)
    {
        zmq_msg_init_data((zmq_msg_t*) &message.m_ext.message, message.m_ext.buffer, message.m_ext.size, free_data_fn, message.m_ext.buffer);
        zmq_msg_send((zmq_msg_t*) &message.m_ext.message, pub_sock, 0);
    }
}

void YONode::sendMessage(const std::string &topic, YOMessage &message)
{
    //TODO check first message size, initSize(0);
    auto stream = m_pub_map.find(topic);
    if (stream == m_pub_map.end())
    {
        std::cout << " SendMessage: " << topic << " is not advertised " << std::endl;
        return;
    }
    message.setType(stream->second.type);
    message.setSubType(stream->second.subtype);
    sendMessage(topic, message, m_sock_data->pub);
}

void free_fn(void *data, void *hint)
{
	delete (msgpack::sbuffer*) hint;
}

void YONode::sendMessageSys(const std::string &topic, YOMessage &message)
{
	sendMessage(topic, message, m_sock_sys->pub);
}

void YONode::subscribeSys(const std::string &topic)
{
	zmq_setsockopt(m_sock_sys->sub, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
	std::cout << " System socket subscribed to [" << topic << "]"<< std::endl;
}

void YONode::unsubscribeSys(const std::string &topic)
{
	zmq_setsockopt(m_sock_sys->sub, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
	std::cout << " System socket unsubscribed from [" << topic << "]"<< std::endl;
}

void YONode::subscribeSysFn(YOSubSharedFn fn, void *param)
{
	m_sys_fn = fn;
	m_sys_param = param;
}

void YONode::subscribe(const std::string &topic, YOSubFn fn, void *data)
{
   	std::cout << " Subscribed to [" << topic<< "]"<< std::endl;
	m_sub_map[topic] = {topic, fn, 0, data};
    zmq_setsockopt(m_sock_data->sub, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
}

void YONode::subscribe(const std::string &topic, YOSubSharedFn fn, void *data)
{
	std::cout << " Subscribed ext to [" << topic<< "]"<< std::endl;
	m_sub_map[topic] = {topic, 0, fn, data};
    zmq_setsockopt(m_sock_data->sub, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
}

void YONode::unsubscribe(const std::string &topic)
{
    std::cout << " Unsubscribed from [" << topic<< "]"<< std::endl;
	m_sub_map.erase(topic);
    zmq_setsockopt(m_sock_data->sub, ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
}

void YONode::advertise(const std::string &topic, uint16_t type, uint16_t subtype)
{
	printf("Advertise topic [%s] type: %02X subtype: %02X  \n", topic.c_str(), type, subtype);
    m_pub_map[topic] = {type, subtype};
}

void YONode::unadvertise(const std::string &topic)
{
    printf("Unadvertise topic [%s] \n", topic.c_str());
	m_pub_map.erase(topic);
}

int YONode::connect()
{
	zmq_connect(m_sock_data->sub, YO_PUB_DATA_SRV);
    zmq_connect(m_sock_data->pub, YO_SUB_DATA_SRV);
    zmq_connect(m_sock_sys->sub, YO_PUB_SYS_SRV);
    zmq_connect(m_sock_sys->pub, YO_SUB_SYS_SRV);

    usleep(2000);
    std::cout << " Connected: " << m_name << std::endl;
    return 0;
}

int YONode::disconnect()
{
    zmq_disconnect(m_sock_data->sub, YO_PUB_DATA_SRV);
    zmq_disconnect(m_sock_data->pub, YO_SUB_DATA_SRV);
    zmq_disconnect(m_sock_sys->sub, YO_PUB_SYS_SRV);
    zmq_disconnect(m_sock_sys->pub, YO_SUB_SYS_SRV);
    return 0;
}

int YONode::shutdown()
{
	zmq_close(m_sock_data->sub);
    zmq_close(m_sock_data->pub);
    zmq_close(m_sock_sys->sub);
    zmq_close(m_sock_sys->pub);
    zmq_ctx_shutdown(m_context);
    zmq_ctx_term(m_context);
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
    zmq_msg_recv((zmq_msg_t*) &msg->m_data.message, sock_sub, 0);
    msg->m_data.buffer = (uint8_t*) zmq_msg_data((zmq_msg_t*) &msg->m_data.message);
    msg->m_data.size = zmq_msg_size((zmq_msg_t*) &msg->m_data.message);
    msg->m_topic_len = std::strlen((char*) msg->m_data.buffer);
    msg->m_header_ptr = (YOHeaderBase*) (msg->m_data.buffer + msg->m_topic_len + 1);
    int more;
    size_t more_size = sizeof(more);
    zmq_getsockopt(sock_sub, ZMQ_RCVMORE, &more, &more_size);
    if (more)
    {
    	zmq_msg_recv((zmq_msg_t*) &msg->m_ext.message, sock_sub, 0);
        msg->m_ext.buffer = (uint8_t*) zmq_msg_data((zmq_msg_t*) &msg->m_ext.message);
        msg->m_ext.size = zmq_msg_size((zmq_msg_t*) &msg->m_ext.message);
    }
	return msg;
}

int YONode::getMessage(int wait)
{
    zmq_pollitem_t *items = (zmq_pollitem_t*) m_poll;
    int rc = zmq_poll(items, 4, 1000);
    if (rc == -1)
        return rc; // Interrupted

    if (items[0].revents & ZMQ_POLLIN) //system message
    {
    	std::shared_ptr<YOMessage> msg = readMessage(m_sock_sys->sub);
    	if(m_sys_fn)
    	{
    		m_sys_fn((const char*) msg->m_data.buffer, msg, m_sys_param);
    	}
    }

    if (items[2].revents & ZMQ_POLLIN)
    {
    	std::shared_ptr<YOMessage> msg = readMessage(m_sock_data->sub);
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
